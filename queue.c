// SSOO-P3 23/24

#include "queue.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// To create a queue
queue *queue_init(int size) {
    queue *q = (queue *)malloc(sizeof(queue));
    q->max_size = size;
    q->head = 0;
    q->tail = 0;
    q->size = 0;
    q->elements = malloc(size * sizeof(struct element));
    return q;
}

// To Enqueue an element
int queue_put(queue *q, struct element *x) {
    if (queue_empty(q)){
        q->elements[q->tail] = *x;
    } else {
        q->tail = (q->tail + 1) % q->max_size;
        q->elements[q->tail];
    }
    q->size++;
    return 0;
}

// To Dequeue an element
struct element *queue_get(queue *q) {
    if (!queue_empty(q)) { // if not empty
        struct element *element = &q->elements[q->head];
        q->head = (q->head + 1) % q->max_size;
        q->size--;
        return element;
    }
    return NULL;
}

// To check queue state
int queue_empty(queue *q) {
    // return true (1) if it is empty, false (0) if not
    return q->size == 0;
}

int queue_full(queue *q) {
    // return true (1) if it is full, false (0) if not
    return q->size == q->max_size;
}

// To destroy the queue and free the resources
int queue_destroy(queue *q) {
    if (q != NULL) {
        free(q->elements);
        free(q);
    }
    return 0;
}
