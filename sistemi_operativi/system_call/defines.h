/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del progetto.

#pragma once

#include <sys/types.h>
#include <linux/limits.h>

#define MAX_LENGTH_PATH 150

// the my_msg structure defines a file fregment sent by a client
struct queue_msg {
    long mtype;
    pid_t pid;
    char pathname[MAX_LENGTH_PATH + 1]; // one more char to compensate for string terminator
    char fragment[1025];                // one more char to compensate for string terminator
    // DA PROVARE TERMINATORE
};

void check_malloc (void *pointer);

/*
struct general_msg {
    pid_t pid;
    char pathname[250];     // assuming pathnames shorter than 250 Bytes
    char fragment[1001];    // one more char to compensate for string terminator
}
*/