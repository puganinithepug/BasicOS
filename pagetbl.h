#pragma once

#include <stdlib.h>

#include "utiltypes.h"

struct PageTableRecord {
    int valid;
    frame_num_t frame;
};
typedef struct PageTableRecord page_tbl_t[PAGE_TBL_SIZE];

#define UNKNOWN_FRAME -1;

page_tbl_t *page_tbl_new();
struct PageTableRecord page_tbl_lookup(page_tbl_t *t, page_num_t n);
void page_tbl_set(page_tbl_t *t, page_num_t n, frame_num_t m);
size_t page_tbl_len(page_tbl_t *t);
void page_tbl_invalidate_frame(page_tbl_t *t, frame_num_t frame);
void page_tbl_free(page_tbl_t *t);
