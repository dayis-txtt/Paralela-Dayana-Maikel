// Compilar ->   gcc smokers.c -o smokers -pthread
// Ejecutar ->   ./smokers


#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>

void* run(void* arg);
void armar(int);
void limpiarMesa();
void sacarIngredientes();

int nucleos;
sem_t *ingredientes;
sem_t mesa_vacia;
char *ingrediente[]= {"Tabaco","Papel","Fosforos"};
int ing1, ing2;

int main(){
    srand(time(NULL));
    nucleos = 4;
    //malloc aparta espacio "nucleos" veces el tamaño de sem_t en el puntero ingredientes
    ingredientes = malloc (3 * sizeof(sem_t)); //3 semaforos para los ingredientes
    //ingrediente 0 = "tabaco", ingrediente 1 = "papel", ingrediente 2 = "fosforos
    pthread_t *threadID = malloc(nucleos * sizeof(pthread_t));
    int *ids = malloc(nucleos * sizeof(int));

    for (int i=0; i<3; i++){
        sem_init (&ingredientes[i],0,0);
    }
    sem_init(&mesa_vacia,0,1);
    int personas; //persona 0,1,2 = fumadores / persona 3 = agente
    for(int i = 0;i < nucleos; i++){
        ids[i] = i;
        personas = pthread_create(&threadID[i], NULL, run, &ids[i]);
    }

    for(int j=0; j < nucleos; j++){
            pthread_join(threadID[j], NULL);
    }

    free(ingredientes);free(threadID);free(ids);

    return 0;
}

void* run (void* arg){
    int  id = *(int*) arg;

    if(id < 3){
        printf("Soy el fumador numero %d y tengo %s\n", id+1, ingrediente[id]);
    }
    sleep (0.5);

    
    while(1){
        if(id == 3){
            sem_wait(&mesa_vacia);
            sacarIngredientes();
        }else{
            int dispon1, dispon2;
            
            while(1){
                dispon1 = sem_trywait(&ingredientes[(id+1)%3]);
                dispon2 = sem_trywait(&ingredientes[(id+2)%3]);
                
                if(dispon1 == 0 && dispon2 == 0){
                    break;
                }
                if (dispon1 == 0)
                    sem_post(&ingredientes[(id+1)%3]);

                if (dispon2 == 0)
                    sem_post(&ingredientes[(id+2)%3]); 
                    usleep(1000);
                }
            armar(id);
            limpiarMesa();
            sem_post(&mesa_vacia);
        }
    } 

    return NULL;
}

void armar(int id){
    printf ("FUMADOR %d: Ya tengo %s, voy a fumar\n", id+1, ingrediente[id]);
}


void sacarIngredientes(){
    ing1 = rand() % 3;
    
    do {
        ing2 = rand() % 3;
    } while (ing1 == ing2);

    sem_post(&ingredientes[ing1]);
    sem_post(&ingredientes[ing2]);

    printf ("\nAGENTE: Atencion, traigo %s y %s\n", ingrediente[ing1], ingrediente[ing2]);
}
void limpiarMesa(){
    printf ("AGENTE: Me llevo %s y %s\n", ingrediente[ing1], ingrediente[ing2]);
    sleep (1);
}
