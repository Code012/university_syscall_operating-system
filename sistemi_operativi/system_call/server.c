/// @file sender_manager.c
/// @brief Contiene l'implementazione del sender_manager.

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"
#include <sys/msg.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h> 
#include <sys/shm.h>
#include <stdio.h>

// Declaration of functions
void sigHandler (int sig);

//init variables
int semid;
int queue_id;
int fifo1_fd;
int fifo2_fd;
int shmem_id;
int n_files;    // files to read from FIFO
ssize_t num_read;
struct queue_msg *shmpointer;
union semun semarg;
pid_t client_pid;

int main(int argc, char * argv[]) {

    // sem order: Access, FIFO1, FIFO2, MsgQueue, ShdMem, Finish
    unsigned short semarray[6] = {0, 0, 1, 1, 0, 1};
    semarg.array = semarray;

    // creation of all the semaphores
    semid = semget_usr(ftok("client_0", 'a'), 6, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

    if (semctl(semid, 0, SETALL, semarg) == -1)
        errExit("Error while initializing semaphore set");

    // creation of all IPC's
    create_fifo("FIFO1");
    create_fifo("FIFO2");
    queue_id = msgget(ftok("client_0", 'a'), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if(queue_id == -1)
        errExit("Error while creating a message queue");
    shmem_id = alloc_shared_memory(ftok("client_0", 'a'),
                                    sizeof(struct queue_msg) * 50,
                                    IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

    // unlocking semaphore 0 (all IPCs have been created)
    semop_usr(semid, 0, 1);

    // opening and attaching all of the IPC
    fifo1_fd = open_fifo("FIFO1", O_RDONLY);
    fifo2_fd = open_fifo("FIFO2", O_RDONLY);
    shmpointer = (struct  queue_msg *) attach_shared_memory(shmem_id, 0);

    // Set sigHandler as a handler for the SIGINT
    if (signal(SIGINT, sigHandler) == SIG_ERR)
            errExit("change signal handler (SIGINT) failed!");

    while(1){

        // reset all the semaphores to restart the cicle
        if (semctl(semid, 0, SETALL, semarg) == -1)
            errExit("Error while initializing semaphore set");

        // lock first semaphore until the number of files are written on FIFO1
        printf("Waiting for client...\n");
        semop_usr(semid, 1, -1);

        // retrieve n_files from FIFO1
        read_fifo(fifo1_fd, &n_files, sizeof(int));
        read_fifo(fifo1_fd, &client_pid, sizeof(pid_t));

        strcpy(shmpointer[0].fragment, "READY");
        semop_usr(semid, 4, 2);

        printf("Numeri di file letti dalla FIFO: %d\n", n_files);

        //Wait for client to finish
        semop_usr(semid, 5, 0);
    }
}

void sigHandler (int sig) {
    // when SIGINT is caught, close everything and send SIGUSR1 to client
    if(sig == SIGINT) {
        
        /**************
        * CLOSE IPCs *
        **************/ 
        close(fifo1_fd);
        close(fifo2_fd);
        free_shared_memory(shmpointer);

        unlink("FIFO1");
        unlink("FIFO2");
        if(msgctl(queue_id, IPC_RMID, NULL) == -1)
            errExit("Error while removing the message queue");
        if(semctl(semid, 0, IPC_RMID, NULL) == -1)
            errExit("Error while removing the message queue");
        remove_shared_memory(shmem_id);

        if(kill(client_pid, SIGUSR1) == -1){
            errExit("Error while sending SIGUSR1 to server");
        }

        exit(0);
    }
}

