#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>


void* run(void* arg);
void armar(int);
void fumar(int);
int nucleos;
sem_t *ingredientes;



int main(){
	nucleos = sysconf(_SC_NPROCESSORS_ONLN);
	//malloc aparta espacio "nucleos" veces el tamaño de sem_t en el puntero ingredientes
	ingredientes = malloc (3 * sizeof(sem_t)); //3 semaforos para los ingredientes
    //ingrediente 0 = "tabaco", ingrediente 1 = "papel", ingrediente 2 = "fosforos"

	pthread_t *threadID = malloc(nucleos * sizeof(pthread_t));
	int *ids = malloc(nucleos * sizeof(int));

	for (int i=0; i<3; i++){
		sem_init (&ingredientes[i],0,1);
	}
	int personas; //persona 0,1,2 = fumadores / persona 3 = agente

	for(int i = 0;i < nucleos; i++){
		ids[i] = i;
            personas = pthread_create(&threadID[i], NULL, run, &ids[i]);
	}	








	for(int j=0; j < nucleos; j++){
                pthread_join(threadID[j], NULL);
        }

	free(ingredientes);
	free(threadID);
	free(ids);
	
	return 0;
}

void* run (void* arg){
	int  id = *(int*) arg;
	//Hacer If donde cada fumador revise de acuerdo a su id que semaforos estan abiertos
    //El agente controla que semáforos están abiertos, 
	while(1){
		think(id);
        if (id == nucleos - 1){
            sem_wait(&ingredientes[(id + 1) % nucleos]); // Ingrediente izquierdo
            sem_wait(&ingredientes[id]); 		// Ingrediente derecho
        } 
        else {
            sem_wait(&ingredientes[id]);
            sem_wait(&ingredientes[(id + 1) % nucleos]);
        }
		
        armar(id);
        fumar(id);

        sem_post(&ingredientes[id]);
        sem_post(&ingredientes[(id + 1) % nucleos]);
	}

	return NULL;
}

void armar(int id){

	printf ("Armando cigarro, soy el fumador %d\n", id);
	sleep(1);
}


void fumar(int id){
        printf ("Fumando, soy el fumador %d\n", id);
	sleep(3);
}
