/// @file shared_memory.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione della MEMORIA CONDIVISA.

#include "err_exit.h"
#include "shared_memory.h"
#include <sys/shm.h>
#include <sys/stat.h>

 int alloc_shared_memory(key_t shmKey, size_t size) {
    // get, or create, a shared memory segment

    int shmid = shmget(shmKey, size, IPC_CREAT | S_IRUSR | S_IWUSR);

    if(shmid == -1)
        errExit("Could not allocate shared memory");

    return shmid;
}

void *get_shared_memory(int shmid, int shmflg) {
    // attach the shared memory
    int *ptr_to_shmem = (int *)shmat(shmid, NULL, shmflg);

    return ptr_to_shmem;
}

void free_shared_memory(void *ptr_sh) {
    // detach the shared memory segments
    shmdt(ptr_sh);
}

void remove_shared_memory(int shmid) {
    // delete the shared memory segment
    if(shmctl(shmid, IPC_RMID, NULL) == -1)
        errExit("could not detach shared memory");
}