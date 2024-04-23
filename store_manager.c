// SSOO-P3 23/24

// TODO: ahora mismo no funciona, si quitas los pthread_join por lo menos llega a imprimir pero a veces se quedan como 4 hilos abiertos

#include "queue.h"
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// global variables
struct element *operations; // contain all the operations done, read from file.txt
queue *shared_buffer;
int profits = 0;
int product_stock[5] = {0};
const int prices[5][2] = {{2, 3}, {5, 10}, {15, 20}, {25, 40}, {100, 125}}; // prices[i][0] = buy cost, prices[i][1] = sale price
pthread_cond_t non_full;                                                    // control wether shared buffer is full
pthread_cond_t non_empty;                                                   // control wether shared buffer is empty
pthread_mutex_t mutex;
int num_ops; // first line of file

void *producers_routine(void *args) {
    int *range = (int *)args; // convert void pointer to integer pointer

    // obtain range to read
    int start = range[0];
    int end = range[1];

    while (start < end) {
        struct element x = operations[start]; // take operation element from operations list
        // Critical part: store operation in shared buffer
        pthread_mutex_lock(&mutex);

        while (queue_full(shared_buffer)) {
            pthread_cond_wait(&non_full, &mutex); // unlock mutex and wait until we can add elements
        }

        queue_put(shared_buffer, &x);
        pthread_cond_signal(&non_empty); // unblock threads that are waiting for the queue to be non empty
        pthread_mutex_unlock(&mutex);
        // end of critical part

        start++;
    }
    printf("producer sale\n");
    pthread_exit(0);
}

void *consumers_routine(void *args) {
    int processed_ops = 0;
    int total_ops = *((int *)args);
    printf("hilo consumer con ID: %lu entra\n", (unsigned long)pthread_self());
    while (processed_ops < total_ops) {
        printf("consumer procesa operacion %d y espera al semaforo\n", processed_ops);
        printf("ops: %d, total ops: %d\n", processed_ops, total_ops);

        struct element *x;
        // Critical part: read from shared buffer & process
        pthread_mutex_lock(&mutex);
        printf("multex blocked\n");
        while (queue_empty(shared_buffer)) {
            pthread_cond_wait(&non_empty, &mutex); // unlock mutex and wait until we can read elements
        }

        x = queue_get(shared_buffer);
        pthread_cond_signal(&non_full); // unblock all threads that are waiting for the queue to be non full
        // process operation
        if (x->op == 1) { // PURCHASE
            product_stock[x->product_id - 1] += x->units;
            profits -= (prices[x->product_id - 1][0]) * x->units; // substract buy cost from profit

        } else if (x->op == 2) { // SALE

            product_stock[x->product_id - 1] -= x->units;         // decrement stock
            profits += (prices[x->product_id - 1][1]) * x->units; // add the profit of the sale
        }
        printf("%d %d %d\n", x->product_id, x->op, x->units);
        printf("multex unblocked\n");
        pthread_mutex_unlock(&mutex);
        // end of critical part
        printf("terminamos de procesar op %d\n", processed_ops);

        processed_ops++;
    }

    printf("hilo consumer con ID: %lu sale con %d operaciones procesadas\n", (unsigned long)pthread_self(), processed_ops + 1);
    pthread_exit(0);
}

int main(int argc, const char *argv[]) {

    // check if arguments are correct
    if (argc != 5 || argv[2] <= 0 || argv[3] <= 0 || argv[4] <= 0) { // name of program + 4 arguments (numbers > 0)
        printf("[ERROR]: usage ./store_manager <file_name><num_producers><num_consumers><buff_size>");
        exit(EXIT_FAILURE);
    }

    // store the arguments
    char *file_name = malloc(sizeof(argv[1]));
    strcpy(file_name, argv[1]);
    int num_producers = atoi(argv[2]);
    int num_consumers = atoi(argv[3]);
    int buff_size = atoi(argv[4]);

    // open the file and read from it
    int fd = open(file_name, O_RDONLY);

    if (fd < 0) {
        perror("Error opening the file");
        exit(EXIT_FAILURE);
    }

    free(file_name); // we have already used it

    dup2(fd, STDIN_FILENO); // change input of process for reading from the file
    close(fd);              // we have duplicated it

    if (scanf("%d", &num_ops) < 0) {
        perror("Error reading from the file");
        exit(EXIT_FAILURE);
    }

    operations = malloc(num_ops * sizeof(struct element));
    shared_buffer = queue_init(buff_size); // initialise queue

    for (int i = 0; i < num_ops; i++) {
        int product_id, units;
        char *op_name = malloc(sizeof("PURCHASE") + 1); // longest word + "\0"

        if (scanf("%d %s %d", &product_id, op_name, &units) < 0) {
            perror("Error reading from the file");
            exit(EXIT_FAILURE);
        }

        if (strcmp(op_name, "PURCHASE") != 0 && strcmp(op_name, "SALE") != 0) { // if not "PURCHASE" or "SALE"
            perror("Some operation is not valid");
            exit(EXIT_FAILURE);
        }

        operations[i].product_id = product_id;
        operations[i].op = strcmp(op_name, "PURCHASE") == 0 ? 1 : 2; // PURCHASE = 1, SALE = 2
        operations[i].units = units;
        free(op_name);
    }

    // print all operations (borrar al acabar)
    // for (int i = 0; i < num_ops; i++) {
    //     printf("%d %d %d\n", operations[i].product_id, operations[i].op, operations[i].units);
    // }

    pthread_t producers_threads[num_producers];
    pthread_t consumers_threads[num_consumers];

    int block_size = num_ops / num_consumers;
    int ranges[num_producers][2]; // to store the ranges from which each producer thread must read

    // initialise conditional vbles
    pthread_cond_init(&non_full, NULL);
    pthread_cond_init(&non_empty, NULL);

    // initialise mutex
    pthread_mutex_init(&mutex, NULL);

    // create producers
    for (int i = 0; i < num_producers; i++) {
        ranges[i][0] = i * block_size;           // initial position
        ranges[i][1] = (i + 1) * block_size - 1; // ending position
        if (i == num_producers - 1) {            // last thread
            ranges[i][1] = num_ops - 1;          // if last thread, ending position is last position of array
        }
        pthread_create(&producers_threads[i], NULL, &producers_routine, (void *)ranges[i]);
    }

    block_size = num_ops / num_producers; // distribute operations among consumers
    // create consumers
    for (int i = 0; i < num_consumers; i++) {
        if (i == num_producers - 1) {              // last thread
            block_size = num_ops - i * block_size; // if last thread, process all remaining ops
        }
        pthread_create(&consumers_threads[i], NULL, &consumers_routine, (void *)&block_size);
    }

    // wait for all threads
    for (int i = 0; i < num_producers; i++) {
        pthread_join(producers_threads[i], NULL);
    }
    for (int i = 0; i < num_consumers; i++) {
        pthread_join(consumers_threads[i], NULL);
    }

    // Output
    printf("Total: %d euros\n", profits);
    printf("Stock:\n");
    printf("  Product 1: %d\n", product_stock[0]);
    printf("  Product 2: %d\n", product_stock[1]);
    printf("  Product 3: %d\n", product_stock[2]);
    printf("  Product 4: %d\n", product_stock[3]);
    printf("  Product 5: %d\n", product_stock[4]);

    // destroy mutex & cond vbles
    pthread_cond_destroy(&non_empty);
    pthread_cond_destroy(&non_full);
    pthread_mutex_destroy(&mutex);

    queue_destroy(shared_buffer);
    free(operations);

    return 0;
}
