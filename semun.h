#ifndef SEMUN_H
#define SEMUN_H                 /* Prevent accidental double inclusion */

#include <stdio.h>
#include <sys/types.h>          /* For portability */
#include <sys/sem.h>

#if ! defined(__FreeBSD__) && ! defined(__OpenBSD__) && \
                ! defined(__sgi) && ! defined(__APPLE__)
                /* Some implementations already declare this union */

union semun {                   /* Used in calls to semctl() */
    int                 val;
    struct semid_ds *   buf;
    unsigned short *    array;
#if defined(__linux__)
    struct seminfo *    __buf;
#endif
};

#endif

int set_semvalue(int sem_id, int i, int value);
void del_semvalue(int sem_id);
int semaphore_p(int sem_id, int i);
int semaphore_v(int sem_id, int i, int ammount);

#endif