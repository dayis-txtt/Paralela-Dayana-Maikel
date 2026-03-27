#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void* suma (void* arg);
int variableCompartida = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;	

int main() {

	int nucleos = sysconf(_SC_NPROCESSORS_ONLN);
	pthread_t threadID[nucleos];
        int hilos;
     
	for(int i = 0;i < nucleos; i++){
                hilos = pthread_create(&threadID[i], NULL, suma, &i);
        }

	for(int j=0; j < nucleos; j++){
                pthread_join(threadID[j], NULL);
        }


	return 0;

}

void* suma (void* arg){
	int id = *(int*) arg;
	int sumaTemp = 0;
	for (int i=0; i < 1000000; i++){
		sumaTemp++;
	}
	
	pthread_mutex_lock(&lock);
	variableCompartida += sumaTemp;
	pthread_mutex_unlock(&lock);

	printf("Termino el hilo %d\n",id);
	printf("Resultado: %d\n", variableCompartida);

	return NULL;



}
