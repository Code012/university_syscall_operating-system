/// @file defines.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche del progetto.

#include "defines.h"
#include "err_exit.h"

void check_malloc (void *pointer) {
    if (pointer == NULL)
        errExit("Malloc failed");
}

// Function that recursively searches files in the specified directory
int search_dir (char buf[], char to_send[][MAX_LENGTH_PATH], int count) {
    // Structs and variables
    DIR *dir = opendir(buf);
    char file_path[MAX_LENGTH_PATH];
    struct dirent *file_dir = readdir(dir);
    struct stat statbuf;

    while (file_dir != NULL) {
        // Check if file_dir refers to a file starting with sendme_
        if (file_dir->d_type == DT_REG && strncmp(file_dir->d_name, "sendme_", 7) == 0) {

            // creating file_path string
            strcpy(file_path, buf);
            strcat(strcat(file_path, "/"), file_dir->d_name);

            // retrieving file stats
            if (stat(file_path, &statbuf) == -1)
                errExit("Could not retrieve file stats");

            // Check file size (4KB -> 4096)
            if (statbuf.st_size <= 4096) {
                // saving file_path
                strcpy(to_send[count], file_path);
                count++;
            }
        }

        // check if file_dir refers to a directory
        if (file_dir->d_type == DT_DIR && strcmp(file_dir->d_name, ".") != 0 && strcmp(file_dir->d_name, "..") != 0) {
            // creating file_path string
            strcpy(file_path, buf);
            strcat(strcat(file_path, "/"), file_dir->d_name);

            // recursive call
            count = search_dir(file_path, to_send, count);
        }

        file_dir = readdir(dir);
    }

    if (closedir(dir) == -1)
        errExit("Error while closing directory");
    
    return count;
}

// Function to initialize the message struct for commodity

struct queue_msg init_struct(long mtype, pid_t pid, char *pathname, char *fragment){
    struct queue_msg message = {.mtype = mtype, .pid = pid};
    strcpy(message.pathname, pathname);
    strcpy(message.fragment, fragment);

    return message;
}