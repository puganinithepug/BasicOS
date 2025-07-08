/*
 *  The ready queue of processes.
 *  (actually a list, because that seemed more practical)
 */

#pragma once

#include "pcb.h"

struct ReadyQueueElement {
    struct pcb *val;
    struct ReadyQueueElement *next;
};

typedef struct {
    struct ReadyQueueElement *head;
    struct ReadyQueueElement *tail;
} ReadyQueue;

typedef struct {
    struct ReadyQueueElement *next;
} ReadyQueue_iterator_t;

ReadyQueue *readyqueue_new();
struct pcb *readyqueue_get(ReadyQueue *q, int i);

ReadyQueue_iterator_t *readyqueue_iterator(ReadyQueue *q);
int readyqueue_iterator_hasnext(ReadyQueue_iterator_t *iter);
struct pcb *readyqueue_iterator_next(ReadyQueue_iterator_t *iter);
void readyqueue_iterator_free(ReadyQueue_iterator_t *iter);
    
void readyqueue_insertafter(ReadyQueue *q, int i, struct pcb *p);
void readyqueue_append(ReadyQueue *q, struct pcb *p);
void readyqueue_delete(ReadyQueue *q, int i);
int readyqueue_remove(ReadyQueue *q, struct pcb *p);
int readyqueue_isempty(ReadyQueue *q);
void readyqueue_free(ReadyQueue *q);
