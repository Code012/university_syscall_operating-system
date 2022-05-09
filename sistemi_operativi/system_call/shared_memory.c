/// @file shared_memory.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione della MEMORIA CONDIVISA.

#include <sys/shm.h>
#include <sys/stat.h>

#include "err_exit.h"
#include "shared_memory.h

int alloc_shared_memory(key_t shmKey, size_t size, int flags) {
    // get, or create, a shared memory segment
    int shmid = shmget(shmKey, size, flags);

    if(shmid == -1)
        errExit("could not allocate shared memory");

    return shmid;
}

void *attach_shared_memory(int shmid, int shmflg) {
    // attach the shared memory
    void *ptr_to_shmem = shmat(shmid, NULL, shmflg);

    if(ptr_to_shmem == (void *)-1)
        errExit("could not attach shared memory");

    return ptr_to_shmem;
}

void free_shared_memory(void *ptr_sh) {
    // detach the shared memory segments
    if(shmdt(ptr_sh) == -1)
        errExit("could not detach shared memory");
}

void remove_shared_memory(int shmid) {
    // delete the shared memory segment
    if(shmctl(shmid, IPC_RMID, NULL) == -1)
        errExit("could not detach shared memory");
}