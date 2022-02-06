#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <unistd.h>
#include "semun.h"
#include <time.h>
#include <math.h>
#include <sys/wait.h>
#include <sys/time.h>

int main(int argc, char **argv){

	if(argc < 5){
		printf("Not enough command line arguments provided.\nTry again.\n");
		return -1;
	}
	

	const int number_of_processes = atoi(argv[1]);
	const int number_of_iterations = atoi(argv[3]);
	const int number_of_entries = atoi(argv[4]);
	const int number_of_writers = (int)(atof(argv[2])*number_of_processes*number_of_iterations);
	const int number_of_readers = number_of_processes*number_of_iterations - number_of_writers;

	if(number_of_processes <=0 || number_of_writers <=0 || number_of_entries<=0){
		printf("All command line arguments must be positive.\nTry again.\n");
		return -1;
	}

	if(atof(argv[2]) >= 1 || atof(argv[2]) <= 0){
		printf("Writer-Reader ratio must be a double in range (0,1).\n");
		return -1;
	}

	/*printf("number_of_processes = %d\n", number_of_processes);
	printf("number_of_writers = %d\n", number_of_writers);
	printf("number_of_readers = %d\n", number_of_readers);
	printf("number_of_iterations = %d\n", number_of_iterations);
	printf("number_of_entries = %d\n", number_of_entries);*/
	
	////////////////////////////


	// shared memory variables
	int shared_memory_id;
	key_t key = IPC_PRIVATE;
	char* shared_memory;
	int size;
	struct shmid_ds shmbuffer;

	//semaphore variables
	int sem_id;

	//Create shared memory segment
	shared_memory_id = shmget(key,2*sizeof(int) + number_of_entries*3*sizeof(int),IPC_CREAT | 0666);
	//first int = number of times the memory was written, second int = number of times the memory was read
	// for every entry : first int = times written, second int = times read, third int = number of reader processes currently reading the entry
	if(shared_memory_id < 0){
		perror("Error with shmget()");
		exit(EXIT_FAILURE);
	}	

	shared_memory = (char*)shmat(shared_memory_id,0,0);
	printf ("shared memory attached at address %p\n", shared_memory);

	if( *shared_memory = -1 && !EXIT_FAILURE){
		perror("Error with shmat()");
		exit(EXIT_FAILURE);		
	}

	shmctl (shared_memory_id, IPC_STAT, &shmbuffer);	
	size = shmbuffer.shm_segsz;
	printf ("segment size: %d\n", size);


	// initialize shared memmory (fill memory with zeros)
	bzero(shared_memory,size); 
	
	
	///////////////////////

	//Create semaphores
	sem_id = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666); // private => access only by coordinator ? must be changed?

	
	if (sem_id == -1){
		perror("semget()");
		exit(EXIT_FAILURE);
	}


	if (set_semvalue(sem_id, 0, 1) == 0){ //binary semaphore (write)
		perror("semctl() ");
		exit(EXIT_FAILURE);
	}

	
	
	if (set_semvalue(sem_id, 1, 1) == 0){ // binary semaphore (mutex)
		perror("semctl() ");
		exit(EXIT_FAILURE);
	}

	///////////////////////////////

	pid_t pid[number_of_processes];

	int writer_count = 0;
	int reader_count = 0;
	
	//Create peers
	
	for(int i = 0; i < number_of_processes; i++){
		
		if( fork()== 0 ){
			
			pid[i] = getpid();
			//printf("%d\n", pid[i]);
			int read_attempts = 0;
			int write_attempts = 0;
			double time_average = 0.0;

			for(int j = 0; j < number_of_iterations; j++){
				
				int shmem_position; // which memory position will be accessed
				int operation; // 1 = writer , 2 = reader
					
				// get writer_count, reader_count from shared memory
				memcpy(&writer_count,shared_memory, sizeof(int));
				memcpy(&reader_count,shared_memory + sizeof(int), sizeof(int));

				srand(getpid() + i*j);
				
				double percentage = (rand() % 100)/100.0;
				
				if(percentage <= atof(argv[2])){ 
					operation = 1; //writer
					if(writer_count < number_of_writers) writer_count++;
					else{
						operation = 2; //reader
						reader_count++;	
					}
				}else{
					operation = 2; //reader
					if(reader_count < number_of_readers) reader_count++;
					else{
						operation = 1; //writer
						writer_count++;
					}
				}

				// save writer_count, reader_count to shared memory
				memcpy(shared_memory,&writer_count, sizeof(int));
				memcpy(shared_memory + sizeof(int),&reader_count, sizeof(int));

				shmem_position = rand() % number_of_entries;

				if(operation == 1){//writer
					
					write_attempts++;
					time_t start = time(NULL);

					if(semaphore_p(sem_id, 0) == 0) exit(1); // write down
				
					int writer_entry = 0;
					//Critical section
					memcpy(&writer_entry,shared_memory +2*sizeof(int)+ shmem_position*3*sizeof(int), sizeof(int));
					writer_entry++;
					memcpy(shared_memory +2*sizeof(int)+ shmem_position*3*sizeof(int),&writer_entry, sizeof(int));
					//End of critical section

					double u = rand()/(RAND_MAX + 1.0); 
					u = -log(1-u)/0.2; // exponentially distributed time
				    //printf("sleep while writing: %f\n", u);
					sleep(u);
					if(semaphore_v(sem_id, 0, 1) == 0) exit(1); //write up
					//printf("write seconds %f\n", (double)(time(NULL) - start));
					time_average += (double)(time(NULL) - start);
					
				}else{
					
					read_attempts++;

					int shmem_read;
					
					time_t start = time(NULL);

					if(semaphore_p(sem_id, 1) == 0) exit(1); //mutex down
					memcpy(&shmem_read,shared_memory + 4*sizeof(int) + shmem_position*3*sizeof(int),sizeof(int));
					shmem_read++;
					memcpy(shared_memory + 4*sizeof(int) + shmem_position*3*sizeof(int),&shmem_read,sizeof(int));
					if(shmem_read == 1) 
						if(semaphore_p(sem_id, 0) == 0) exit(1); // write  down
					
					if(semaphore_v(sem_id, 1, 1) == 0) exit(1);	//mutex up
					
					int reader_entry = 0;
					//Critical section
					memcpy(&reader_entry,shared_memory +3*sizeof(int)+ shmem_position*3*sizeof(int), sizeof(int));
					reader_entry++;
					memcpy(shared_memory +3*sizeof(int)+ shmem_position*3*sizeof(int),&reader_entry, sizeof(int));
					//End of critical section
					double u = rand()/(RAND_MAX + 1.0);
					u = -log(1-u)/2;  // exponentially distributed time
					//printf("sleep while reading: %f\n", u);
					sleep(u);
					
					if(semaphore_p(sem_id, 1) == 0) exit(1); //mutex down
					memcpy(&shmem_read,shared_memory + 4*sizeof(int) + shmem_position*3*sizeof(int),sizeof(int));
					shmem_read--;
					memcpy(shared_memory + 4*sizeof(int) + shmem_position*3*sizeof(int),&shmem_read,sizeof(int));
					if(shmem_read == 0)
						if(semaphore_v(sem_id, 0, 1) == 0) exit(1);	//write up
					if(semaphore_v(sem_id, 1, 1) == 0) exit(1);	//mutex up
					time_average += (double)(time(NULL) - start);
					
				}
				

			}

			printf("Printing statistics of peer process with process id = %d\n", getpid());
			printf("Write memory  attempts: %d, Read memory  attempts: %d, Average memory access waiting time: %f\n",write_attempts,read_attempts,time_average/(write_attempts+read_attempts));
			exit(0);

		}else if (pid < 0 ){
			perror("fork failed");
			exit(EXIT_FAILURE);
		}else{
			waitpid(-1, NULL, 0);
		}

	}

	//////////////////////////////

	waitpid(-1, NULL, 0);

	int a,b;
	printf("Printing shared memory statistics:\n");
	memcpy(&a,shared_memory, sizeof(int));
	memcpy(&b,shared_memory + sizeof(int), sizeof(int));
	printf("Overall Writes: %d\nOverall Reads %d\nEntries:\n", a,b);
	int offset = 2*sizeof(int);
	for(int i = 0; i < number_of_entries; i++){
		int reader, writer;
		memcpy(&writer,shared_memory + offset, sizeof(int));
		offset += sizeof(int);
		memcpy(&reader,shared_memory + offset, sizeof(int));
		offset += 2*sizeof(int);
		printf("entry number: %d, writer sum: %d , reader sum: %d\n", i+1, writer, reader);
	}


	///////////////////////////////


	//Destroy shared memory segment
	shmdt(shared_memory);
	shmctl(shared_memory_id, IPC_RMID, 0);

	//Delete semaphores
	del_semvalue(sem_id);

}