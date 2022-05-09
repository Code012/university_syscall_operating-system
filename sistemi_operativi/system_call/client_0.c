/// @file client.c
/// @brief Contiene l'implementazione del client.

#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "defines.h"
#include "err_exit.h"
#include "fifo.h"
#include "semaphore.h"
#include "shared_memory.h"

#define MAX_NUM_FILES 100

// set of signals
sigset_t original_set_signals;
sigset_t new_set_signals;

// manipulation of a signal & mod signal mask
void sigHandler(int sig);
int search_dir (char *buf, char **to_send, int count);
void set_original_mask();
void create_signal_mask();
int semid;

int main(int argc, char * argv[]) {
    /**************
     * CHECK ARGS *
     **************/

    if (argc != 2)
        errExit("Intended usage: ./client_0 <HOME>/myDir/");

    // pointer to the inserted path
    char *path_to_dir = argv[1];


    /**********************
     * CREATE SIGNAL MASK *
     **********************/

    create_signal_mask();

    // attend a signal...
    pause();

    // blocking all blockable signals
    sigfillset(&new_set_signals);
    if(sigprocmask(SIG_SETMASK, &new_set_signals, NULL) == -1)
        errExit("sigprocmask(original_set) failed");


    /*****************
     * OBTAINING IDs *
     *****************/

    // try to obtain set id of semaphores
    do {
        printf("Looking for the semaphore...\n\n");
        semid = semget_usr(ftok("client_0", 'a'), 0, S_IRUSR | S_IWUSR);
        sleep(2);
    } while(semid == -1);

    // waiting for IPCs to be created
    semop_usr(semid, 0, -1);
    
    // opening of all the IPC's
    int fifo1_fd = open_fifo("FIFO1", O_WRONLY);
    int fifo2_fd = open_fifo("FIFO2", O_WRONLY);
    int queue_id = msgget(ftok("client_0", 'a'), S_IRUSR | S_IWUSR);
    int shmem_id = alloc_shared_memory(ftok("client_0", 'a'),
                                        sizeof(struct queue_msg) * 50,
                                        S_IRUSR | S_IWUSR);
    struct queue_msg *shmpointer = (struct  queue_msg *) attach_shared_memory(shmem_id, 0);


    /****************
     * FILE READING *
     ****************/

    // change process working directory
    if (chdir(path_to_dir) == -1)
        errExit("Error while changing directory");

    // alloc PATH_MAX (4096) character
    char *buf = malloc(sizeof(char) * PATH_MAX);
    check_malloc(buf);
    
    // get current wotking directory
    getcwd(buf, PATH_MAX);

    printf("Ciao %s, ora inizio l’invio dei file contenuti in %s\n\n", getenv("USER"), buf);

    // file found counter
    int count = 0;
    
    // creating an array to store the file paths
    char *letters[MAX_NUM_FILES];
    char **to_send = letters;
    //!!! DeBuG !!!
    //printf("Zona di memoria letters: %p. Zona di memoria to_send: %p", *letters, *to_send);
    //!!!       !!!

    // search files into directory
    count = search_dir (buf, to_send, count);


    /*******************************
     * CLIENT-SERVER COMMUNICATION *
     *******************************/

    // Writing n_files on FIFO1
    write_fifo(fifo1_fd, &count, sizeof(count));

    // Unlocking semaphore 1 (allow server to read from FIFO1)
    semop_usr(semid, 1, 1);

    // Printing all file routes
    printf("\nn = %d\n\n", count);
    for (int i = 0 ; i < count ; i++)
        printf("to_send[%d] = %s\n", i, (char *) to_send[i]);
    

    /**************
     * CLOSE IPCs *
     **************/ 

    close(fifo1_fd);
    close(fifo2_fd);
    free_shared_memory(shmpointer);

    free(buf);

    return 0;
}


    /******************************
    * BEGIN FUNCTIONS DEFINITIONS *
    *******************************/

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

// function that recursively searches files in the specified directory
int search_dir (char *buf, char **to_send, int count) {
    // Structs and variables
    DIR *dir = opendir(buf);
    char *file_path = malloc(sizeof(char) * PATH_MAX);
    check_malloc(file_path);
    struct dirent *file_dir = readdir(dir);
    struct stat *statbuf = malloc(sizeof(struct stat));
    check_malloc(statbuf);

    while (file_dir != NULL) {
        // check if file_dir refers to a file starting with sendme_
        if (file_dir->d_type == DT_REG &&
                strncmp(file_dir->d_name, "sendme_", 7) == 0) {

            // creating file_path string
            strcpy(file_path, buf);
            strcat(strcat(file_path, "/"), file_dir->d_name);

            // retrieving file stats
            if (stat(file_path, statbuf) == -1)
                errExit("Could not retrieve file stats");

            // check file size (4KB -> 4096)
            if (statbuf->st_size <= 4096) {
                // allocate memory for file_path
                to_send[count] = malloc(sizeof(char *) * strlen(file_path));
                check_malloc(to_send[count]);
                // saving file_path
                strcpy(to_send[count], file_path);
                count++;
            }
        }

        // check if file_dir refers to a directory
        if (file_dir->d_type == DT_DIR &&
                strcmp(file_dir->d_name, ".") != 0 &&
                strcmp(file_dir->d_name, "..") != 0) {
            // creating file_path string
            strcpy(file_path, buf);
            strcat(strcat(file_path, "/"), file_dir->d_name);

            // recursive call
            count = search_dir(file_path, to_send, count);
        }

        file_dir = readdir(dir);
    }
    
    free(statbuf);
    free(file_path);

    if (closedir(dir) == -1)
        errExit("Error while closing directory");
    
    return count;
}

// function used to reset the default mask of the process
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