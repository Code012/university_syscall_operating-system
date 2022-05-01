#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include "errExit.h"

pid_t child1, child2;

void sigHandlerChild1(int sig) {
    if (sig == SIGUSR2) {
        printf("<child1> received SIGUSR2!\n");
        exit(0);
    }
}

void sigHandlerChild2(int sig) {
    if (sig == SIGUSR1) {
        printf("<child2> received SIGUSR1. Sending SIGUSR2 to parent\n");
        if (kill(getppid(), SIGUSR2) == -1)
            errExit("kill failed");

        exit(0);
    }
}

void sigHandlerParent(int sig) {
    if (sig == SIGUSR1) {
        printf("<parent> received SIGUSR1. Sending SIGUSR1 to child2\n");
        if (kill(child2, SIGUSR1) == -1)
            errExit("kill failed");
    } else if (sig == SIGUSR2) {
        printf("<parent> received SIGUSR2. Sending SIGUSR2 to child1\n");
        if (kill(child1, SIGUSR2) == -1)
            errExit("kill failed");
    }
}

int main (int argc, char *argv[]) {
    // set of signals. Not initialized!
    sigset_t mySet;
    // initialize mySet to contain all signals
    sigfillset(&mySet);
    // blocking all signals
    sigprocmask(SIG_SETMASK, &mySet, NULL);

    // create the first process child (figlio1)
    child1 = fork();
    if (child1 == -1)
        errExit("fork failed");

    // keep in mind: Also for child1 all signals are blocked!

    if (child1 == 0) {
        // child1 uses sigHandlerChild1 as handler for SIGUSR2,
        // which is blocked since the parent blocked all signals
        // Set sigHandlerChild1 as handler for SIGUSR2
        if (signal(SIGUSR2, sigHandlerChild1) == SIG_ERR)
            errExit("<child1> change signal handler failed");

        // remove SIGUSR2 to the list of blocked signals
        sigdelset(&mySet, SIGUSR2);
        // update signal mask
        if (sigprocmask(SIG_SETMASK, &mySet, NULL) == -1)
            errExit("sigprocmask failed");

        printf("<child1> sending SIGUSR1 to parent\n");

        // send SIGUSR1 to parent process
        if (kill(getppid(), SIGUSR1) == -1)
            errExit("kill failed");

        // wait for SIGUSR2 signal, which could have already beed delivered!
        pause();

        exit(0);
    }

    // create the second process child (figlio2)
    child2 = fork();
    if (child2 == -1)
        errExit("fork failed");

    if (child2 == 0) {
        // child2 uses sigHandlerChild2 as handler for SIGUSR1,
        // which is blocked since the parent blocked all signals
        // set sigHandlerChild2 as handler for SIGUSR1
        if (signal(SIGUSR1, sigHandlerChild2) == SIG_ERR)
            errExit("<child2> change signal handler failed");

        // remove SIGUSR1 from the list of blocked signals
        sigdelset(&mySet, SIGUSR1);
        // update signal mask
        if (sigprocmask(SIG_SETMASK, &mySet, NULL) == -1)
            errExit("sigprocmask failed");

        // wait for SIGUSR1 signal, which could have already beed delivered!
        pause();

        exit(0);
    }

    // Code executed by parent task
    // set sigHandler as handler for SIGUSR1 and SIGUSR2
    if (signal(SIGUSR1, sigHandlerParent) == SIG_ERR ||
        signal(SIGUSR2, sigHandlerParent) == SIG_ERR)
    { errExit("change signal handler failed"); }

    // remove SIGUSR1 from mySet
    sigdelset(&mySet, SIGUSR1);
    // remove SIGUSR2 from mySet
    sigdelset(&mySet, SIGUSR2);

    // update signal mask to block all signals but SIGUSR1 and SIGUSR2
    if (sigprocmask(SIG_SETMASK, &mySet, NULL) == -1)
        errExit("sigprocmask failed");

    // wait termination of both task children
    int status;
    while ( wait(&status) != -1);

    printf("Both the child processes are terminated\n");

    return 0;
}
