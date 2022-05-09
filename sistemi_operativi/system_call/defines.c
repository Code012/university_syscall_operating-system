/// @file defines.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche del progetto.

#include <stdlib.h>
#include "defines.h"
#include "err_exit.h"

void check_malloc (void *pointer) {
    if (pointer == NULL)
        errExit("Malloc failed");
}