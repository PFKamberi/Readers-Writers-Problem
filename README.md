# Readers-Writers-Problem

## The Problem Statement:
There is a shared resource (in our case a shared memory) that should be accessed by multiple peer processes. There are two types of peer processes in this context: readers and writers.
Any number of readers can read from the shared resource simultaneously, but only one writer can write to the shared resource. When a writer is writing data to the resource, 
no other peer process can access the resource. A writer cannot write to the resource if there are non zero number of readers accessing the resource at that time. The solution 
to this problem is given via the use of semaphores.

## Shared Memory:

The shared memory has the following structure:

Firstly, two integers are stored at the beginning of the shared memory containing the number of writer processes (first integer), and the number of reader processes (second integer). 
The rest part of the shared memory is used to store entries each of which consists of three integers representing respectively:
* the number of times a peer process had written to the memory entry.
* the number of times a peer process had read from the memory entry
* the number of reader peer processes currently reading from the memory entry.


## Compilation and Execution:
To compile the program simply run: 
```
make
```

To execute the program the command line must be provided with four arguments in the following particular order:
* the total number of processes (i.e. both readers and writers) (integer).
* the writers-readers ratio (double in range (0,1) ).
* the number of iterations.
* the number of entries in shared memory.

Execution example:

```
./coordinator 20 0.4 10 100
```
