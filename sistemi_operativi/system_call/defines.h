/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del progetto.

#pragma once

#include <sys/types.h>

// the my_msg structure defines a file fregment sent by a client
struct queue_msg {
    long mtype;
    pid_t pid;
    char pathname[150];     // assuming pathnames shorter than 150 Bytes
    char fragment[1025];    // one more char to compensate for string terminator
    // DA PROVARE TERMINATORE
};

/*
struct general_msg {
    pid_t pid;
    char pathname[250];     // assuming pathnames shorter than 250 Bytes
    char fragment[1001];    // one more char to compensate for string terminator
}
*/