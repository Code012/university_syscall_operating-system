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
int search_dir (char *buf, char *to_send[], int count);
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

    // Blocking all blockable signals
    sigfillset(&new_set_signals);
    if(sigprocmask(SIG_SETMASK, &new_set_signals, NULL) == -1)
        errExit("sigprocmask(original_set) failed");

    /****************
     * FILE READING *
     ****************/

    // change process working directory
    if (chdir(path_to_dir) == -1)
        errExit("Error while changing directory");

    // alloc 150 character
    char *buf = malloc(sizeof(char) * 150);
    check_malloc(buf);
    
    // get current wotking directory
    getcwd(buf, 150);

    printf("Ciao %s, ora inizio l’invio dei file contenuti in %s\n\n", getenv("USER"), buf);

    // Counters and memory
    int count = 0;
    //char to_send[100][150];
    char *to_send[100];

    count = search_dir (buf, to_send, count);

    free(buf);
  
    printf("n = %d\n\n", count);
    for (int i = 0 ; i < count ; i++) {
        printf("to_send[%d] = %s\n", i, to_send[i]);
    }
    

    return 0;
}

// Personalised signal handler for SIGUSR1 and SIGINT
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

// Function that recursively searches files in the specified directory
int search_dir (char *buf, char *to_send[], int count) {
    // Structs and variables
    DIR *dir = opendir(buf);
    char *file_path = malloc(sizeof(char) * 150);
    check_malloc(file_path);
    struct dirent *file_dir = readdir(dir);
    struct stat statbuf;

    while (file_dir != NULL) {

        // Check if file_dir refers to a file starting with sendme_
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
                // Allocate memory for file_path
                to_send[count] = malloc(sizeof(char) * strlen(file_path));
                check_malloc(to_send[count]);
                // saving file_path
                strcpy(to_send[count], file_path);
                count++;
                printf("%d\n", count);
            }
        }

        // Check if file_dir refers to a directory
        if (file_dir->d_type == DT_DIR && strcmp(file_dir->d_name, ".") != 0 && strcmp(file_dir->d_name, "..") != 0) {
            // Creating file_path string
            strcpy(file_path, buf);
            strcat(strcat(file_path, "/"), file_dir->d_name);

            count = search_dir(file_path, to_send, count);
        }

        file_dir = readdir(dir);
    }
    free(file_path);
    if (closedir(dir) == -1)
        errExit("Error while closing directory");

    return count;
}

// Function used to reset the default mask of the process
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