/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del progetto.

#pragma once

#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/wait.h>

#define MAX_LENGTH_PATH 150
#define MAX_FILES 100

// the my_msg structure defines a file fregment sent by a client
struct queue_msg {
    long mtype;
    pid_t pid;
    char pathname[MAX_LENGTH_PATH + 1]; // one more char to compensate for string terminator
    char fragment[1025];                // one more char to compensate for string terminator
    // DA PROVARE TERMINATORE
};

void check_malloc (void *pointer);
int search_dir (char buf[], char to_send[][MAX_LENGTH_PATH], int count);

/*
struct general_msg {
    pid_t pid;
    char pathname[250];     // assuming pathnames shorter than 250 Bytes
    char fragment[1001];    // one more char to compensate for string terminator
}
*/