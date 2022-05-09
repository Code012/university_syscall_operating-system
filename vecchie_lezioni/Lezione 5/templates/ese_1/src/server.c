#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <sys/stat.h>
#include <sys/msg.h>

#include "order.h"
#include "errExit.h"

// the message queue identifier
int msqid = -1;

void signTermHandler(int sig) {
    // do we have a valid message queue identifier?
    if (msqid > 0) {
        // ...
    }

    // terminate the server process
    exit(0);
}

int main (int argc, char *argv[]) {
    // check command line input arguments
    if (argc != 2) {
        printf("Usage: %s message_queue_key\n", argv[0]);
        exit(1);
    }

    // read the message queue key defined by user
    int msgKey = atoi(argv[1]);
    if (msgKey <= 0) {
        printf("The message queue key must be greater than zero!\n");
        exit(1);
    }

    // set of signals (N.B. it is not initialized!)
    sigset_t mySet;
    // blocking all signals but SIGTERM
    // ...

    // set the function sigHandler as handler for the signal SIGTERM
    // ...

    printf("<Server> Making MSG queue...\n");
    // get the message queue, or create a new one if it does not exist
    msqid = // ...

    struct order order;

    // endless loop
    while (1) {
        // read a message from the message queue
        // ...

        // print the order on standard output
        printOrder(&order);
    }
}
