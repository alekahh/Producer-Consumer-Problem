#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/shm.h>
#include <errno.h>
#include <assert.h>
#include <dirent.h>

#include "helpers.h"

#define BUFFER_SIZE 1000000 // 1MB

#define PARENT "/sem_parent_09922222"
#define CHILD "/sem_chlid_09922222"

//the program uses two semaphores to achieven synchronizatoin, parent semaphore and child semaphore
sem_t *sem_parent;
sem_t *sem_child;
int shmid; 
char *shared_mem;
char *buffer_start;

//this is an array of pointers pointing to the file entries
char *text_files[100];
int file_count = 0;
 
void traverseDir(char *dir_name);


int main(int argc, char **argv) {
    int process_id;

    char *dir_name = argv[1];

    if (argc < 2) {
        printf("Main process: Please enter a source directory name.\nUsage: ./main <dir_name>\n");
        exit(-1);
    }

    traverseDir(dir_name);

    //initialize semaphore for parent process, when it is locked by parent the parent can safely read the data into the shared memory

    //I used the same code to initialize inner process semaphores as was given in problem1 

    sem_parent = sem_open(PARENT, O_CREAT|O_EXCL, S_IRUSR|S_IWUSR, 1);
        // Check for error while opening the semaphore
    if (sem_parent != SEM_FAILED){
        printf("Successfully created new semaphore! parent\n");
    }
    else if (errno == EEXIST) {   // Semaphore already exists
        printf("Semaphore appears to exist already! parent\n");
        sem_parent = sem_open("PARENT", 0);
    }
    else {  // An other error occured
        assert(sem_parent != SEM_FAILED);
        exit(-1);
    }
    sem_child = sem_open(CHILD, O_CREAT|O_EXCL, S_IRUSR|S_IWUSR, 1);
    if (sem_child != SEM_FAILED){
        printf("Successfully created new semaphore child!\n");
    }
    else if (errno == EEXIST) {   // Semaphore already exists
        printf("Semaphore appears to exist already! child\n");
        sem_child = sem_open(CHILD, 0);
    }
    else {  // An other error occured
        assert(sem_child != SEM_FAILED);
        exit(-1);
    }

    shmid = shmget(IPC_PRIVATE, BUFFER_SIZE, 0666|IPC_CREAT);
    shared_mem = (char *) shmat(shmid, 0, 0);
    
    buffer_start = shared_mem + 2;

    //we will allocate two bytes of shared memory to 2 indicators that will be further explain in the report and child process
    shared_mem[0] = '0';
    shared_mem[1] = '0';





    switch (process_id = fork()) {
        default:
            // Parent Process
            printf("Parent process: My ID is %jd\n", (intmax_t) getpid());
            sem_wait(sem_parent);
			
            //this is the loop for every file, file_count is the amount of txt files found in the traverseDir functoin
            for (int i = 0; i < file_count; i++) {
            
                FILE *file = fopen(text_files[i], "r");
                shared_mem[1] = '0';
                long int size = fileLength(file);

            //this shared_mem indicator is needed to check if we need to check the special case when shared_mem[0] is 1, but we don't need
            //to decrement, confusing, I will expalin better in the report
                if (size % (BUFFER_SIZE - 1)  == 0) shared_mem[1] = '1';
                size_t bytes_read;
                
        
                //every time the parent fills the buffer or finishes the file reading it wakes the child process to read the data by sem_post(child)
                //and every time before changing shared mem it waits for child process's signal to start reading by sem_wait(parent)
                do {
                  
                    sem_wait(sem_parent);
                    shared_mem[0] = '0';
                    bytes_read = fread(buffer_start, 1, BUFFER_SIZE - 1, file);
                    buffer_start[bytes_read] = '\0';     
                    int count = wordCount(buffer_start);
                    sem_post(sem_child);

                    if (bytes_read < BUFFER_SIZE - 1) break;

                    //the indicator will tell child to decrement counter by one because the wordCount function may calculate
                    // one word two times if the file size is large and if the buffer fills up in the middle of the word
                    else {
                        shared_mem[0] = '1';
                    }
  
                        
                } while(1);

 			                                                                         
                fclose(file);

     
            }

            //the parent waits until the last chunk of file is read
            sem_wait(sem_parent);
            //it signals to child that it should stop reading the shared memory by typing THIS_IS_THE_END into sharedm emory
            strcpy(buffer_start, "THIS_IS_THE_END");
            sem_post(sem_child);

            // cleaning up of shared memory
            shmdt(shared_mem);
            shmctl(shmid, IPC_RMID, 0);
            printf("Parent process: Finished.\n");

            break;

        case 0:
            // Child Process
            printf("Child process: My ID is %jd\n", (intmax_t) getpid());   

            long int total_word_count = 0;
            do {
                //wait for parent signal
                sem_wait(sem_child);
                if (strcmp(buffer_start, "THIS_IS_THE_END") == 0) {
                    break;
                }

                else if (buffer_start[0] != '\0') {
                    int count = wordCount(buffer_start);
		
                    if (shared_mem[0] == '1') {
                        count--;
                        if (shared_mem[1] == '1') {
                            count++;
                        }
                    }
                    total_word_count += count;
                  
                }
                //give signal to parent that it can write new data
                sem_post(sem_parent);

            } while(1);

            saveResult("p2_result.txt", total_word_count);

            printf("Child process: Finished.\n");
            exit(0);

        case -1:
            // Error occurred.
            printf("Fork failed!\n");
            exit(-1);
    }

    //this clean up of the semaphores, close and delete
    sem_close(sem_parent);
    sem_close(sem_child);
    sem_unlink(PARENT);
    sem_unlink(CHILD);

    exit(0);
}

int is_current_or_parent_dir(const char *dir_name) {
    if ((strcmp(dir_name, ".") == 0)) return 1;
    else if ((strcmp(dir_name, "..") == 0)) return 1;
    else return 0;
}

void traverseDir(char *dir_name) {
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(dir_name))) {
        printf("No such directory exists\n");
        return;
    }
    
    while (1) {
        entry = readdir(dir);
        if (entry == NULL) break;       
        char path[1024];
        //snprintf contrcuts the full path
        snprintf(path, sizeof(path), "%s/%s", dir_name, entry->d_name);
        //stat is used to retrieve fiel information    
        if (entry->d_type == DT_DIR) {
            //check whether current or the parent directory
            if (is_current_or_parent_dir(entry->d_name) == 1)
				continue;
			traverseDir(path);
		} 
        else {
            //validates using function from helpers
			if (validateTextFile(entry->d_name)) {
				text_files[file_count] = strdup(path);
				file_count++;
			}
		}
	}
	closedir(dir);
}