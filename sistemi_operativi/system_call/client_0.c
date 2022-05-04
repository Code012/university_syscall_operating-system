/// @file client.c
/// @brief Contiene l'implementazione del client.

#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "defines.h"
#include "err_exit.h"

sigset_t original_set_signals;

// manipulation of a signal
void sigHandler(int sig);
void set_original_mask();

int main(int argc, char * argv[]) {
    // init program

    /**************
     * CHECK ARGV *
     **************/



    /***********************
     * CREATE SIGNAL MASK *
     ***********************/

    // set of signals
    sigset_t new_set_signals;

    // initialize new_set_signals to contain all signals of OS
    if(sigfillset(&new_set_signals) == -1)
        errExit("sigfillset failed!\n");

    // remove SIGINT from mySet
    if(sigdelset(&new_set_signals, SIGINT) == -1)
        errExit("sigdelset(SIGINT) failed!\n");
    // remove SIGUSR1 from mySet
    if(sigdelset(&new_set_signals, SIGUSR1) == -1)
        errExit("sigdelset(SIGUSR1) failed!\n");

    // blocking all signals except:
    // SIGKILL, SIGSTOP (default)
    // SIGINT, SIGUSR1
    if(sigprocmask(SIG_SETMASK, &new_set_signals, &original_set_signals) == -1)
        errExit("sigprocmask(new_set) failed!\n");

    // definition manipulate SIGINT
    if (signal(SIGINT, sigHandler) == SIG_ERR)
        errExit("change signal handler (SIGINT) failed!\n");

    // definition manipulate SIGUSR1
    if (signal(SIGUSR1, sigHandler) == SIG_ERR)
        errExit("change signal handler (SIGUSR1) failed!\n");

    // attend a signal...
    pause();

    // [Some code here...]

    return 0;
}

void sigHandler (int sig) {
    // if signal is SIGUSR1, set original mask and kill process
    if(sig == SIGUSR1) {
        set_original_mask();
        exit(0);
    }

    // if signal is SIGINT, happy(end) continue
    if(sig == SIGINT)
        printf("I'm awake!\n\n");
}

void set_original_mask() {
    // reset the signal mask of the process = restore original mask
    if(sigprocmask(SIG_SETMASK, &original_set_signals, NULL) == -1)
        errExit("sigprocmask(original_set) failed\n");
}