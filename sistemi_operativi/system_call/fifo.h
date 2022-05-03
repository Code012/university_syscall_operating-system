/// @file fifo.h
/// @brief Contiene la definizioni di variabili e
///         funzioni specifiche per la gestione delle FIFO.

#pragma once

// The create_fifo create a fifo or
// terminate process if there is an error
void create_fifo (const char *pathname, mode_t permissions); // !!! Da provare se funziona permissions = S_IRUSR | S_IWUSR (esempio) !!!

// Open a fifo and return a file descriptor,
// terminate process if there is an error
int open_fifo(const char *pathname, int flags); // !!! Si può ottimizzare dato che alla fin fine la open non la usa solo la FIFO, quindi si potrebbe definire in define.h !!!

// Read a fifo or
// terminate process if there is an error
void read_fifo(int fd, char *buf, size_t bytes_to_read);