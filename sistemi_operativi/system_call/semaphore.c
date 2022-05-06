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
    // initialize the struct to check value 0 of a semaphore (flag 0)
    // if the value of semaphore isn't zero,
    // the process will block until the semaphore = 0
    struct sembuf sop = {
        .sem_num = sem_num,
        .sem_op = sem_op,
        .sem_flg = 0
    };

    // open a semaphore
    if(semop(semid, &sop, 1) == -1)
        errExit("semop failed!\n");
}