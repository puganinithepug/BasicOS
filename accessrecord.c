#include <stdlib.h>

#include "accessrecord.h"
#include "scheduler.h"

// Linked list of frame records sorted least recently used -> most recently used.

int accessrecord_isempty(struct AccessRecord *access_r) {
    return access_r->oldest == NULL;
}

// Get least recently used element
frame_num_t accessrecord_get_lru(struct AccessRecord *access_r) {
    // Empty queue
    if (accessrecord_isempty(access_r)) {
        printf("accessrecord: Runtime error: attempted to get LRU element from empty AccessRecord.\n");
        exit(99);
    }

    // Output frame number (save before delete)
    frame_num_t out = access_r->oldest->frame;

    // Invalidate the frame for its previous owner (if present)
    struct Scheduler *sch = get_running_scheduler();
    if (sch != NULL) {
        // Invalidate pages if the scheduler is running.
        struct pcb *owner = get_running_pcb_by_pid(sch, access_r->oldest->owner);
        if (owner != NULL) {
            page_tbl_invalidate_frame(owner->page_tbl, out);
        }
    }

    // Delete entry
    struct AccessRecordNode *to_delete = access_r->oldest;
    if (access_r->oldest->next_newest == NULL) {
        // one element list
        access_r->newest = NULL;
    }
    access_r->oldest = access_r->oldest->next_newest;
    free(to_delete);

    // Return frame number
    return out;
}

// Update AccessRecord in light of a frame number being accessed.
void accessrecord_frame_used(struct AccessRecord *access_r, frame_num_t used, spid_t owner) {
    struct AccessRecordNode *used_node = NULL;
    if (!accessrecord_isempty(access_r)) {
        if (access_r->oldest->frame == used) {
            // oldest element was accessed
            used_node = access_r->oldest;
        } else {
            struct AccessRecordNode *cursor = access_r->oldest;
            while (cursor->next_newest != NULL) {
                if (cursor->next_newest->frame == used) {
                    // Found existing node for used frame number
                    used_node = cursor->next_newest;
                    
                    // Stitch together
                    cursor->next_newest = used_node->next_newest;
                    
                    break;
                }
                cursor = cursor->next_newest;
            }
        }
    }

    if (used_node == NULL) {
        // frame number was not in the list
        // create new node
        used_node = (struct AccessRecordNode *) malloc(sizeof(struct AccessRecordNode));
        used_node->frame = used;

        // handle empty list
        if (accessrecord_isempty(access_r)) {
            access_r->oldest = used_node;
        } else {
            access_r->newest->next_newest = used_node;
        }
    } else if (used_node->next_newest != NULL) {
        // move to back (newest)
        if (used_node == access_r->oldest) { access_r->oldest = used_node->next_newest; }
        access_r->newest->next_newest = used_node;
    }

    // Update position
    access_r->newest = used_node;
    used_node->next_newest = NULL;
    
    // Update owner
    used_node->owner = owner;
}

// Empty access record and free contents
void accessrecord_empty(struct AccessRecord *access_r) {
    while (access_r->oldest != NULL) {
        struct AccessRecordNode *to_free = access_r->oldest;
        access_r->oldest = access_r->oldest->next_newest;
        free(to_free);
    }
}
