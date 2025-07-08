#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pagetbl.h"

void _pagetbl_throw_error(const char *msg) {
    printf("pagetbl: Runtime error: %s\n", msg);
    exit(99);
}

page_tbl_t *page_tbl_new() {
    page_tbl_t *new = (page_tbl_t *) malloc(sizeof(page_tbl_t));
    for (int i = 0; i < PAGE_TBL_SIZE; i++) {
        (*new)[i].frame = UNKNOWN_FRAME;
        (*new)[i].valid = 0;
    }
    return new; 
}

// Look up a value in the page table. Return -1 if invalid.
struct PageTableRecord page_tbl_lookup(page_tbl_t *t, page_num_t n) {
    if (n < PAGE_TBL_SIZE) {
        return (*t)[n];
    } else {
        _pagetbl_throw_error("page table index out of bounds.");
    }
}

// Set a value in the page table
void page_tbl_set(page_tbl_t *t, page_num_t n, frame_num_t m) {
    (*t)[n].frame = m;
    (*t)[n].valid = 1;
}

void page_tbl_invalidate_frame(page_tbl_t *t, frame_num_t frame) {
    for (int i = 0; i < PAGE_TBL_SIZE; i++) {
        if ((*t)[i].frame == frame) { (*t)[i].valid = 0; }
    }
}

void page_tbl_free(page_tbl_t *t) {
    free(t);
}
