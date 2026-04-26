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
int N, M, K_min, K_max, T_max, Q;
int* pasajeros;
char* clase[] = {"Economic", "Business", "International"};


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


Mostrador* mostradores;  // arreglo de todos los mostradores
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

typedef struct {
    int indiceMostrador;  // índice en el arreglo mostradores[]
    Fila* filas;          // acceso a las 3 filas
} ArgsMostrador;


void push(Fila*, int);
Nodo* pop(Fila *fila);

void* counter_thread(void* arg) {
    ArgsMostrador* args = (ArgsMostrador*) arg;
    Mostrador* m = &mostradores[args->indiceMostrador];
    Fila* filas = args->filas;

    // Cada mostrador atiende su fila según su tipo
    // ECONOMY_COUNTER=0, BUSINESS_COUNTER=1, INTERNATIONAL_COUNTER=2
    int indiceFila = m->tipo; // coincide con el índice de la fila

    while (1) {
        // Esperar a que el mostrador esté en OPEN
        pthread_mutex_lock(&m->lock);
        while (m->estado == ON_BREAK) {
            pthread_cond_wait(&m->cond, &m->lock);
        }
        m->estado = SERVING;
        pthread_mutex_unlock(&m->lock);

        //Tomar un pasajero de la fila correspondiente
        Nodo* pasajero = pop(&filas[indiceFila]);

        //Simular tiempo de servicio (10-100 ms)
        int tiempoServicio = 10 + rand() % 91;
        usleep(tiempoServicio * 1000);

        //Calcular tiempo de espera del pasajero
        struct timespec ahora;
        clock_gettime(CLOCK_MONOTONIC, &ahora);
        long espera = (ahora.tv_sec - pasajero->tiempoEspera.tv_sec) * 1000
                    + (ahora.tv_nsec - pasajero->tiempoEspera.tv_nsec) / 1000000;

        //Imprimir resultado
        char* tipoNombre[] = {"Economy", "Business", "International"};
        printf("Mostrador %d [%-15s] atendio pasajero clase %-15s | Espera: %ld ms | Servicio: %d ms\n",
            m->id,
            tipoNombre[m->tipo],
            tipoNombre[pasajero->clase],
            espera,
            tiempoServicio);

        free(pasajero);

        //Contar pasajero atendido y verificar si toca descanso
        pthread_mutex_lock(&m->lock);
        m->pasajerosAtendidos++;
        m->estado = OPEN;

        if (m->pasajerosAtendidos >= m->K) {
            m->estado = ON_BREAK;
            m->pasajerosAtendidos = 0;
            m->K = K_min + rand() % (K_max - K_min + 1); // nuevo K para el próximo ciclo
            printf("Mostrador %d entro en descanso\n", m->id);
        }
        pthread_mutex_unlock(&m->lock);
    }

    return NULL;
}
//Clase main
int main(int argc, char *argv[]){
    if (argc !=7) return 1; // No se pasan valores desde la terminal
    N      = atoi(argv[1]);
    M      = atoi(argv[2]);
    K_min  = atoi(argv[3]);
    K_max  = atoi(argv[4]);
    T_max  = atoi(argv[5]);
    Q      = atoi(argv[6]);


// Creacion e inicializacion de filas
    Fila filas[3]; 
    for (int i = 0; i < 3; i++){
        filas[i].inicio = NULL;
        filas[i].fin = NULL;
        filas[i].cantidad = 0;
        pthread_mutex_init(&filas[i].lock, NULL);
        pthread_cond_init(&filas[i].cond, NULL);
    }
//Inicialización de mostradores
    mostradores = malloc(M * sizeof(Mostrador));
    srand(time(NULL));  // inicializar semilla para rand()

for (int i = 0; i < M; i++) {
    mostradores[i].id = i;
    mostradores[i].estado = OPEN;
    mostradores[i].pasajerosAtendidos = 0;
    mostradores[i].K = K_min + rand() % (K_max - K_min + 1);

    // Dividir los mostradores en tercios por tipo
    if (i < M / 3) {
        mostradores[i].tipo = ECONOMY_COUNTER;
    } else if (i < 2 * M / 3) {
        mostradores[i].tipo = BUSINESS_COUNTER;
    } else {
        mostradores[i].tipo = INTERNATIONAL_COUNTER;
    }

    pthread_mutex_init(&mostradores[i].lock, NULL);
    pthread_cond_init(&mostradores[i].cond, NULL);
}

// ── Inicialización de pasajeros
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

// Lanzar hilos de mostradores
    pthread_t hilosMostradores[M];
    ArgsMostrador argsMostradores[M];

    for (int i = 0; i < M; i++) {
        argsMostradores[i].indiceMostrador = i;
        argsMostradores[i].filas = filas;
        pthread_create(&hilosMostradores[i], NULL, counter_thread, &argsMostradores[i]);
    }

//Print temporal para prueba
printf("\n--- Verificacion de Mostradores ---\n");
char* tipoNombre[] = {"Economy", "Business", "International"};
for (int i = 0; i < M; i++){
    printf("Mostrador %d | Tipo: %-15s | Estado: OPEN | K: %d\n",
        mostradores[i].id,
        tipoNombre[mostradores[i].tipo],
        mostradores[i].K);


}
for (int i = 0; i < M; i++) {
    pthread_join(hilosMostradores[i], NULL);
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