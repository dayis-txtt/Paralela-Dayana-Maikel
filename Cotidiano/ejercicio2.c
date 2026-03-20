#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void* suma (void* arg);
int variable = 0;

int main() {

	int nucleos = sysconf(_SC_NPROCESSORS_ONLN);
	pthread_t threadID[nucleos];
        int hilos;
        int i;
	int j;
	for(i = 0;i < nucleos; i++){
                hilos = pthread_create(&threadID[i], NULL, suma, &i);
        }

	for(j=0; j < nucleos; j++){
                pthread_join(threadID[j], NULL);
        }

	return 0;

}

void* suma (void* arg){
	int i;
	int id = *(int*) arg;
	
	for (i=0; i < 10000000; i++){
		variable++;
	}
	
	printf("Termino el hilo %d\n",id);
	printf("Resultado: %d\n", variable);

	return NULL;



}
