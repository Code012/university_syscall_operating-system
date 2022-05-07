/// @file fifo.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione delle FIFO.

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>     
#include <stdio.h>
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

ssize_t read_fifo (int fd, void *buf, ssize_t bytes_to_read) {
    ssize_t char_read = read(fd, buf, bytes_to_read);

    if (char_read == -1)
        errExit("read_fifo failed!");

    // check if read was succesfull
    if (char_read != bytes_to_read && char_read != 0)
        errExit("broken fifo while reading error!");

    return char_read;
}

void write_fifo (int fd, void *buf, ssize_t bytes_to_write) {
    ssize_t char_write = write(fd, buf, bytes_to_write); 

    if (char_write == -1)
        errExit("write_fifo failed!");

    // check if write was succesfull
    if (char_write != bytes_to_write)
        errExit("broken fifo while writing error!");
}