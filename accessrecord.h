#pragma once

#include "pcb.h"

struct AccessRecordNode {
    frame_num_t frame;
    spid_t owner;
    struct AccessRecordNode *next_newest;
};

struct AccessRecord {
    struct AccessRecordNode *oldest;
    struct AccessRecordNode *newest;
};

frame_num_t accessrecord_get_lru(struct AccessRecord *access_r);
void accessrecord_frame_used(struct AccessRecord *access_r, frame_num_t used, spid_t owner);
void accessrecord_empty(struct AccessRecord *access_r);
