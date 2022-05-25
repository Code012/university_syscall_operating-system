/// @file sender_manager.c
/// @brief Contiene l'implementazione del sender_manager.

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <unistd.h>
#include <fcntl.h> 
#include <sys/shm.h>
#include <stdio.h>

// Declaration of functions
void sigHandler (int sig);

// init variables
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
int opened;


int main(int argc, char * argv[]) {

    struct queue_msg packet;

    // set flag to zero (never say never)
    opened = 0;

    // sem order: Access, FIFO1, FIFO2, MsgQueue, ShdMem, FINISH_CLIENT
    unsigned short semarray[6] = {0, 0, 1, 1, 0, 0};
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
                                    sizeof(struct queue_msg) * IPC_MAX,
                                    IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

    // set sigHandler as a handler for SIGINT
    if (signal(SIGINT, sigHandler) == SIG_ERR)
        errExit("change signal handler (SIGINT) failed!");

    // opening and attaching all the IPCs
    fifo1_fd = open_fifo("FIFO1", O_RDONLY | O_NONBLOCK);
    fifo2_fd = open_fifo("FIFO2", O_RDONLY | O_NONBLOCK);
    shmpointer = (struct  queue_msg *) attach_shared_memory(shmem_id, 0);

    // unlocking semaphore ACCESS (all IPCs have been created)
    semop_usr(semid, ACCESS, 1);

    printf("Server ready!\n");

    // wait for client to open IPCs
    semop_usr(semid, FINISH_CLIENT, -1);

    // set flag to one, this tells the server to close all IPCs when receiving SIGINT
    opened = 1;

    while(1) {

        // reset all the semaphores to restart the cicle
        // Access, FIFO1, FIFO2, MsgQueue, ShdMem, FINISH_CLIENT
        // 0,      0,     1,     1,        0,      0             
        if (semctl(semid, 0, SETALL, semarg) == -1)
            errExit("Error while initializing semaphore set");

        // lock first semaphore until the number of files are written on FIFO1
        printf("\nWaiting for client...\n\n");
        semop_usr(semid, FIFO1, -1);

        // retrieve n_files from FIFO1 and then retrieve client PID
        read_fifo(fifo1_fd, &n_files, sizeof(int));
        read_fifo(fifo1_fd, &client_pid, sizeof(pid_t));

        strcpy(shmpointer[0].fragment, "READY\0");
        semop_usr(semid, SHDMEM, 1);

        printf("Numero di file da elaborare: %d\n", n_files);

        // Array of structs used to save all files fragments
        struct to_save output[n_files];

        init_output(output, n_files);

        for(int i = 0 ; i < n_files ;) {

            // Retrieve message from FIFO1
            read_fifo(fifo1_fd, &packet, sizeof(packet));
            if (errno == 0) {

                //printf("FIFO1 process PID: %d, Message: %s\n", packet.pid, packet.fragment);

                // saving pid and pathname only once since they're the same
                output[packet.mtype - 1].pid = packet.pid;
                strcpy(output[packet.mtype - 1].pathname, packet.pathname);
                // saving fragment 1
                strcpy(output[packet.mtype - 1].fragment1, packet.fragment);

                semop_usr(semid, FIFO1, 1);
            }

            // Retrieve message from FIFO2
            read_fifo(fifo2_fd, &packet, sizeof(packet));
            if (errno == 0) {

                //printf("FIFO2 process PID: %d, Message: %s\n", packet.pid, packet.fragment);

                // saving fragment 2
                strcpy(output[packet.mtype - 1].fragment2, packet.fragment);

                semop_usr(semid, FIFO2, 1);
            }

            // Retrieve message from MSGQUEUE
            errno = 0;
            msgrcv(queue_id, &packet, sizeof(struct queue_msg) - sizeof(long), 0, IPC_NOWAIT);
            if (errno == 0) {

                //printf("MSGQUEUE process PID: %d, Message: %s\n", packet.pid, packet.fragment);

                // saving fragment 3
                strcpy(output[packet.mtype - 1].fragment3, packet.fragment);

                semop_usr(semid, MSGQUEUE, 1);
            }
            else if (errno != ENOMSG) {
                errExit("Error while receiving message");
            }

            // Retrieve message from SHMEM
            semop_nowait(semid, SHDMEM, -1);
            if(errno == 0){
                for(int k = 0, read = 0; k < IPC_MAX && read == 0; k++){
                    if(shmpointer[k].mtype != 0){
                                                // saving fragment 4
                        strcpy(output[shmpointer[k].mtype - 1].fragment4, shmpointer[k].fragment);

                        //printf("SHDMEM process PID: %d, Message: %s\n", shmpointer[k].pid, shmpointer[k].fragment);

                        shmpointer[k].mtype = 0;
                        read++;
                    }
                }
                semop_nowait(semid, SHDMEM, 1);
            }

            // Creating files _out
            for (int k = 0 ; k < n_files ; k++) {

                if (check_frags(output[k])) {
                    // Creating path for _out files
                    char *out_path = gen_out_path(output[k].pathname);
                    char temp_int[30];
                    char to_write[MAX_LENGTH_PATH + 100];
                                      
                    int fp = open(out_path, O_WRONLY | O_TRUNC | O_CREAT , S_IRWXU | S_IRWXG | S_IRWXO);
                    if(fp == -1)
                        errExit("Errore nella creazione del file");

                    // Writing first fragment
                    strcpy(to_write, "[Parte 1, del file ");
                    strcat(to_write, output[k].pathname);
                    strcat(to_write, ", spedita dal processo ");
                    sprintf(temp_int, "%d" ,output[k].pid);
                    strcat(to_write, temp_int);
                    strcat(to_write, " tramite FIFO1]\n");
                    write(fp, to_write, strlen(to_write));
                    write(fp, output[k].fragment1, strlen(output[k].fragment1));
                    // Writing second fragment
                    strcpy(to_write, "\n\n[Parte 2, del file ");
                    strcat(to_write, output[k].pathname);
                    strcat(to_write, ", spedita dal processo ");
                    sprintf(temp_int, "%d" ,output[k].pid);
                    strcat(to_write, temp_int);
                    strcat(to_write, " tramite FIFO2]\n");
                    write(fp, to_write, strlen(to_write));
                    write(fp, output[k].fragment2, strlen(output[k].fragment2));
                    // Writing third fragment
                    strcpy(to_write, "\n\n[Parte 3, del file ");
                    strcat(to_write, output[k].pathname);
                    strcat(to_write, ", spedita dal processo ");
                    sprintf(temp_int, "%d" ,output[k].pid);
                    strcat(to_write, temp_int);
                    strcat(to_write, " tramite MsgQueue]\n");
                    write(fp, to_write, strlen(to_write));
                    write(fp, output[k].fragment3, strlen(output[k].fragment3));
                    // Writing fourth fragment
                    strcpy(to_write, "\n\n[Parte 4, del file ");
                    strcat(to_write, output[k].pathname);
                    strcat(to_write, ", spedita dal processo ");
                    sprintf(temp_int, "%d" ,output[k].pid);
                    strcat(to_write, temp_int);
                    strcat(to_write, " tramite ShdMem]\n");
                    write(fp, to_write, strlen(to_write));
                    write(fp, output[k].fragment4, strlen(output[k].fragment4));

                    close(fp);
                    
                    strcpy(output[k].fragment1, "\0");
                    // freeing malloc
                    free(out_path);

                    i++;
                }
            }

        }
        
        // Sending message to confirm end of operations
        packet.mtype = 666;
        strcpy(packet.fragment, "END");
        msgsnd(queue_id, &packet, sizeof(struct queue_msg) - sizeof(long), 0);

    }
}

void sigHandler (int sig) {
    // when SIGINT is caught, close everything and send SIGUSR1 to client
    if(sig == SIGINT) {
        printf("\nShutdown server...\n");

        if (opened) {
            // close IPCs
            close_fifo(fifo1_fd);
            close_fifo(fifo2_fd);
            free_shared_memory(shmpointer);
        }

        unlink("FIFO1");
        unlink("FIFO2");
        if(msgctl(queue_id, IPC_RMID, NULL) == -1)
            errExit("Error while removing the message queue");
        if(semctl(semid, 0, IPC_RMID, NULL) == -1)
            errExit("Error while removing the message queue");
        remove_shared_memory(shmem_id);

        // send SIGUSR1 to terminate client_0
        if(kill(client_pid, SIGUSR1) == -1)
            errExit("Error while sending SIGUSR1 to server");

        exit(0);
    }
}