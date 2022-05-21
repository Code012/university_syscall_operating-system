/// @file client.c
/// @brief Contiene l'implementazione del client.

#include "defines.h"
#include "err_exit.h"
#include "fifo.h"
#include "semaphore.h"
#include "shared_memory.h"

// set of signals
sigset_t original_set_signals;
sigset_t unlocked_set_signals;
sigset_t blocked_set_signals;

// manipulation of a signal & mod signal mask
void sigHandler(int sig);
void set_original_mask();
void create_signal_mask();

// global variables
int fifo1_fd;
int fifo2_fd;
int queue_id;
int shmem_id;
struct queue_msg *shmpointer;
pid_t client_pid;

int main(int argc, char * argv[]) {
    
    // Init variables
    int semid;
    pid_t pid = 1;
    int child_num;
    client_pid = getpid();                          // get the current pid to send to server
    int count = 0;                                  // file found counter
    char to_send[MAX_FILES][MAX_LENGTH_PATH];       // creating a matrix to store the file paths
    char buf[MAX_LENGTH_PATH];                      // creating an array to store current directory
    struct stat statbuf;
    int files_dim[4] = {0};
    int file_descriptor;
    char char_to_read[4][1025];
    struct queue_msg *packet;
    union semun semarg;
    





    /**************
     * CHECK ARGS *
     **************/

    if (argc != 2)
        errExit("Intended usage: ./client_0 <HOME>/myDir/");
    
    // pointer to the inserted path
    char *path_to_dir = argv[1];

    // try to obtain set id of semaphores
    // if the client is unable to find it,
    // we leave the original masks so the user could kill the process with a CTRL + C
    do {
        printf("Looking for the semaphore...\n\n");
        errno = 0;
        semid = semget(ftok("client_0", 'a'), 0, S_IRUSR | S_IWUSR  | S_IRGRP | S_IWGRP);
        if (errno == ENOENT){
            sleep(2);
        }
        else if(semid == -1)
            errExit("Error while retrieving the semaphore");
    } while(semid == -1);

    // invoke fun to create new set of masks
    create_signal_mask();

    // waiting for IPCs to be created
    semop_usr(semid, ACCESS, -1);

    // definition manipulate SIGINT
    if (signal(SIGINT, sigHandler) == SIG_ERR)
        errExit("change signal handler (SIGINT) failed!");

    // definition manipulate SIGUSR1
    if (signal(SIGUSR1, sigHandler) == SIG_ERR)
        errExit("change signal handler (SIGUSR1) failed!");





    /*****************
     * OBTAINING IDs *
     *****************/
    fifo1_fd = open_fifo("FIFO1", O_WRONLY | O_NONBLOCK);
    fifo2_fd = open_fifo("FIFO2", O_WRONLY | O_NONBLOCK);
    queue_id = msgget(ftok("client_0", 'a'), S_IRUSR | S_IWUSR);
    shmem_id = alloc_shared_memory(ftok("client_0", 'a'), sizeof(struct queue_msg) * IPC_MAX, S_IRUSR | S_IWUSR);
    shmpointer = (struct  queue_msg *) attach_shared_memory(shmem_id, 0);

    // Unlocking finish (IPCs opened)
    semop_usr(semid, FINISH_CLIENT, 1);





    /*************
     * MAIN LOOP *
     *************/
    
    // change process working directory
    if (chdir(path_to_dir) == -1)
        errExit("Error while changing directory");

    while(1) {
        // set the count to 0 everytime so the recursive function works alright
        count = 0;

        // blocking all signals except:
        // SIGKILL, SIGSTOP (default)
        // SIGINT, SIGUSR1
        if(sigprocmask(SIG_SETMASK, &unlocked_set_signals, NULL) == -1)
            errExit("sigprocmask(new_set) failed!");

        printf("Ready to go! press ctrl + c\n");

        // waiting for a signal...
        pause();

        // blocking all blockable signals
        if(sigprocmask(SIG_SETMASK, &blocked_set_signals, NULL) == -1)
            errExit("sigprocmask(original_set) failed");




        /****************
         * FILE READING *
         ****************/
        
        // get current working directory
        getcwd(buf, MAX_LENGTH_PATH);

        printf("Ciao %s, ora inizio l’invio dei file contenuti in %s\n\n", getenv("USER"), buf);

        // search files into directory
        count = search_dir (buf, to_send, count);





        /*******************************
         * CLIENT-SERVER COMMUNICATION *
         *******************************/

        // writing n_files on FIFO1
        write_fifo(fifo1_fd, &count, sizeof(count));
        write_fifo(fifo1_fd, &client_pid, sizeof(pid_t));

        // unlocking semaphore 1 (allow server to read from FIFO1)
        semop_usr(semid, FIFO1, 2);

        // wait for server to send "READY", 
        // waiting ShdMem semaphore unlock by server 
        semop_usr(semid, SHDMEM, -1);

        if(strcmp(shmpointer[0].fragment, "READY") != 0)
            errExit("Corrupted start message");

        // set sem ACCESS to count (number of files)
        // semop_usr(semid, ACCESS, count);
        // set sem ACCESS to count (number of files). set FIFO1, FIFO2, MSGQUEUE to IPC_MAX
        unsigned short semarray[7] = {count, IPC_MAX, IPC_MAX, IPC_MAX, 1, 0, 0};
        semarg.array = semarray;
        if (semctl(semid, 0, SETALL, semarg) == -1)
            errExit("Error while setting semaphore set");

        // child creation, parent operations
        for(child_num = 0; child_num < count && pid != 0; child_num++)
            if((pid = fork()) == -1)
                errExit("Error while forking");

        // child operations
        if(pid == 0) {
            // retrieving file stats
            if (stat(to_send[child_num - 1], &statbuf) == -1)
                errExit("Could not retrieve file stats");

            // splitting files
            switch (statbuf.st_size)
            {
                case 5:
                    files_dim[0] = 2;
                    for (int j = 1; j < 4; j++)
                        files_dim[j] = 1;
                    break;

                case 6:
                    files_dim[0] = 2;
                    files_dim[1] = 2;
                    files_dim[2] = 1;
                    files_dim[3] = 1;
                    break;

                case 9:
                    files_dim[0] = 3;
                    files_dim[1] = 2;
                    files_dim[2] = 2;
                    files_dim[3] = 2;
                    break;

                default:
                    if (statbuf.st_size % 4 == 0)
                        for (int j = 0; j < 4; j++)
                            files_dim[j] = statbuf.st_size / 4;
                    else {
                        for (int j = 0; j < 3; j++)
                            files_dim[j] = (statbuf.st_size / 4) + 1;

                        files_dim[3] = statbuf.st_size - (((statbuf.st_size / 4) + 1) * 3);
                    }
                    break;
            }

            // open files
            file_descriptor = open(to_send[child_num - 1], O_RDONLY);
            if (file_descriptor == -1)
                errExit("Error while opening file");

            
            for (int j = 0; j < 4; j++)
            {
                if(read(file_descriptor, char_to_read[j], files_dim[j]) == -1)
                    errExit("Error while reading file");
                
                char_to_read[j][files_dim[j]] = '\0';
            }

            // lowering sem 0
            semop_usr(semid, ACCESS, -1);
            // block child until sem 0 is == 0
            semop_usr(semid, ACCESS, 0);

            
            for (int j = 0, arr_flag[4] = {0} ; j < 4 ;) {
                if (arr_flag[0] == 0) {
                    semop_nowait(semid, FIFO1, -1);
                    if (errno == 0) {
                        packet = init_struct(child_num, getpid(), to_send[child_num - 1], char_to_read[0]);
                        write_fifo(fifo1_fd, packet, sizeof(struct queue_msg));

                        //if everything went well:
                        if(errno == 0) {
                            arr_flag[0] = 1;
                            j ++;
                        }
                    }
                }

                if (arr_flag[1] == 0) {
                    semop_nowait(semid, FIFO2, -1);
                    if (errno == 0) {
                        packet = init_struct(child_num, getpid(), to_send[child_num - 1], char_to_read[1]);
                        write_fifo(fifo2_fd, packet, sizeof(struct queue_msg));

                        //if everything went well:
                        if(errno == 0) {
                            arr_flag[1] = 1;
                            j ++;
                        }
                    }
                }

                if (arr_flag[2] == 0) {
                    semop_nowait(semid, MSGQUEUE, -1);
                    if (errno == 0) {
                        packet = init_struct(child_num, getpid(), to_send[child_num - 1], char_to_read[2]);

                        errno = 0;

                        msgsnd(queue_id, packet, sizeof(struct queue_msg) - sizeof(long), IPC_NOWAIT);

                        //if everything goes well: go on! else: NOOP
                        if(errno == 0) {
                            arr_flag[2] = 1;
                            j ++;
                        } else if(errno != EAGAIN)
                            errExit("Error while sending the message");
                    }
                }

                if (arr_flag[3] == 0) {
                    semop_nowait(semid, SHDMEM, -1);
                    if (errno == 0) {
                        for(int k = 0; k < IPC_MAX && arr_flag[3] == 0; k++){
                            if(shmpointer[k].mtype == 0){

                                // copy the payload in the shmem
                                strcpy(shmpointer[k].fragment, char_to_read[3]);
                                strcpy(shmpointer[k].pathname, to_send[child_num - 1]);
                                shmpointer[k].mtype = child_num;
                                shmpointer[k].pid = getpid();

                                // if everything goes as planned:
                                arr_flag[3] = 1;
                                j ++;
                            }
                            // else: halt and catch fire :(
                        }
                        semop_nowait(semid, SHDMEM, 1);
                    }
                }
            }

            // closing file
            close(file_descriptor);
            exit(0);
        } else {
            // waiting for all child to terminate
            for(int j = 0; j < count; j++) {
                if(wait(NULL) == -1)
                    errExit("Error while waiting for children");
            }
        }

        printf("Il valore di FINISH_CLIENT: %d\n", semctl(semid, FINISH_CLIENT, GETVAL));

        // let the server know that we are done
        semop_usr(semid, FINISH_CLIENT, 1);
        // wait for server to be done
        semop_usr(semid, FINISH_SERVER, -1);
    }

    return 0;
}




