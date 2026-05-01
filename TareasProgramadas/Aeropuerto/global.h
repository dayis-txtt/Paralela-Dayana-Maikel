#ifndef GLOBALS_H
#define GLOBALS_H

#include<stdio.h>
#include<stdlib.h>
#include<semaphore.h>
#include<pthread.h>
#include<string.h>
#include<unistd.h>
#include <time.h>

typedef struct { // Estadisticas para el final del programa
    long tiempoEsperaTotal;   // ms
    long tiempoServicioTotal; // ms
    int totalAtendidos;
    int atendidosPorTipo[3];
    pthread_mutex_t lock;
} Estadisticas;

//estados de los mostradores
typedef enum {
    OPEN,
    SERVING,
    ON_BREAK
} EstadoMostrador;

//tipos de mostradores
typedef enum {
    ECONOMY_COUNTER,
    BUSINESS_COUNTER,
    INTERNATIONAL_COUNTER
} TipoMostrador;

typedef struct {
    int id;
    TipoMostrador tipo;
    EstadoMostrador estado;
    int pasajerosAtendidos;   // cuántos ha atendido desde el último descanso
    int K;                    // cuántos debe atender antes del próximo descanso
    pthread_mutex_t lock;
    pthread_cond_t cond;
} Mostrador;

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

typedef struct {     // Maneja el acceso
    int indiceMostrador;  // índice en el arreglo mostradores[]
    Fila* filas;          // acceso a las 3 filas
} ArgsMostrador;

// Variables globales (extern dice "esto existe en otro lado")
extern int N, M, K_min, K_max, T_max, Q, perRest;
extern pthread_mutex_t lockRest;
extern int N, M, K_min, K_max, T_max, Q;
extern int perRest;
extern int* pasajeros;
extern char* clase[];
extern pthread_mutex_t lockRest;
extern Estadisticas stats;
extern Mostrador* mostradores;

#endif