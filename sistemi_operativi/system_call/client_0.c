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

// manipulation of a signal & mod signal mask
void sigHandler(int sig);
void set_original_mask();
void create_signal_mask();

int main(int argc, char * argv[]) {
    /**************
     * CHECK ARGS *
     **************/

    if (argc != 2)
        errExit("Intended usage: ./client_0 <HOME>/myDir/");

    // pointer to the inserted path
    char *path_to_dir = argv[1];


    /***********************
     * CREATE SIGNAL MASK *
     ***********************/

    create_signal_mask();

    // attend a signal...
    pause();
    
    /* resume execution after SIGINT */

    // add at the set of block signals: SIGINT && SIGUSR1
    if (sigaddset(&new_set_signals, SIGINT) == -1 &&
            sigaddset(&new_set_signals, SIGUSR1) == -1)
        errExit("Error while adding signals to mask");

    /*
    !!! Nell'operazione precedente viene risparmiata una creazione (bit-mask) !!!
    // block all signals
    sigfillset(&new_set_signals);
    if(sigprocmask(SIG_SETMASK, &new_set_signals, NULL) == -1)
        errExit("sigprocmask(original_set) failed");
    */


    /****************
     * FILE READING *
     ****************/

    // change process working directory
    if (chdir(path_to_dir) == -1)
        errExit("Error while changing directory");

    // alloc 150 character
    char *buf = malloc(sizeof(char) * 150); 
    /// TODO: Da eseguire controllo sulla malloc
    getcwd(buf, 150);

    printf("Ciao %s, ora inizio l’invio dei file contenuti in %s\n\n", getenv("USER"), buf);

    // Counters and memory
    int count = 0;
    //char to_send[100][150];
    char *to_send = malloc(sizeof(char) * 100 * 150);
    char *file_path = malloc(sizeof(char) * 150); 
    /// TODO: Da eseguire controllo sulla malloc

    //// TODO: make a function out of it
    // Variables and structs
    DIR *dir = opendir(buf);
    struct dirent *file_dir = readdir(dir);
    struct stat statbuf;

    while (file_dir != NULL) {
        if (file_dir->d_type == DT_REG &&
                strncmp(file_dir->d_name, "sendme_", 7) == 0) {
            // Creating file_path string
            strcpy(file_path, buf);
            strcat(strcat(file_path, "/"), file_dir->d_name);

            // Retrieving file stats
            if (stat(file_path , &statbuf) == -1)
                errExit("Could not retrieve file stats");

            // Check file size
            if (statbuf.st_size <= 4096) {
                // Save file pathname
                strcpy(to_send[count], file_path);
                count++;
            }
        }
        file_dir = readdir(dir);
    }
    closedir(dir);
    ////
    /// TODO: da eseguire controllo sulle free (?)
    free(buf);
    free(file_path);
  
    printf("n = %d\n\n", count);
    for (int i = 0 ; i < count ; i++) {
        printf("to_send[%d] = %s\n", i, to_send[i]);
    }
    

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
        printf("\n\nI'm awake!\n\n");
}

void set_original_mask() {
    // reset the signal mask of the process = restore original mask
    if(sigprocmask(SIG_SETMASK, &original_set_signals, NULL) == -1)
        errExit("sigprocmask(original_set) failed");
}

void create_signal_mask() {
    // initialize new_set_signals to contain all signals of OS
    if(sigfillset(&new_set_signals) == -1)
        errExit("sigfillset failed!");

    // remove SIGINT from new_set_signals
    if(sigdelset(&new_set_signals, SIGINT) == -1)
        errExit("sigdelset(SIGINT) failed!");
    // remove SIGUSR1 from new_set_signals
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
}