#include <stdio.h>
#include <pthread.h>


void* imprimirthread(void* arg);

int main () {	
	pthread_t threadID[4];
	int status;
	int i;
	for(i = 0;i < 4; i++){
		status = pthread_create(&threadID[i], NULL, imprimirthread, &i);
	}
	
	for(i=0;i<4;i++){
		pthread_join(threadID[i], NULL);
	}
	return 0;
}

void* imprimirthread(void* arg){
	int id = *(int*)arg;

	printf("Hello World\tSoy el hilo %d\n", id);

	return NULL;
}
