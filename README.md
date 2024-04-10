# Producer-Consumer-Problem


## The goal of this project:
 Learn how to synchronize the execution of processes and coordinate the use of shared 
memory segment with POSIX Semaphores. Learn how to improve the efficiency of process execution using multi-threaded programming 
with POSIX Thread. Learn to understand the file system and related directory/file operations.
 
In this project I implement three programs to solve Problem 1, Problem 2, and Problem 3, respectively. 



## Problem 1

This problem aims to understand how the shared memory and semaphores are used between processes based on problem1.c. 
In this question, I implement all my code into multi_threads_run() function.

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

In this problem, you are required to implement a word counter with two processes to automatically count the number of words in text files (with suffix .txt). 
You should achieve this with a C program that uses two processes (named parent process and child process respectively, both of which use only one thread). The parent process reads text files (with suffix .txt) under the given directory (which can be obtained from the input parameter of this program). Then the child process counts the number of words in those text files. Two processes communicate with each other using a shared memory (the limited size is 1MB). You should write your code in the provided problem2.c file. 


1. The first step is to find all the text files (no more than 100) under the given directory. Since the number of the text files and their locations in the directory are unknown, you should design your code to traverse the directory to find all the text files under it. You can directly implement this into the traverseDir() function. By invoking traverseDir() function with a given directory, the program should find all the text files and their access paths under the directory. 
2. The parent process should read the text files one-by-one and write the content of each text file into a shared memory with limited buffer size of 1MB. (Hint: you should handle the case that the total size of a text file exceeds the buffer size.) Please note only one shared memory with limited buffer size of 1MB is allowed to be used in your program. 
3. Once the parent has written the content of a text file into the shared memory, the child process can read the content from the shared memory and count the number of words. After that, the parent process can write data of another text file into the shared memory again. 
4. Since the parent process should not modify the shared memory when the child process is reading from it (and vice versa), you should use the semaphore to achieve mutual exclusion while the processes access (i.e., read or modify) the shared memory. 
5. It requires that the writing operation of the parent process and the reading operation of the child process must alternate. You should use semaphore to synchronize the operations of the two processes on the shared memory. 
6. We provide the wordCount() function in helplers.h, in which the counting is done in a simple manner by looking for spaces (i.e., “ ”) and newline characters (i.e., “\n”) until the string terminator is reached. You must invoke this provided function to count the number of words in each of the text files.
7. When all text files have been counted, the child process should write the total number of words in all text files into a text file by calling the saveResult() function, which is located in helper.h. The function has two input parameters, i.e., the name of the text file (must be “p2_result.txt” for Problem 2) and the total number of words in all text files. 


You can compile the files based on the provided Makefile (you may modify it before using it for this problem) following this command: 

```bash
make 
```
or you can directly use the following command: 

```bash
gcc problem1.c helpers.c -o problem1 -I. -pthread 
```
After compiling, you can run the executable file to test your program with the provided test cases, by typing command: 
```bash
./problem2 <source_dir> 
```