// GIMMY attento che qui finisce la main!!!




    /******************************
    * BEGIN FUNCTIONS DEFINITIONS *
    *******************************/

// Personalised signal handler for SIGUSR1 and SIGINT
void sigHandler (int sig) {
    // if signal is SIGUSR1, set original mask and kill process
    if(sig == SIGUSR1) {
        printf("\nShutdown client_0...\n");

        set_original_mask();

        // close IPCs
        close_fifo(fifo1_fd);
        close_fifo(fifo2_fd);
        free_shared_memory(shmpointer);

        exit(0);
    }

    // if signal is SIGINT, happy(end) continue
    if(sig == SIGINT)
        printf("\n\nI'm awake!\n\n");
}

// Function used to reset the default mask of the process
void set_original_mask() {
    // reset the signal mask of the process = restore original mask
    if(sigprocmask(SIG_SETMASK, &original_set_signals, NULL) == -1)
        errExit("sigprocmask(original_set) failed");
}

void create_signal_mask() {
    // initialize a set with all the signals
    if(sigfillset(&blocked_set_signals) == -1)
        errExit("sigfillset failed!");

    // initialize unlocked_set_signals to contain all signals of OS
    if(sigfillset(&unlocked_set_signals) == -1)
        errExit("sigfillset failed!");

    // remove SIGINT from unlocked_set_signals
    if(sigdelset(&unlocked_set_signals, SIGINT) == -1)
        errExit("sigdelset(SIGINT) failed!");
    
    // remove SIGUSR1 from unlocked_set_signals
    if(sigdelset(&unlocked_set_signals, SIGUSR1) == -1)
        errExit("sigdelset(SIGUSR1) failed!");
    
    // set new mask (unlocked_set_signals) and save old mask (original_set_signals)
    if(sigprocmask(SIG_SETMASK, &unlocked_set_signals, &original_set_signals) == -1)
        errExit("sigprocmask(new_set) failed!");   
}