#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include "errExit.h"

pid_t child1, child2;

// The sigHandler is the signal handler for the signal SIGINT
// The function gently terminates the two sub processes by sending SIGTERM
void sigHandler(int sig) {
    // ...
    // eventually, also the parent process termiantes
    exit(0);
}

int main (int argc, char *argv[]) {
    // create the first process (figlio1)
    child1 = fork();
    if (child1 == -1)
        errExit("fork failed");

    if (child1 == 0) {
        // the infinite loop to simulate a working task
        while (1) {
            // ...
        }
    }

    // create the process (figlio2)
    child2 = fork();
    if (child2 == -1)
        errExit("fork failed");

    if (child2 == 0) {
        // the second process simulates an asincronous event, which suspends
        // the working task
        while (1) {
            sleep(10);
            printf("Sono figlio2, disturbo figlio1...\n");
            // send SIGSTOP to child 1
            // ...
        }
    }

    // Set sigHandler as handler to manage SIGINT signal sent by user
    if (/*...*/)
        errExit("change signal handler failed");

    int status;
    while (1) {
        // The parent process implements a monitoring process, which resets
        // the normal execution of the working process (child1)
        sleep(15);
        // monitor status of child 1 (see waitpid options)
        pid_t p = // ...

        if (p == -1)
            errExit("waitPid failed");
        else if (/*...*/) {
            // if child1 process is stopped, then the parent resumes it
            printf("<parent> Resume figlio1...\n");
            // send SIGCONT to child1
            // ...
        }
    }

    return 0;
}
