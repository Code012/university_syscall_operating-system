#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "errExit.h"

// the FIFO pathname
char *path2ServerFIFO;
// the file descriptor for the FIFO
int serverFIFO;

int main (int argc, char *argv[]) {
    // Check command line input arguments
    // The program only wants a FIFO pathname
    if (argc != 2) {
        printf("Usage: %s fifo_pathname\n", argv[0]);
        return 1;
    }

    // read the FIFO's pathname
    path2ServerFIFO = argv[1];

    printf("<Server> Making FIFO...\n");
    // make a FIFO with the following permissions:
    // user:  read, write
    // group: write
    // other: no permission
    if (mkfifo(/*....*/) == /*...*/)
        errExit("mkfifo failed");

    printf("<Server> FIFO %s created!\n", path2ServerFIFO);

    // Wait for clients in read-only mode. The open blocks the calling process
    // until another process opens the same FIFO in write-only mode
    printf("<Server> waiting for a client...\n");
    // ...

    int v [] = {0, 0};
    printf("<Server> waiting for vector [a,b]...\n");
    // Reading 2 integers from the FIFO.
    // ...

    // Checking the number of bytes from the FIFO
    if (/*...*/)
        printf("<Server> it looks like the FIFO is broken");
    if (/*...*/)
        printf("<Server> it looks like I did not receive 2 numbers");
    else
        printf("<Server> %d is %s %d\n", v[0],
        (v[0] < v[1])? "lower than" : "greater/equals to", v[1]);

    // Close the FIFO
    // ...

    printf("<Server> removing FIFO...\n");
    // Remove the FIFO
    // ...
}
