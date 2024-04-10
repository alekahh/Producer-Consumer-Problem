# Producer-Consumer-Problem


## The goal of this project:
 Learn how to synchronize the execution of processes and coordinate the use of shared 
memory segment with POSIX Semaphores. Learn how to improve the efficiency of process execution using multi-threaded programming 
with POSIX Thread. Learn to understand the file system and related directory/file operations.
 
In this project I implement three programs to solve Problem 1, Problem 2, and Problem 3, respectively. 



## Problem 1

This problem aims to understand how the shared memory and semaphores are used between processes based on problem1.c. 
In this question, I implement all my code into multi_threads_run() function.

Compile problem1.c with the provided Makefile: make 
or by typing: gcc problem1.c helper.c -o problem1 -I. -pthread 


Compile problem1.c with the provided Makefile:

```bash
make 
```
or by typing:

```bash
gcc problem1.c helpers.c -o problem1 -I. -pthread 
```
If everything goes well, run the executable file with command: 
```bash
./problem1 <input_param> 
```


## Problem requirements:
1. This function can only be invoked in the child process. It should catch an eight-digit decimal number, i.e., the input parameter, which should be obtained from shared_param_c variable.
2. With the eight-digit decimal number, this function must create a semaphore (by calling sem_init() or sem_open()) for each digit of the number. The semaphore is used to ensure that only one thread can access (i.e., read and write) the corresponding digit at any time. 
3. The function (in the master thread, i.e., the main thread that enters the main function in your program) should create eight threads (i.e., child threads) by calling pthread_create(). 
4. Each thread must access (i.e., read and write) two adjacent digits among the eight digits of the number as shown in the following figure. Such as, the first thread should access the digit1 and digit2, the second thread should access the digit2 and digit3, and so on. Note that the eighth thread should access the digit8 and digit1. 
5. When a thread tries to access one digit, it must be able to lock the corresponding semaphore (which has a value greater than zero). Note that the sem_wait() function is used to lock a semaphore and the sem_post() function can unlock a semaphore. 
6. You should design to achieve mutual exclusion when the threads access the corresponding digits. That is, only when a thread can lock two semaphores at the same time, it can access the two corresponding digits and further proceed to execute operations. 
7. Once a thread can access the two digits, it can read their values and increase each by 1 respectively. If a digit's value reaches ten after adding 1, the thread should reassign modulo 10 of the value to this digit since the number is decimal. Once the thread executes the above operation, we say it execute an addition operation. 
8. Your program should accept two command-line parameters. For example, you should run your program with the command line: ./problem1 <input_param> <num_of_operations>. The first parameter is an eight-digit decimal number. The second parameter is used to control the number of times for executing the addition operation for each thread, i.e., each thread should execute the addition operation for several times, depending on this parameter. 
9. Once a thread is created or once it finishes an addition operation, it must try to lock the semaphores immediately until it finishes all the addition operations. 
10. When all eight child threads finish their calculations, the master thread should write the modified eight-digit decimal number into a text file by calling the saveResult() function, which is located in helper.h. The function has two input parameters, i.e., the name of the text file (must be p1_result.txt for Problem 1) and an int type number, such as the modified eight- digit decimal number for Problem 1. 
11. Please use the semaphore to achieve the mutual exclusion when the threads execute addition operations on the digits. A




## Implemented solution

In problem1.c I used sem_trywait function to avoid the deadlock possibility (trywait does not block like sem_wait). If either required semaphore is busy then the loop will release the lock and continue to execute the loop until both locks are acquired.

The sem_trywait function avoids deadlocks, but the possibility of livelock appears or any other problem because when running the code, the results were sometimes incorrect, so I introduced random delay at the end of the code.
First, I used no preemption design to avoid deadlocks and random delay to avoid livelocks.
Then, I created an array pthread_t threads[8] tread[i] corresponding to thread i+1 in the project requirements (it can increment i and i+1 digits). Afterwards, I used global array digits[8] where digits[i] are the digits of i. array semaphores[8] is used to hold the semaphores for each corresponding digit (semaphores[i] for digits[I]). I initialized the threads and semaphores in the multi_thread function.

## Problem 2


