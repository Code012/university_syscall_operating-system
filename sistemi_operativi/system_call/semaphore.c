/// @file semaphore.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione dei SEMAFORI.

#include "err_exit.h"
#include "semaphore.h"


int semget_usr (key_t key, int nsems, int flags){
    int semid = semget(key, nsems, flags);

    if(semid == -1)
        errExit("Error while creating the semaphore");

    return semid;
}

void semop_usr (int semid, unsigned short sem_num, short sem_op) {
    // initialize the struct
    struct sembuf sop = {
        .sem_num = sem_num,
        .sem_op = sem_op,
        .sem_flg = 0
    };

    // open a semaphore
    if(semop(semid, &sop, 1) == -1)
        errExit("semop failed");
}

void semop_nowait (int semid, unsigned short sem_num, short sem_op) {
    // initialize the struct with flag = IPC_NOWAIT
    struct sembuf sop = {
        .sem_num = sem_num,
        .sem_op = sem_op,
        .sem_flg = IPC_NOWAIT
    };
    
    errno = 0;

    // open a semaphore
    semop(semid, &sop, 1);

    // check errors
    if(errno != EAGAIN && errno != 0)
        errExit("semop nowait failed");
}