## Implemented solution
1. To find all the text files under a directory the following system calls were implemented: <br />
opendir(): Opens a directory stream corresponding to the given directory name. <br />
readdir(): Reads the next directory entry from the directory stream. <br />
closedir(): Closes the directory stream. stat(): Retrieves information about a file. <br />
strcmp(): Compares two strings and returns 0 if they are equal. snprintf(): Writes formatted output to a string, up to a specified maximum size. is_current_or_parent_dir(): takes in a directory name as an argument and checks whether it is either the current directory (represented by ".") or the parent directory (represented by ".."). It returns 1 if it is either of them, otherwise returns 0. <br />
traverseDir(): takes in a directory name as an argument and opens the directory using the opendir() function. If the directory does not exist, it prints an error message and returns it. The program then enters a loop where it reads the contents of the directory using the readdir() function. It reads each file and directory in the current directory one by one and performs the following operations on them:  It constructs the full path of the file/directory using the snprintf() function. The snprintf() function constructs a string from a format string and a variable number of arguments.  If the current entry is a directory, the program recursively calls traverseDir() with the newly constructed path. If the current entry is the current or parent directory, it skips the current iteration using the continue statement.  If the current entry is a file, the program checks whether it is a valid text file using the validateTextFile() function. If the file is valid, its path is stored in an array called text_files. After processing all the files and directories in the current directory, the program closes the directory using the closedir() function.
3. To achieve the synchronization between two processes using the semaphore, we performed the following steps: <br />
We used two semaphores – for parent and child respectively. The parent needs to obtain sem_parent to start writing data to the shared memory. Then when the buffer is filled or the file ends, the parent process signals the child process by sem_post(sem_parent) for it to start counting words, then it waits for the signal from the child. The child process obtains sem_child (available after the parent signal) then it reads the data, counts the word, and signals back to the parent that it can overwrite the shared memory with new data.
4. To handle the case of large text files, the program reads the file in chunks, and each chunk is read into the buffer with a size equal to buffer size – 1 leaving 1 space for the NULL value. The loop continues until no more bytes are left.
5. Description of other processes within the program: <br />
We used char * text_files[] array to hold the entries for each file, the traverseDir uses a loop to find all the .txt files. We used two semaphores to achieve synchronization between parent and child processes. The signal to use sem_child can be obtained only from the parent and the signal to use sem_parent can be obtained only from the child, so synchronization is achieved. The termination of the child process happens when the parent writes “THIS IS THE END” string into the shared memory and signals the child to read it. The parent loop ends when all the bytes in all the files have been read.  About the shared_mem[0] and shared_mem[1]. The first shared_mem[0] is set to ‘1’ everytime a buffer is completely filled. This is to indicate to the child that this is not the end of the file, so the total count should be decremented by one. We need to do this because the wordCount function adds 1 at the end to count the last word, but if the word count is interrupted in the middle of the word because the buffer is filled, this word will be counted twice. shared_mem[1] is set to ‘1’ because we need the special case when shared_mem[0] is ‘1’ but it is in fact the end of the file, so the total count should be incremented by one.


## Problem 3

In Problem 2, both the parent and child processes have only one thread. When the given directory contains many text files, or the text files have a large size of content, it would be inefficient. In this problem, you are asked to extend your program implemented in Problem 2 with multi-threaded programming. You should implement this program into a source file named as problem3.c.

You can compile the files based on the provided Makefile (you may modify it before using it for this problem) following this command: 

```bash
make 
```
or you can directly use the following command: 

```bash
gcc problem1.c helpers.c -o problem3 -I. -pthread 
```
After compiling, you can run the executable file to test your program with the provided test cases, by typing command: 
```bash
./problem3 <source_dir> 
```

## Problem requirements:

The requirements for this problem are same as those in Problem 2, except that the child process should be implemented with multiple threads to count the words of each text file based on POSIX thread. Your solution should comply with the following requirements: 
1. You should use 4 threads in the child process.
2. You must call the provided wordCount() function to count the number of words. Please do not modify the implementation of wordCount() function.
3.  When all text files have been counted, the child process should write the total number of words in all text files into a text file (must be “p3_result.txt” for this problem) by calling the saveResult(). 


## Implemented Solution:

To parallelize the word counting operation with multiple threads in the child process we created four threads; each thread is allocated ¼ of the shared memory. They all count the words in parallel in their parts then after all threads are finished executing, the summation of their results is added to the total word count. <br />
We also used shared_mem[0] and shared_mem[1] for interrupted counting of the words due to the buffer being filled similarly to problem2, you can refer to problem2’s last paragraph for a more detailed explanation.<br />
We used a new struct with 3 int parameters, start,end, and word_count. The start and end are for determining which substring of shared memory the thread needs to count the words in. The word_count is the number of words in their respective substrings. The multi_thread function initializes the threads with their start and end values and countes the word_count. The thread_func is a thread function that counts the words in each respective thread.








