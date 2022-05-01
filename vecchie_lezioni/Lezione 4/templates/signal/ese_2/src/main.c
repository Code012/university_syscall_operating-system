#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "errExit.h"

// The signal handler that will be used when the signal SIGALRM
// is delivered to the process
void sigHandler(int sig) {
    printf("Che bella dormita!\n");
}

int main (int argc, char *argv[]) {

    // Check command line argument
    if (argc != 2) {
        printf("Usage: %s seconds\n", argv[0]);
        return 1;
    }

    int sleepFor = atoi(argv[1]);
    if (sleepFor <= 0)
        return 1;

    // set the function sigHandler as handler for the signal SIGALRM
    //...

    // set a timer, an alarm, with a delay of sleepFor seconds
    //...

    // pause the calling process until any signal is delivered
    //...

    return 0;
}
