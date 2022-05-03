/// @file fifo.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione delle FIFO.

#include <sys/stat.h>
#include <sys/types.h>
#include "err_exit.h"
#include "fifo.h"

void create_fifo (const char *pathname, mode_t permissions) {
    if (mkfifo(pathname, permissions) == -1)
        errExit("mkfifo failed!\n");
}

int open_fifo (const char *pathname, int flags) {
    if (open(pathname, flags) == -1)
        errExit("open_fifo failed!\n");
}

void read_fifo (int fd, char *buf, size_t bytes_to_read) {
    ssize_t charRead = read(fd, buf, bytes_to_read);

    if (charRead == -1)
        errExit("read_fifo failed!\n");

    // !!!
    // Dobbiamo verificare che il numero di caratteri letti
    // sia uguale a al numero di caratteri che abbiamo richiesto di leggere?
    // Se sì, nel caso in cui il numero dei caratteri letti sia minore di quello richiesto
    // bisogna anche controllare la lunghezza del file
    // P.S. non si ritorna niente perché buf è un puntatore!
    // !!!
}