#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sys/shm.h>            // This is necessary for using shared memory constructs
#include <semaphore.h>          // This is necessary for using semaphore
#include <fcntl.h>                      // This is necessary for using semaphore
#include <pthread.h>        // This is necessary for Pthread
#include <string.h>
#include "helpers.h"


#define PARAM_ACCESS_SEMAPHORE "/param_access_semaphore_041"



long int global_param = 0;

//global array to semaphores
sem_t sems[8];
//global array for treads to share;
int digits[8];
/**
* This function should be implemented by yourself. It must be invoked
* in the child process after the input parameter has been obtained.
* @parms: The input parameter from the terminal.
*/
void multi_threads_run(long int input_param, int numberOf);
int num_operations;

int main(int argc, char **argv)
{
        int shmid, status;
        long int local_param = 0;
        long int *shared_param_p, *shared_param_c;

        if (argc < 2) {
                printf("Please enter an eight-digit decimal number as the input parameter.\nUsage: ./main <input_param>\n");
                exit(-1);
        }

        /*
                Creating semaphores. Mutex semaphore is used to acheive mutual
                exclusion while processes access (and read or modify) the global
                variable, local variable, and the shared memory.
        */

        // Checks if the semaphore exists, if it exists we unlink him from the process.
        sem_unlink(PARAM_ACCESS_SEMAPHORE);

        // Create the semaphore. sem_init() also creates a semaphore. Learn the difference on your own.
        sem_t *param_access_semaphore = sem_open(PARAM_ACCESS_SEMAPHORE, O_CREAT|O_EXCL, S_IRUSR|S_IWUSR, 1);

        // Check for error while opening the semaphore
        if (param_access_semaphore != SEM_FAILED){
                printf("Successfully created new semaphore!\n");
        }
        else if (errno == EEXIST) {   // Semaphore already exists
                printf("Semaphore appears to exist already!\n");
                param_access_semaphore = sem_open(PARAM_ACCESS_SEMAPHORE, 0);
        }
        else {  // An other error occured
                assert(param_access_semaphore != SEM_FAILED);
                exit(-1);
        }

        /*
            Creating shared memory.
        The operating system keeps track of the set of shared memory
            segments. In order to acquire shared memory, we must first
            request the shared memory from the OS using the shmget()
        system call. The second parameter specifies the number of
            bytes of memory requested. shmget() returns a shared memory
            identifier (SHMID) which is an integer. Refer to the online
            man pages for details on the other two parameters of shmget()
        */
        shmid = shmget(IPC_PRIVATE, sizeof(long int), 0666|IPC_CREAT); // We request an array of one long integer

        /*
            After forking, the parent and child must "attach" the shared
            memory to its local data segment. This is done by the shmat()
            system call. shmat() takes the SHMID of the shared memory
            segment as input parameter and returns the address at which
            the segment has been attached. Thus shmat() returns a char
            pointer.
        */

        if (fork() == 0) { // Child Process

                printf("Child Process: Child PID is %jd\n", (intmax_t) getpid());

                /*  shmat() returns a long int pointer which is typecast here
                    to long int and the address is stored in the long int pointer shared_param_c. */
                shared_param_c = (long int *) shmat(shmid, 0, 0);

                while (1) // Loop to check if the variables have been updated.
                {
                        // Get the semaphore
                        sem_wait(param_access_semaphore);
                        printf("Child Process: Got the variable access semaphore.\n");

                        if ( (global_param != 0) || (local_param != 0) || (shared_param_c[0] != 0) )
                        {
                                printf("Child Process: Read the global variable with value of %ld.\n", global_param);
                                printf("Child Process: Read the local variable with value of %ld.\n", local_param);
                                printf("Child Process: Read the shared variable with value of %ld.\n", shared_param_c[0]);

                        // Release the semaphore
                        sem_post(param_access_semaphore);
                        printf("Child Process: Released the variable access semaphore.\n");

                                break;
                        }

                        // Release the semaphore
                        sem_post(param_access_semaphore);
                        printf("Child Process: Released the variable access semaphore.\n");
                }

        /**
         * After you have fixed the issue in Problem 1-Q1,
         * uncomment the following multi_threads_run function
         * for Problem 1-Q2. Please note that you should also
         * add an input parameter for invoking this function,
         * which can be obtained from one of the three variables,
         * i.e., global_param, local_param, shared_param_c[0].
         */

                //the function will receive two parameters, first is the number from shared memory and the second is second argument int which is number of operations for each tread
                multi_threads_run(shared_param_c[0], strtol(argv[2], NULL, 10));

                /* each process should "detach" itself from the
                   shared memory after it is used */

                shmdt(shared_param_c);

                exit(0);
        }
        else { // Parent Process

                printf("Parent Process: Parent PID is %jd\n", (intmax_t) getpid());

                /*  shmat() returns a long int pointer which is typecast here
                    to long int and the address is stored in the long int pointer shared_param_p.
                    Thus the memory location shared_param_p[0] of the parent
                    is the same as the memory locations shared_param_c[0] of
                    the child, since the memory is shared.
                */
                shared_param_p = (long int *) shmat(shmid, 0, 0);

                // Get the semaphore first
                sem_wait(param_access_semaphore);
                printf("Parent Process: Got the variable access semaphore.\n");

                global_param = strtol(argv[1], NULL, 10);
                local_param = strtol(argv[1], NULL, 10);
                shared_param_p[0] = strtol(argv[1], NULL, 10);
            

                // Release the semaphore
                sem_post(param_access_semaphore);
                printf("Parent Process: Released the variable access semaphore.\n");

                wait(&status);

                /* each process should "detach" itself from the
                   shared memory after it is used */

                shmdt(shared_param_p);

                /* Child has exited, so parent process should delete
                   the cretaed shared memory. Unlike attach and detach,
                   which is to be done for each process separately,
                   deleting the shared memory has to be done by only
                   one process after making sure that noone else
                   will be using it
                 */

                shmctl(shmid, IPC_RMID, 0);

        // Close and delete semaphore.
        sem_close(param_access_semaphore);
        sem_unlink(PARAM_ACCESS_SEMAPHORE);

                exit(0);
        }

        exit(0);
}




