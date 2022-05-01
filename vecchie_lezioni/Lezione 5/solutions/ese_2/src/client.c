#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/msg.h>

#include "order.h"
#include "errExit.h"

// The readInt reads a integer value from an array of chars
// It checks that only a number n is provided as an input parameter,
// and that n is greater than 0
int readInt(const char *s) {
    char *endptr = NULL;

    errno = 0;
    long int res = strtol(s, &endptr, 10);

    if (errno != 0 || *endptr != '\n' || res < 0) {
        printf("invalid input argument\n");
        exit(1);
    }

    return res;
}

int main (int argc, char *argv[]) {
    // check command line input arguments
    if (argc != 2) {
        printf("Usage: %s message_queue_key\n", argv[0]);
        exit(1);
    }

    // read the message queue key defined by user
    int msgKey = atoi(argv[1]);
    if (msgKey <= 0) {
        printf("The message queue key must be greater than zero!\n");
        exit(1);
    }

    // get the message queue identifier
    int msqid = msgget(msgKey, S_IRUSR | S_IWUSR);
    if (msqid == -1)
        errExit("msgget failed");

    char buffer[10];
    size_t len;

    // crea an order data struct
    struct order order;

    // setting the order's type
    printf("Are you a prime user[y/n]: ");
    fgets(buffer, sizeof(char) * 3, stdin); // [y|n] + \n + \0

    if (strlen(buffer) != 2
        || (buffer[0] != 'y' && buffer[0] != 'n')) {
        printf("invalid input argument");
        exit(0);
    }

    order.mtype = (buffer[0] == 'y')? 1 : 2;

    // read the code of the client's order
    printf("Insert order code: ");
    fgets(buffer, sizeof(buffer), stdin);
    order.code = readInt(buffer);

    // read a description of the order
    printf("Insert a description: ");
    fgets(order.description, sizeof(order.description), stdin);
    len = strlen(order.description);
    order.description[len - 1] = '\0';

    // read a quantity
    printf("Insert quantity: ");
    fgets(buffer, sizeof(buffer), stdin);
    order.quantity = readInt(buffer);

    // read an e-mail
    printf("Insert an e-mail: ");
    fgets(order.email, sizeof(order.email), stdin);
    len = strlen(order.email);
    order.email[len - 1] = '\0';

    // send the order to the server through the message queue
    printf("Sending the order...\n");
    size_t mSize = sizeof(struct order) - sizeof(long);
    if (msgsnd(msqid, &order, mSize, 0) == -1)
        errExit("msgsnd failed");

    printf("Done\n");
    return 0;
}
