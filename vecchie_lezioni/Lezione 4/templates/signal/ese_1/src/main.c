#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "errExit.h"

// The signal handler that will be used when the signal SIGINT
// is delivered to the process
/*...*/ sigHandler(/*...*/) {
    printf("Ctrl+C non mi puo' fermare!\n");
}

int main (int argc, char *argv[]) {
    // set of signals (N.B. it is not initialized!)
    sigset_t mySet;
    // initialize mySet to contain all signals
    //...
    // remove SIGINT from mySet
    //...
    // blocking all signals but SIGINT
    //...

    // set the function sigHandler as handler for the signal SIGINT
    if (signal(/*...*/) == /*...*/)
        errExit("change signal handler failed");

    // inifinite loop
    while (1) {
        printf("Ciao, io sono un processo in esecuzione (%d)\n", getpid());
        // sleep for 5 seconds.
        unsigned int t = 5;
        // sleep is interrupted when a signal is delivered to the process!
        // we iterate until all 5 seconds have passed
        while (t!= 0)
            t = sleep(t);
    }

    return 0;
}