/**
* This function should be implemented by yourself. It must be invoked
* in the child process after the input parameter has been obtained.
* @parms: The input parameter from terminal.
*/
void *thread_func(void *arg) {

    int thread_num = (int)(long)arg;


    //the digits that are successible based on the arg value (index of the thread, if thread1 then 0, 1)
        int index1 = thread_num;
        int index2 = (thread_num + 1) % 8;
    
//loop to complete operations
    for (int i = 0; i < num_operations; i++) {

        //before each incremenation we will lock and release the lock as required by the requirements
        while(1) {

                //will trywait to avoid deadlocks, the loops will continue running until both locks are acquired
                if (sem_trywait(&sems[index1]) == 0) {
                        if (sem_trywait(&sems[index2]) == 0) {
                                digits[index1] = (digits[index1] + 1) % 10;
                                digits[index2] = (digits[index2] + 1) % 10;
                                sem_post(&sems[index1]);
                                sem_post(&sems[index2]);      
                                break;                            
                        }
                }

                else {
                        sem_post(&sems[index1]);
                } 
                //we will make delay to avoid livelock
                int delay_us = rand() % 1000000; // Maximum delay of 1 second
                usleep(delay_us);                            
        }

    }

    pthread_exit(NULL);
}

void multi_threads_run(long int input_param, int numberOf)
{       
        num_operations = numberOf;
        // Add your code here
        pthread_t threads[8];
        //assign the values of in the input to the global array digits so that threads can use it
        for (int i = 7; i >= 0; i--) {
                digits[i] = input_param % 10;
                input_param /= 10;
        }

        //initialize semaphores so that threads can use the shared variable safely
        for (int i = 0; i < 8; i++) {
                sem_init(&sems[i], 0, 1);  
        }

        //create threads and pass i argument to each one (which determines which digits the thread can access and modify)
        for (int i = 0; i < 8; i++) {
                pthread_create(&threads[i], NULL, thread_func, (void *)(long)i);
        }
        //wait until every thread finishes
        for (int i = 0; i < 8; i++) {
                pthread_join(threads[i], NULL);
        }
        
        //destroy the sempahores after finishing to use
        for (int i = 0; i < 8; i++) {
                sem_destroy(&sems[i]);
        }

        // Combine digits back into a number
        long int result = 0;
        for (int i = 0; i < 8; i++) {
                result *= 10;
                result += digits[i];
        }

        // Save the result to the file
        saveResult("p1_result.txt", result);        
        

}
