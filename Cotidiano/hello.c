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
	
	int j;
	for(j=0;j<4;j++){
		pthread_join(threadID[j], NULL);
	}
	return 0;
}

void* imprimirthread(void* arg){
	int id = *(int*)arg;

	printf("Hello World\tSoy el hilo %d\n", id);

	return NULL;
}
