#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "producer.h"
#include "errExit.h"

#define MSG_BYTES 100

struct Item {
    ssize_t size;
    char value [MSG_BYTES];
};

void producer (int *pipeFD, const char *filename) {
    // Close the read-end of the pipe
    if (close(pipeFD[0]) == -1)
        errExit("close failed");

    printf("<Producer> text file: %s\n", filename);

    // Open the text file in read only mode
    int file = open(filename, O_RDONLY);
    if (file == -1)
        errExit("open failed");

    struct Item item;
    do {
        // read a chunk of bytes from file:
        // copy in item.value a chunk of bytes from the file, and
        // set item.size equal to the number of read bytes
        item.size = read(file, item.value, sizeof(item.value));
        if (item.size == -1)
            errExit("read failed");

        if (item.size > 0) {
            // What is the righ size?
            // (size of a ssize_t and the number of read chars)
            ssize_t bytes2send = sizeof(item.size) + item.size;
            // send struct item to Consumer through pipe
            ssize_t bW = write(pipeFD[1], &item, bytes2send);
            // check bW value
            if (bW != bytes2send)
                errExit("write failed");
        }
    } while (item.size > 0);

    // Close the write end of the pipe and the file
    if (close(pipeFD[1]) == -1 || close(file) == -1)
        errExit("close failed");
}
