#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codestore.h"
#include "limits.h"
#include "pagetbl.h"
#include "accessrecord.h"
#include "scheduler.h"

char framestore[MEMORY_MAX_LINES][CMD_MAX_CHARS];
struct AccessRecord access_record = {
    .oldest = NULL,
    .newest = NULL,
};

void _codestore_throw_error(const char *msg) {
    printf("codestore: Runtime error: %s\n", msg);
    exit(99);
}

// Get index for frame n.
int _get_line_by_frame(int n) {
    int p = FRAME_SIZE * n;

    if (p < MEMORY_MAX_LINES) {
        return p;
    } else {
        _codestore_throw_error("tried to get line for out-of-bounds frame number.");
    }
}

// Initialize the code store
void init_code_store() {
    // Zero the code store
    memset(framestore, 0, sizeof(framestore));
}

// Get a frame without triggering an access record update
frame_t _get_frame_no_touch(frame_num_t frame) {
    if (frame >= N_FRAMES || frame < 0) {
        _codestore_throw_error("frame number out of bounds.");
    }
    return &framestore[_get_line_by_frame(frame)];
}

// Get a frame from the frame store
frame_t get_frame(frame_num_t frame, spid_t caller) {
    accessrecord_frame_used(&access_record, frame, caller);
    return _get_frame_no_touch(frame);
}

// Get a line from a frame
char *frame_get_line(frame_t frame, int line_n) {
    if (line_n >= FRAME_SIZE || line_n < 0) {
        _codestore_throw_error("tried to get a line from a frame with out-of-bounds index.");
    }
    return frame[line_n];
}

// Evict least recently used frame
frame_num_t _evict_frame() {
    frame_num_t new_frame = accessrecord_get_lru(&access_record);
    // Only print output if a page fault occurs while the scheduler is running (don't
    // print output when loading scripts)
    if (get_running_scheduler() != NULL) { 
        printf("Victim page contents:\n\n");
        for (int i = 0; i < PAGE_SIZE; i++) {
            printf("%s", frame_get_line(_get_frame_no_touch(new_frame), i));
        }
        printf("\nEnd of victim page contents.");
    }
    return new_frame;
}

// Find the next empty frame.
frame_num_t _find_empty_frame() {
    for (int i = 0; i < N_FRAMES; i++) {
        if (framestore[_get_line_by_frame(i)][0] == '\0') {
            // First char of frame is a null char, frame must be empty
            return i;
        }
    }
    // No available frames, evict
    return _evict_frame();
}

// Load one page of a file into the frame store. Return the frame number.
// Ensure input does not have error or EOF set.
// Advance by `advance_to` pages before reading.
frame_num_t load_page(FILE *input, spid_t owner) {
    // Find a frame to load the file into
    frame_num_t frame_n = _find_empty_frame();
    frame_t frame = get_frame(frame_n, owner);  // also ensures the frame is updated in access record

    // Load one page worth of lines into the frame
    char *ret_val;
    char *write_to;
    for (int i = 0; i < FRAME_SIZE; i++) {
        // Read file into frame line-by-line
        write_to = frame_get_line(frame, i);
        ret_val = fgets(write_to, CMD_MAX_CHARS - 1, input);
        if (ret_val == NULL) {
            // end of file, no characters
            write_to[0] = '\0';  // set null char so the program knows where to stop
        }
    }

    return frame_n;
}

// Clear a frame
void clear_frame(frame_num_t frame) {
    if (frame >= N_FRAMES || frame < 0) {
        _codestore_throw_error("frame number out of bounds.");
    }
    memset(&framestore[_get_line_by_frame(frame)], 0, FRAME_SIZE * CMD_MAX_CHARS);
}

// Load a script
page_tbl_t *load_script(FILE *script, spid_t owner) {
    page_tbl_t *pt = page_tbl_new();

    // Load at most two pages into shell memory
    for (int i = 0; !feof(script) && !ferror(script) && i < INITIAL_PAGE_N; i++) {
        page_tbl_set(pt, i, load_page(script, owner));
    }

    return pt;
}

// Tasks to perform before termination
void codestore_terminate() {
    accessrecord_empty(&access_record);
}
