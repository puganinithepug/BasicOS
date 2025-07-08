#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "utiltypes.h"
#include "pagetbl.h"
#include "shell.h"

struct pcb {
    spid_t pid;
    page_tbl_t *page_tbl;
    unsigned int pc;                // program counter
    int executing;
    unsigned int job_length_score;  // used by AGING
    char *code_file;
};

struct pcb *pcb_new(spid_t pid, page_tbl_t *pt, char *code_file);
size_t pcb_n_lines(struct pcb *p);
void pcb_free(struct pcb *p);
