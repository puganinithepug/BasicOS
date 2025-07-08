#pragma once

#include <stdio.h>

#include "utiltypes.h"
#include "pcb.h"
#include "pagetbl.h"

#define INITIAL_PAGE_N 2       // number of pages to load from a new process

void init_code_store();
int load_page(FILE *input, spid_t owner);
char *frame_get_line(frame_t frame, int line_n);
frame_t get_frame(frame_num_t frame, spid_t caller);
void clear_frame(frame_num_t frame);
page_tbl_t *load_script(FILE *script, spid_t owner);
void codestore_terminate();
