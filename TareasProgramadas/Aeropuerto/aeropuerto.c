// Compilar -> gcc aeropuerto.c -o aeropuerto -pthread
// Ejecutar -> ./aeropuerto 15 3 3 1000 4


#include<stdio.h>
#include<stdlib.h>
#include<semaphore.h>
#include<pthread.h>
#include<string.h>
#include<unistd.h>
#include <time.h>

// N(Pasajeros), M(Registros), K(Pasajeros hasta break), T_max(espera en cola), Q(Maximo por cola)
int N, M, K, T_max, Q;
int* pasajeros;
char* clase[] = {"Economic", "Business", "International"};

typedef struct Nodo{  //Cada Pasajero en la fila
    int clase;
    struct timespec tiempoEspera;
    struct Nodo *sig;
} Nodo;
typedef struct {  //Cada fila de pasajeros
    Nodo *inicio;
    Nodo *fin ;
    int cantidad;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} Fila;
void push(Fila*, int);
Nodo* pop(Fila *fila);

int main(int argc, char *argv[]){
    if (argc !=6) return 1; // No se pasan valores desde la terminal
    N = atoi(argv[1]);
    M = atoi(argv[2]);
    K = atoi(argv[3]);
    T_max = atoi(argv[4]);
    Q = atoi(argv[5]);

    Fila filas[3]; // Creacion e inicializacion de filas
    for (int i = 0; i < 3; i++){
        filas[i].inicio = NULL;
        filas[i].fin = NULL;
        filas[i].cantidad = 0;
        pthread_mutex_init(&filas[i].lock, NULL);
        pthread_cond_init(&filas[i].cond, NULL);
    }

    pasajeros = malloc (N*(sizeof(int)));
    int temp;
    for (int i=0; i<N ; i++){
        temp = rand()%3;
        pasajeros[i] = temp;
        push(&filas[temp], temp);
    }   

    for (int i=0; i<N ; i++){
        printf ("%d: tipo %s\n",i , clase[pasajeros[i]]);
    }

    return 0;
}

void push(Fila *fila, int clase){
    Nodo *nuevo = (Nodo*) malloc(sizeof(Nodo));
    nuevo->clase = clase;
    clock_gettime(CLOCK_MONOTONIC, &nuevo->tiempoEspera);
    nuevo->sig = NULL; 

    pthread_mutex_lock(&fila->lock);    
    if (fila->fin == NULL) {
        fila->inicio = nuevo;
        fila->fin = nuevo;
    } else {
        fila->fin->sig = nuevo;
        fila->fin = nuevo;
    }
    fila->cantidad++;

    pthread_cond_signal(&fila->cond);
    pthread_mutex_unlock(&fila->lock);
}

Nodo* pop(Fila *fila) {
    pthread_mutex_lock(&fila->lock);

    while (fila->inicio == NULL) {
        pthread_cond_wait(&fila->cond, &fila->lock); 
    }
    Nodo *temp = fila->inicio;
    fila->inicio = fila->inicio->sig;

    if (fila->inicio == NULL) {
        fila->fin = NULL;
    }
    fila->cantidad--;
    pthread_mutex_unlock(&fila->lock);

    return temp;
}
