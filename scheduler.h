#pragma once

#define RR_DELTA 2
#define RR30_DELTA 30
#define POLICIES 5

#include "readyqueue.h"

enum Policy {
    NULL_POLICY,
    RR,
    RR30,
};

struct Scheduler {
    enum Policy policy;
    ReadyQueue *ready_queue;
    int running;
};

int generate_pid();
struct Scheduler *get_running_scheduler();
struct pcb *get_running_pcb_by_pid(struct Scheduler *sch, spid_t pid);
struct Scheduler *scheduler_get(enum Policy policy);
void scheduler_add(struct Scheduler *sch, struct pcb *job);
void scheduler_add_to_front(struct Scheduler *sch, struct pcb *job);
void scheduler_run(struct Scheduler *sch);
void scheduler_run_multithreaded(struct Scheduler *sch);
void scheduler_free();
struct pcb *new_process(FILE *code, char *code_file);
int run_lines_from_process(struct Scheduler *sch, struct pcb *process, int lines);
spid_t getspid();
