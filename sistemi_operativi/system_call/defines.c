/// @file defines.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche del progetto.

#include <string.h>
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
    char dir_path[MAX_LENGTH_PATH];
    strcpy(dir_path, buf);
    struct dirent *file_dir = readdir(dir);
    struct stat statbuf;

    while (file_dir != NULL) {
        // Check if file_dir refers to a file starting with sendme_
        if (file_dir->d_type == DT_REG && strncmp(file_dir->d_name, "sendme_", 7) == 0) {

            // printf("prima: %s\n", buf);
            // printf("prima: %s\n", file_path);

            // creating file_path string
            strcpy(file_path, dir_path);
            strcat(strcat(file_path, "/"), file_dir->d_name);

            // retrieving file stats
            if (stat(file_path, &statbuf) == -1)
                errExit("Could not retrieve file stats");

            // printf("\n---------------\n");

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
            strcpy(file_path, dir_path);
            strcat(strcat(file_path, "/"), file_dir->d_name);

            // recursive call
            count = search_dir(file_path, to_send, count);

            // printf("Ben tornato: %s\n\n\n", dir_path);
        }

        file_dir = readdir(dir);
    }

    if (closedir(dir) == -1)
        errExit("Error while closing directory");

    // printf("Dir path: %s\n", buf);
    
    return count;
}

// Function to initialize the message struct for commodity

struct queue_msg *init_struct(long mtype, pid_t pid, char *pathname, char *fragment){
    struct queue_msg *message = malloc(sizeof(struct queue_msg));
    check_malloc(message);

    message->mtype = mtype;

    message->pid = pid;
    
    strcpy(message->pathname, pathname);
    strcpy(message->fragment, fragment);

    return message;
}

// Function to initialize th output struct for commodity

void init_output(struct to_save output[], int n) {
    for (int i = 0 ; i < n ; i++) {
        strcpy(output[i].fragment1, "");
        strcpy(output[i].fragment2, "");
        strcpy(output[i].fragment3, "");
        strcpy(output[i].fragment4, "");
    }
}

// Check that every fragment is not ""
bool check_frags(struct to_save output) {
    return strcmp(output.fragment1, "") && strcmp(output.fragment2, "") && strcmp(output.fragment3, "") && strcmp(output.fragment4, "");
}

// Create path for _out files
char *gen_out_path(char pathname[]) {
    char *pathcpy = malloc(sizeof(char) * MAX_LENGTH_PATH);
    strcpy(pathcpy, pathname);
    char *extcpy = NULL;
    char *ext = strrchr(pathcpy, '.');

    if(ext != NULL) {
        //Gimmy/madoska.txt
        strcpy(extcpy, ext);
        //Gimmy/madoska.txt     .txt
        strcpy(ext, "_out");
        //Gimmy/madoska_out     .txt
        strcat(pathcpy, extcpy);
        //Gimmy/madoska_out.txt
    } else {
        //Gimmy/madoska
        strcat(pathcpy, "_out");
        //Gimmy/madoska_out
    }

    printf("\nIl file di output è: %s\n", pathcpy);

    return pathcpy;
}