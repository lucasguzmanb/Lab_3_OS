// SSOO-P3 23/24

#include "queue.h"
#include <fcntl.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, const char *argv[]) {
    int profits = 0;
    int product_stock[5] = {0};

    // check if arguments are correct
    if (argc != 5) { // name of program + 4 arguments
        perror("Incorrect number of arguments");
        exit(EXIT_FAILURE);
    }

    char *file_name = malloc(sizeof(argv[1]));
    strcpy(file_name, argv[1]);

    // open the file and read from it
    int fd = open(file_name, O_RDONLY);

    if (fd < 0) {
        perror("Error opening the file");
        exit(EXIT_FAILURE);
    }

    free(file_name); // we have already used it

    dup2(fd, STDIN_FILENO); // change input of process for reading from the file
    close(fd);              // we have duplicated it
    int num_ops;            // first line of file

    if (scanf("%d", &num_ops) < 0) {
        perror("Error reading from the file");
        exit(EXIT_FAILURE);
    }

    struct element *operations = malloc(num_ops * sizeof(struct element));

    for (int i = 0; i < num_ops; i++) {
        int product_id, units;
        char *op_name = malloc(sizeof("PURCHASE") +1); // longest word + "\0"

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
    }

    // Output
    printf("Total: %d euros\n", profits);
    printf("Stock:\n");
    printf("  Product 1: %d\n", product_stock[0]);
    printf("  Product 2: %d\n", product_stock[1]);
    printf("  Product 3: %d\n", product_stock[2]);
    printf("  Product 4: %d\n", product_stock[3]);
    printf("  Product 5: %d\n", product_stock[4]);

    free(operations);

    return 0;
    }
