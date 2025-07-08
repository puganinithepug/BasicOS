#include <string.h>

#include "pcb.h"
#include "pagetbl.h"

// PCB constructor
struct pcb *pcb_new(spid_t pid, page_tbl_t *pt, char *code_file) {
    struct pcb *new = malloc(sizeof(struct pcb));
    *new = (struct pcb) {
        .pid = pid,
        .page_tbl = pt,
        .pc = 0,
        .executing = 0,
        .job_length_score = 0, 
        .code_file = strdup(code_file),
    };
    return new;
}

// PCB destructor
void pcb_free(struct pcb *p) {
    page_tbl_free(p->page_tbl);
    free(p->code_file);
    free(p);
};
