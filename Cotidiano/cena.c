#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>

void* run(void* arg);
void think(int);
void eat(int);
int nucleos;
sem_t *forks;

pthread_mutex_t mutex_fideos;
int fideos = 12;


int main(){
	nucleos = sysconf(_SC_NPROCESSORS_ONLN);
	//malloc aparta espacio "nucleos" veces el tamanno de sem_t en el puntero forks
	forks = malloc (nucleos * sizeof(sem_t));

	pthread_mutex_init(&mutex_fideos, NULL);

	pthread_t *threadID = malloc(nucleos * sizeof(pthread_t));
	int *ids = malloc(nucleos * sizeof(int));

	for (int i=0; i<nucleos; i++){
		sem_init (&forks[i],0,1);
	}
	int filosofos;

	for(int i = 0;i < nucleos; i++){
		ids[i] = i;
        	filosofos = pthread_create(&threadID[i], NULL, run, &ids[i]);
	}	


	for(int j=0; j < nucleos; j++){
                pthread_join(threadID[j], NULL);
        }
	printf("El plato de fideos se acabo\n");

	free(forks);
	free(threadID);
	free(ids);
	
	return 0;
}

void* run (void* arg){
	int  id = *(int*) arg;
	
	while(fideos > 1){
		think(id);
	        if (id == nucleos - 1){
            		sem_wait(&forks[(id + 1) % nucleos]); // Tenedor izquierdo
	       		sem_wait(&forks[id]); 		// Tenedor derecho
        	} else {
            		sem_wait(&forks[id]);
            		sem_wait(&forks[(id + 1) % nucleos]);
        	}
		
        	eat(id);
	        pthread_mutex_lock(&mutex_fideos);
                fideos--;
       		pthread_mutex_unlock(&mutex_fideos);

        	sem_post(&forks[id]);
	        sem_post(&forks[(id + 1) % nucleos]);
	}
	return NULL;
}

void think(int id){
	printf ("Filosofando, mi nombre es %d\n", id);
	sleep(1);
}


void eat(int id){
        printf ("Comiendo, mi nombre es %d\n", id);
	sleep(3);
}


