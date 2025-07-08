#include "readyqueue.h"

// TODO better separation of input validation and business logic

// Print an error and exit (fail fast)
void _readyqueue_throw_error(const char *msg) {
    printf("readyqueue: Runtime error: %s\n", msg);
    exit(99);
}

// ReadyQueueElement constructor
struct ReadyQueueElement *_readyqueue_new_element(struct pcb *val, struct ReadyQueueElement *next) {
    struct ReadyQueueElement *new = (struct ReadyQueueElement *) malloc(sizeof(struct ReadyQueueElement));
    new->val = val;
    new->next = next;
    return new;
}

// ReadyQueue constructor
ReadyQueue *readyqueue_new() {
    ReadyQueue *new = (ReadyQueue *) malloc(sizeof(ReadyQueue));
    new->head = NULL;
    new->tail = NULL;
    return new;
}

// Get the ith element of the list, return NULL if out of bounds.
struct ReadyQueueElement *_readyqueue_get_element(ReadyQueue *q, int i) {
    struct ReadyQueueElement *cursor = q->head;
    for (int j = 0; j < i; j++) {
        if (cursor == NULL) { return cursor; }  // exceeded bounds of the list
        cursor = cursor->next;
    }
   return cursor;
}

// Get the ith value of the list, return NULL if out of bounds.
struct pcb *readyqueue_get(ReadyQueue *q, int i) {
    struct ReadyQueueElement *out = _readyqueue_get_element(q, i);

    // Error checking
    if (out == NULL) { _readyqueue_throw_error("attempted to get index out of bounds."); }

    return out->val;
}

ReadyQueue_iterator_t *readyqueue_iterator(ReadyQueue *q) {
    ReadyQueue_iterator_t *new = (ReadyQueue_iterator_t *) malloc(sizeof(ReadyQueue_iterator_t));
    new->next = q->head;
    return new;
}

// Return 1 if there is another item present, otherwise 0.
int readyqueue_iterator_hasnext(ReadyQueue_iterator_t *iter) {
    return iter->next != NULL;
}

// Get next item from iterator. Return NULL if there is no next item.
struct pcb *readyqueue_iterator_next(ReadyQueue_iterator_t *iter) {
    if (readyqueue_iterator_hasnext(iter)) {
        struct pcb *out = iter->next->val;
        iter->next = iter->next->next;
        return out;
    } else {
        return NULL;
    }
}

void readyqueue_iterator_free(ReadyQueue_iterator_t *iter) {
    free(iter);
}

// Insert an element after index i. Insert at the beginning of the list if i < 0.
void readyqueue_insertafter(ReadyQueue *q, int i, struct pcb *p) {
    if (i < 0) {
        // Insert at the front of the list
        q->head = _readyqueue_new_element(p, q->head);
        if (q->head->next == NULL) { q->tail = q->head; }
        return;
    }

    // Find the predecessor
    struct ReadyQueueElement *predecessor = _readyqueue_get_element(q, i);

    // Error checking
    if (predecessor == NULL) {
        _readyqueue_throw_error("attempted to insert after index out of bounds.");
    }

    // Allocate a new element
    struct ReadyQueueElement *new = _readyqueue_new_element(p, predecessor->next);

    // Insert 
    predecessor->next = new;
    if (predecessor == q->tail) { q->tail = new; }
}

void readyqueue_append(ReadyQueue *q, struct pcb *p) {
    struct ReadyQueueElement *new = _readyqueue_new_element(p, NULL);
    if (readyqueue_isempty(q)) {
        q->head = new;
        q->tail = q->head;
    }
    else { q->tail->next = new; }  // when head is NULL, tail is also NULL
    q->tail = new;
}

void readyqueue_delete(ReadyQueue *q, int i) {
    struct ReadyQueueElement *to_delete;

    // Error checking
    if (readyqueue_isempty(q)) {
        _readyqueue_throw_error("attempted to delete from empty queue.");
    }

    if (i == 0) {
        to_delete = q->head;
        q->head = to_delete->next;
        if (q->head == NULL || q->head->next == NULL) { q->tail = q->head; }
    } else {
        struct ReadyQueueElement *predecessor = _readyqueue_get_element(q, i - 1);
        if (predecessor == NULL) { _readyqueue_throw_error("attempted to delete index out of bounds."); }

        to_delete = predecessor->next;
        if (to_delete == NULL) { _readyqueue_throw_error("attempted to delete index out of bounds."); }
        
        predecessor->next = to_delete->next;
        if (to_delete == q->tail) { q->tail = predecessor; }
    }
    free(to_delete);
}

// Remove element from the ready queue. Return 0 if successful, 1 otherwise.
int readyqueue_remove(ReadyQueue *q, struct pcb *p) {
    // empty queue
    if (readyqueue_isempty(q)) {
        _readyqueue_throw_error("attempted to delete from empty queue.");
        return 1;
    }
    
    struct ReadyQueueElement *predecessor = q->head;
    struct ReadyQueueElement *to_delete;

    // element at head
    if (q->head->val == p) {
        to_delete = q->head;
        q->head = to_delete->next;
        if (q->head == NULL || q->head->next == NULL) { q->tail = q->head; }
        free(to_delete);
        return 0;
    }

    predecessor = q->head;
    
    // Move to the entry before the one to be removed
    while (1) {
        if (predecessor->next == NULL) { return 1; }
        if (predecessor->next->val == p) { break; }
        predecessor = predecessor->next;
    }

    // Delete the entry
    to_delete = predecessor->next;
    predecessor->next = to_delete->next;
    if (to_delete == q->tail) { q->tail = predecessor; }
    free(to_delete);

    return 0;
}

int readyqueue_isempty(ReadyQueue *q) {
    return q->head == NULL;
}

void readyqueue_free(ReadyQueue *q) {
    if (!readyqueue_isempty(q)) {
        // Free all elements
        struct ReadyQueueElement *current = q->head;
        struct ReadyQueueElement *next = current->next;
        while (current != NULL) {
            free(current);
            current = next;
            next = current->next;
        }
    }

    // Free the list
    free(q);
}
