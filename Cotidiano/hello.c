#include <stdio.h>
#include <pthread.h>

void* imprimirthread(void* arg);

int main () {	
	pthread_t threadID;
	int id = 1;

	pthread_create(&threadID, NULL, imprimirthread, &id);
	pthread_join(threadID, NULL);

	return 0;
}

void* imprimirthread(void* arg){
	int id = *(int*)arg;

	printf("Hello World\n");
	printf("Soy el hilo %d\n", id);

	return NULL;
}
