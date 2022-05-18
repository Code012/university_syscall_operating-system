/// @file fifo.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione delle FIFO.

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>     
#include <stdio.h>
#include <sys/errno.h>
#include "err_exit.h"
#include "fifo.h"

void create_fifo (const char *pathname) {
    if (mkfifo(pathname, O_CREAT | S_IRUSR | S_IWUSR) == -1)
        errExit("mkfifo failed!");
}

int open_fifo (const char *pathname, int flags) {
    // file descriptor
    int fd = open(pathname, flags);

    if (fd == -1)
        errExit("open_fifo failed!");

    return fd;
}

void read_fifo (int fd, void *buf, ssize_t bytes_to_read) {
    errno = 0;

    ssize_t char_read = read(fd, buf, bytes_to_read);

    // check if write was succesfull
    if(errno == EAGAIN || errno == EWOULDBLOCK)
        return;

    if (char_read == -1)
        errExit("write_fifo failed!");

    if (char_read != bytes_to_read)
        errExit("broken fifo while writing!");
}

void write_fifo (int fd, void *buf, ssize_t bytes_to_write) {
    errno = 0;

    ssize_t char_write = write(fd, buf, bytes_to_write); 

    //printf("%ld e %ld\n\n", char_write, bytes_to_write);

    // check if write was succesfull
    if(errno == EAGAIN || errno == EWOULDBLOCK)
        return;

    if (char_write == -1)
        errExit("write_fifo failed!");

    if (char_write != bytes_to_write)
        errExit("broken fifo while writing!");
}

void close_fifo (int fifo_fd) {
    if(close(fifo_fd) == -1)
        errExit("Error while closing fifo");
}