#include <sys/types.h>
#include <sys/sem.h>

#include "semun.h"

int set_semvalue(int sem_id, int i, int value){
	union semun sem_union;
	sem_union.val = value;
	if (semctl(sem_id, i, SETVAL, sem_union) == -1)
		return(0);
	return(1);
}

void del_semvalue(int sem_id){
	union semun sem_union;
	if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
		fprintf(stderr, "Failed to delete semaphore\n");
}

int semaphore_p(int sem_id, int i){
	struct sembuf sem_b;
	sem_b.sem_num = i;
	sem_b.sem_op = -1; /* P() */
	sem_b.sem_flg = 0;
	if (semop(sem_id, &sem_b, 1) == -1) {
		fprintf(stderr, "semaphore_p failed\n");
		return(0);
	}
	return(1);
}

int semaphore_v(int sem_id, int i, int ammount){
	struct sembuf sem_b;
	sem_b.sem_num = i;
	sem_b.sem_op = ammount; /* V() */
	sem_b.sem_flg = 0;
	if (semop(sem_id, &sem_b, 1) == -1) {
		fprintf(stderr, "semaphore_v failed\n");
		return(0);
	}	

	return(1);
}