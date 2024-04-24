// SSOO-P3 23/24

#ifndef HEADER_FILE
#define HEADER_FILE

struct element {
    int product_id; // Product identifier
    int op;         // Operation
    int units;      // Product units
};

typedef struct queue {
    int max_size;
    int size;
    int head;
    int tail;
    struct element *elements;
} queue;

queue *queue_init(int size);
int queue_destroy(queue *q);
int queue_put(queue *q, struct element *elem);
struct element *queue_get(queue *q);
int queue_empty(queue *q);
int queue_full(queue *q);
void print_queue(queue *q);

#endif
