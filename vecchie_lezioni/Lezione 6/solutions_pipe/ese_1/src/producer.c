#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "producer.h"
#include "errExit.h"

#define MSG_BYTES 100

void producer (int *pipeFD, const char *filename) {
    // Close the read-end of the pipe
    if (close(pipeFD[0]) == -1)
        errExit("close failed");

    printf("<Producer> text file: %s\n", filename);

    // open filename for reading only
    int file = open(filename, O_RDONLY);
    if (file == -1)
        errExit("open failed");

    char buffer[MSG_BYTES];
    ssize_t bR, bW = -1;
    do {
        // read max MSG_BYTES chars from the file
        bR = read(file, buffer, sizeof(buffer));
        if (bR == -1)
            errExit("read failed");

        if (bR > 0) {
            // write bR chars to the pipe
            bW = write(pipeFD[1], buffer, bR);
            if (bW != bR)
                errExit("write failed");
        }
    } while (bR > 0);

    // Close the write end of the pipe and pipe
    if (close(pipeFD[1]) == -1 || close(file) == -1)
        errExit("close failed");
}
