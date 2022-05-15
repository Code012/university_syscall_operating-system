/// @file semaphore.h
/// @brief Contiene la definizioni di variabili e funzioni
///         specifiche per la gestione dei SEMAFORI.

#pragma once

#ifndef SEMUN_H
#define SEMUN_H
#include <sys/sem.h>
#include <errno.h>

// definition of the union semun
union semun {
    int val;
    struct semid_ds * buf;
    unsigned short * array;
};

// Definition of semaphore functions
int semget_usr (key_t key, int nsems, int flags);
void semop_usr (int semid, unsigned short sem_num, short sem_op);
void semop_nowait (int semid, unsigned short sem_num, short sem_op);

#endif