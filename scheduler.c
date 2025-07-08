#include <limits.h>
#include <stdio.h>

#include "scheduler.h"
#include "shell.h"
#include "codestore.h"
#include "pagetbl.h"
#include "pcb.h"

struct Scheduler *running_scheduler;

int current_pid = 0;
int last_pid = 0;

// Get simulated PID
spid_t getspid() {
    return current_pid;
}

int generate_pid() {
    return ++last_pid;
}

struct Scheduler *get_running_scheduler() {
    if (running_scheduler != NULL && running_scheduler->running) {
        return running_scheduler;
    } else {
        return NULL;
    }
}

// Get PCB by PID. Return NULL if the PCB is not running. NOT THREAD-SAFE.
struct pcb *get_running_pcb_by_pid(struct Scheduler *sch, spid_t pid) {
    struct pcb *out = NULL;

    // Iterate over the ready queue
    ReadyQueue_iterator_t *iter = readyqueue_iterator(sch->ready_queue);
    struct pcb *cursor;
    while (readyqueue_iterator_hasnext(iter)) {
        cursor = readyqueue_iterator_next(iter);
        if (cursor->pid == pid) {
            out = cursor;
            break;
        }
    }
    readyqueue_iterator_free(iter); 

    return out;
}

struct Scheduler *scheduler_new(enum Policy policy) {
    struct Scheduler *new = (struct Scheduler *) malloc(sizeof(struct Scheduler));
    new->policy = policy;
    new->ready_queue = readyqueue_new();
    new->running = 0;
    return new;
}

// Flyweight store
struct {
    enum Policy policy;
    struct Scheduler *scheduler;
} flyweight_store[POLICIES];

// Scheduler static factory
struct Scheduler *scheduler_get(enum Policy policy) {
    int i = 0;
    for (; i < POLICIES; i++) {
        if (flyweight_store[i].policy == policy) { return flyweight_store[i].scheduler; }  // return pre-initialized scheduler
        if (flyweight_store[i].policy == 0) { break; }  // break on first empty entry
    }
    // scheduler hasn't been initialized
    flyweight_store[i].policy = policy;
    flyweight_store[i].scheduler = scheduler_new(policy);
    return flyweight_store[i].scheduler;
}

void scheduler_add(struct Scheduler *sch, struct pcb *job) {
    int insert_after_i;
    struct pcb *cursor;
    ReadyQueue_iterator_t *iter;

    switch (sch->policy) {
        case RR:
        case RR30:
            readyqueue_append(sch->ready_queue, job);
            break;
    }
}

// Remove a job
void scheduler_remove(struct Scheduler *sch, struct pcb *job) {
    readyqueue_remove(sch->ready_queue, job);  // remove from ready queue
    pcb_free(job);                               // deallocate (also frees shell memory)
}

// Helper method to RR and RR30
void _round_robin(struct Scheduler *sch, size_t delta) {
    ReadyQueue_iterator_t *iter = readyqueue_iterator(sch->ready_queue);
    struct pcb *cursor;
    int done;               // 1 if the process finished, 0 otherwise
    while (readyqueue_iterator_hasnext(iter)) {
        cursor = readyqueue_iterator_next(iter);
        done = run_lines_from_process(sch, cursor, delta);
        if (done) {
            // Process finished
            scheduler_remove(sch, cursor); 
        }
    }
    readyqueue_iterator_free(iter);
}

// Main scheduler loop
void scheduler_run(struct Scheduler *sch) {
    // Check that no other scheduler is running
    if (running_scheduler != NULL) {
        printf("scheduler: Runtime error: attempted to run two schedulers at once.\n");
        return;  // don't exit
    }
    sch->running = 1;
    running_scheduler = sch;

    // Run until no jobs remain
    while (!readyqueue_isempty(sch->ready_queue)) {
        switch (sch->policy) {
            case RR:
                _round_robin(sch, RR_DELTA);
                break;
            case RR30:
                _round_robin(sch, RR30_DELTA);
                break;
        }
    }

    sch->running = 0;
    running_scheduler = NULL;
}

// Free all schedulers
void scheduler_free() {
    for (int i = 0; i < POLICIES; i++)  {
        if (flyweight_store[i].scheduler != NULL) {
            readyqueue_free(flyweight_store[i].scheduler->ready_queue);
            free(flyweight_store[i].scheduler);
        }
    }
}

// Create a new process from a stream which produces code
struct pcb *new_process(FILE *code, char *code_file) {
    // Generate PID
    spid_t pid = generate_pid();

    // Create page table
    page_tbl_t *pt = load_script(code, pid);

    // Create PCB
    struct pcb *new = pcb_new(generate_pid(), pt, code_file);
    return new;
}

// Page fault system call to scheduler. Return 0 if the process should continue, 1 if it is finished.
int scheduler_page_fault(struct Scheduler *sch, struct pcb *caller, page_num_t page) {
    // Open the caller's code source file (backing store or otherwise)
    FILE *codesource = fopen(caller->code_file, "r");
    if (codesource == NULL) {
        printf("scheduler: Runtime error: could not open code file '%s' to read.", caller->code_file);
        exit(99);
    }
    
    // Advance the buffer
    for (int newlines = 0; newlines < page * PAGE_SIZE; ) {
        char c = getc(codesource);
        if (c == '\n') { newlines++; }
        else if (c == EOF) { return 1; }  // Process finished
    }

    // Load the missing page
    printf("Page fault! ");
    frame_num_t new_frame = load_page(codesource, caller->pid);
    page_tbl_set(caller->page_tbl, page, new_frame);
    putchar('\n');

    fclose(codesource);

    return 0;
}

// Run a specified number of lines from a process. Return 1 if the process finishes.
// Set lines to -1 to run until termination (or page fault).
int run_lines_from_process(struct Scheduler *sch, struct pcb *proc, int lines) {
    current_pid = proc->pid;
    proc->executing = 1;

    char *line;                     // line to execute
    frame_num_t frame_n;            // current frame number
    frame_t frame = NULL;           // current frame
    int stop = lines < 0 ? INT_MAX : proc->pc + lines;
    while (proc->pc < stop) {
        if (frame == NULL || (proc->pc % PAGE_SIZE) == 0) {
            // Load a new frame

            // Lookup page table
            page_num_t page_n = proc->pc / PAGE_SIZE;
            struct PageTableRecord record = page_tbl_lookup(proc->page_tbl, page_n);
            if (record.valid) {
                // Found valid record, get frame and continue
                frame = get_frame(record.frame, proc->pid);
            } else {
                // Page fault
                return scheduler_page_fault(sch, proc, page_n);
                /* N.B. Due to the way round-robin is implemented (iterating
                 * over the ready queue), no further action is needed to ensure
                 * this process will not run again until all others have (it is
                 * at the "back" of the queue).
                 */
            }
        }
        line = frame_get_line(frame, proc->pc % PAGE_SIZE);
        if (line[0] == '\0') { return 1; }  // reached end of file

        // Run the command
        execute_line(line);
        proc->pc++;
    }

    return 0;
}
