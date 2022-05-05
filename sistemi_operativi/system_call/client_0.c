/// @file client.c
/// @brief Contiene l'implementazione del client.

#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "defines.h"
#include "err_exit.h"

// set of signals
sigset_t original_set_signals;
sigset_t new_set_signals;

// manipulation of a signal
void sigHandler(int sig);
void set_original_mask();

int main(int argc, char * argv[]) {
    // init program

    /**************
     * CHECK ARGS *
     **************/

    if (argc != 2) {
        errExit("Intended usage: ./client_0 <HOME>/myDir/");
    }

    char *path_to_dir = argv[1];

    /***********************
     * CREATE SIGNAL MASK *
     ***********************/
    

    // initialize new_set_signals to contain all signals of OS
    if(sigfillset(&new_set_signals) == -1)
        errExit("sigfillset failed!");

    // remove SIGINT from mySet
    if(sigdelset(&new_set_signals, SIGINT) == -1)
        errExit("sigdelset(SIGINT) failed!");
    // remove SIGUSR1 from mySet
    if(sigdelset(&new_set_signals, SIGUSR1) == -1)
        errExit("sigdelset(SIGUSR1) failed!");

    // blocking all signals except:
    // SIGKILL, SIGSTOP (default)
    // SIGINT, SIGUSR1
    if(sigprocmask(SIG_SETMASK, &new_set_signals, &original_set_signals) == -1)
        errExit("sigprocmask(new_set) failed!");

    // definition manipulate SIGINT
    if (signal(SIGINT, sigHandler) == SIG_ERR)
        errExit("change signal handler (SIGINT) failed!");

    // definition manipulate SIGUSR1
    if (signal(SIGUSR1, sigHandler) == SIG_ERR)
        errExit("change signal handler (SIGUSR1) failed!");

    // attend a signal...
    pause();

    sigfillset(&new_set_signals);
    if(sigprocmask(SIG_SETMASK, &new_set_signals, NULL) == -1)
        errExit("sigprocmask(original_set) failed");

    /****************
     * FILE READING *
     ****************/

    if (chdir(path_to_dir) == -1)
        errExit("Error while changing directory");

    char buf[150];
    getcwd(buf, 150);

    printf("Ciao %s, ora inizio l’invio dei file contenuti in %s\n\n", getenv("USER"), buf);

    // Counters and memory
    int n = 0;
    char to_send[100][150];
    char file_path[150];

    //// TODO: make a function out of it

    // Variables and structs
    DIR *dir = opendir(buf);
    struct dirent *file_dir = readdir(dir);
    struct stat statbuf;

    while (file_dir != NULL) {
        if (file_dir->d_type == DT_REG && strncmp(file_dir->d_name, "sendme_", 7) == 0) {
            // Creating file_path string
            strcpy(file_path, buf);
            strcat(strcat(file_path, "/"), file_dir->d_name);

            // Retrieving file stats
            if (stat(file_path , &statbuf) == -1)
                errExit("Could not retrieve file stats");

            // Check file size
            if (statbuf.st_size <= 4096) {
                // Save file pathname
                strcpy(to_send[n], file_path);
                n++;
            }
        }
        file_dir = readdir(dir);
    }
    closedir(dir);
    ////

    /*  
    printf("n = %d\n\n", n);
    for (int i = 0 ; i < n ; i++) {
        printf("to_send[%d] = %s\n", i, to_send[i]);
    }
    */

    return 0;
}

void sigHandler (int sig) {
    // if signal is SIGUSR1, set original mask and kill process
    if(sig == SIGUSR1) {
        set_original_mask();
        exit(0);
    }

    // if signal is SIGINT, happy(end) continue
    if(sig == SIGINT) {
        printf("\n\nI'm awake!\n\n");
    }

}

void set_original_mask() {
    // reset the signal mask of the process = restore original mask
    if(sigprocmask(SIG_SETMASK, &original_set_signals, NULL) == -1)
        errExit("sigprocmask(original_set) failed");
}