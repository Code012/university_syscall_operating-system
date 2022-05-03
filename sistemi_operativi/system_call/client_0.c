/// @file client.c
/// @brief Contiene l'implementazione del client.

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "defines.h"
#include "errExit.h"

// manipulation of a signal
void sigHandler(int sig);

int main(int argc, char * argv[]) {

    /*
     ***********************
     * CREATE SIGNAL MASK *
     ***********************
     */

    // set of signals
    sigset_t new_set_signals, original_set_signals;

    // initialize new_set_signals to contain all signals of OS
    if(sigfillset(&new_set_signals) == -1)
        errExit("sigfillset failed!\n");

    /*******************************************************
     *                                                     *
     * Si può fare così?                                   *
     * sigdelset(&new_set_signals, SIGINT && SIGUSR1);        *
     *                                                     *
     *                                                     *
     * *****************************************************/
    // remove SIGINT from mySet
    if(sigdelset(&new_set_signals, SIGINT) == -1)
        errExit("sigdelset(SIGINT) failed!\n");
    // remove SIGINT from mySet
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



    // !!!
    //      è buona programmazione ripristinare i segnali originari
    //      del processo. Avevo pensato al "SIG_UNBLOCK"
    // !!!
    // reset the signal mask of the process = restore original mask
    if(sigprocmask(SIG_SETMASK, &original_set_signals, NULL) == -1)
        errExit("sigprocmask(original_set) failed!\n");

    return 0;
}

void sigHandler (int sig) {
    // if signal is SIGUSR1, kill process
    //if(sig == 30)
    //    errExit("SIGUSR1 kill client!\n");
    // Questa opzione fa exit(1)

    if(sig == 30)
        exit(0);
    else;
        // TO DO
}