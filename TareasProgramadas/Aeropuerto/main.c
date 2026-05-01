// Compilar  ->  gcc main.c counter.c -o aeropuerto -pthread
// Ejecutar  ->  ./aeropuerto 50 9 3 5 500 10


#include "global.h"
#include "counter.h"

int N, M, K_min, K_max, T_max, Q;
int perRest = 0; 
int* pasajeros;
char* clase[] = {"Economic", "Business", "International"};
pthread_mutex_t lockRest = PTHREAD_MUTEX_INITIALIZER; // Necesario para terminar el programa
Estadisticas stats = {0, 0, 0, {0, 0, 0}, PTHREAD_MUTEX_INITIALIZER}; // Estadisticas finales
Mostrador* mostradores;  // Arreglo de todos los mostradores


int main(int argc, char *argv[]){
    if (argc !=7) return 1; // No se pasan valores desde la terminal
    struct timespec start_total, end_total;
    clock_gettime(CLOCK_MONOTONIC, &start_total);
    N      = atoi(argv[1]);
    M      = atoi(argv[2]);
    K_min  = atoi(argv[3]);
    K_max  = atoi(argv[4]);
    T_max  = atoi(argv[5]);
    Q      = atoi(argv[6]);

    long nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    printf("Hilos disponibles: %ld\tHilos de Counter: %d\tTotal de hilos usados: %d\n\n", nprocs, M, M+2);

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

    //  Inicialización de pasajeros
    pasajeros = malloc (N*(sizeof(int)));
    int temp;
    for (int i=0; i<N ; i++){
        temp = rand()%3;
        pasajeros[i] = temp;
        push(&filas[temp], temp);
    }   

    for (int i=0; i<3 ; i++){
        printf ("Fila de %s: %d pasajeros\n", clase[i], filas[i].cantidad);
    } printf("\n");

    //Crear mostradores
    pthread_t hilosMostradores[M];
    ArgsMostrador argsMostradores[M];

    for (int i = 0; i < M; i++) {
        argsMostradores[i].indiceMostrador = i;
        argsMostradores[i].filas = filas;
        pthread_create(&hilosMostradores[i], NULL, counter_thread, &argsMostradores[i]); // Envia a los counters a procesar pasajeros
    }


    // Crea el hilo balancer y lo envia a hacer su trabajo
    pthread_t hiloBalancer;
    pthread_create(&hiloBalancer, NULL, balance, filas);

    // Crea hilo Supervisor y lo envia a hacer su trabajo
    pthread_t hiloSupervisor;
    pthread_create(&hiloSupervisor, NULL, supervise, NULL);

    
    for (int i = 0; i < M; i++) {
        pthread_join(hilosMostradores[i], NULL);  // Une los hilos al final del programa
    }


    // Prints finales
    printf("\n\tREPORTE FINAL\n");
    printf("Pasajeros totales atendidos: %d\n", stats.totalAtendidos);
    
    if (stats.totalAtendidos > 0) {
        printf("Tiempo de espera promedio:   %.2f ms\n", 
               (double)stats.tiempoEsperaTotal / stats.totalAtendidos);
        printf("Tiempo de servicio promedio: %.2f ms\n", 
               (double)stats.tiempoServicioTotal / stats.totalAtendidos);
    }

    printf("\nDesglose por Mostrador:\n");
    printf("- Económicos:      %d pasajeros\n", stats.atendidosPorTipo[0]);
    printf("- Business:        %d pasajeros\n", stats.atendidosPorTipo[1]);
    printf("- Internacionales: %d pasajeros\n", stats.atendidosPorTipo[2]);
    
    // Imprime el tiempo total de ejecucion del programa
    clock_gettime(CLOCK_MONOTONIC, &end_total);
    double total_ms = (end_total.tv_sec - start_total.tv_sec) * 1000.0 +
                      (end_total.tv_nsec - start_total.tv_nsec) / 1000000.0;

    printf("\n>>> Tiempo total de simulacion: %.2f ms <<<\n", total_ms);
    return 0;
}