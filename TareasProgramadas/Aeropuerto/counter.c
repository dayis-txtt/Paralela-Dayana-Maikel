#include "counter.h"

void push(Fila *fila, int clase){
    Nodo *nuevo = (Nodo*) malloc(sizeof(Nodo)); // Aparta el espacio para el nodo
    nuevo->clase = clase; 
    clock_gettime(CLOCK_MONOTONIC, &nuevo->tiempoEspera);
    nuevo->sig = NULL; 

    pthread_mutex_lock(&fila->lock);    
    if (fila->fin == NULL) { // Si esta vacio, agrega en la primera pocision
        fila->inicio = nuevo;
        fila->fin = nuevo;
    } else {        // Si no esta vacia, agrega al final
        fila->fin->sig = nuevo;
        fila->fin = nuevo;
    }
    fila->cantidad++;

    pthread_cond_signal(&fila->cond);
    pthread_mutex_unlock(&fila->lock); // libera la fila ya que la modifico
}

Nodo* pop(Fila *fila) {
    pthread_mutex_lock(&fila->lock);

    while (fila->inicio == NULL) {    
        pthread_mutex_lock(&lockRest);
        if (perRest >= N) { // Si ya no quedan pasajeros por atender en todo el sistema, salir
            pthread_mutex_unlock(&lockRest);
            pthread_mutex_unlock(&fila->lock);
            return NULL; 
        }
        pthread_mutex_unlock(&lockRest);
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

void* counter_thread(void* arg) {
    ArgsMostrador* args = (ArgsMostrador*) arg;
    Mostrador* m = &mostradores[args->indiceMostrador];
    Fila* filas = args->filas;

    // Cada mostrador atiende su fila según su tipo
    // ECONOMY_COUNTER=0, BUSINESS_COUNTER=1, INTERNATIONAL_COUNTER=2
    int indiceFila = m->tipo;

    while (1) {
        // Revisa si quedan personas en la fila
        pthread_mutex_lock(&lockRest);
        if (perRest >= N) {
            pthread_mutex_unlock(&lockRest);
            break; // Sale del while y termina el hilo
        }
        pthread_mutex_unlock(&lockRest);
        
        // Esperar a que el mostrador esté en OPEN
        pthread_mutex_lock(&m->lock);
        while (m->estado == ON_BREAK) {
            pthread_cond_wait(&m->cond, &m->lock);
        }
        m->estado = SERVING;
        pthread_mutex_unlock(&m->lock);

        //Tomar un pasajero de la fila correspondiente, si no hay, termina
        Nodo* pasajero = pop(&filas[indiceFila]);
        if (pasajero == NULL) break; 

        //Simular tiempo de servicio (10-100 ms)
        int tiempoServicio = 10 + rand() % 91;
        usleep(tiempoServicio * 1000);

        //Calcular tiempo de espera del pasajero
        struct timespec ahora;
        clock_gettime(CLOCK_MONOTONIC, &ahora);
        long espera = (ahora.tv_sec - pasajero->tiempoEspera.tv_sec) * 1000
                    + (ahora.tv_nsec - pasajero->tiempoEspera.tv_nsec) / 1000000;

        //Imprimir resultado de proceso de cada pasajero
        printf("\tMostrador %d [%-15s] atendio pasajero clase %-15s | Espera: %ld ms | Servicio: %d ms\n",
            m->id, clase[m->tipo], clase[pasajero->clase], espera, tiempoServicio);

        pthread_mutex_lock(&stats.lock);
        stats.tiempoEsperaTotal += espera;
        stats.tiempoServicioTotal += tiempoServicio;
        stats.totalAtendidos++;
        stats.atendidosPorTipo[m->tipo]++; // Qué mostrador lo atendió
        pthread_mutex_unlock(&stats.lock);

        free(pasajero); // Despues del check-in el pasajero se va

        //Contar pasajero en el total de procesados
        pthread_mutex_lock(&lockRest);
        perRest++;
        // Si fue el ultimo pasajero, despierta a todos los hilos
        if (perRest >= N) { 
            for (int i = 0; i < 3; i++) {
                pthread_cond_broadcast(&filas[i].cond);
            }
        }
        pthread_mutex_unlock(&lockRest);
        
        //Contar pasajero atendido y verificar si toca descanso
        pthread_mutex_lock(&m->lock);
        m->pasajerosAtendidos++;
        m->estado = OPEN;
        if (m->pasajerosAtendidos >= m->K) {
            m->estado = ON_BREAK;
            m->pasajerosAtendidos = 0;
            m->K = K_min + rand() % (K_max - K_min + 1); // nuevo K para el próximo ciclo
            printf("AVISO: Mostrador %d entro en descanso\n", m->id);
        }
        pthread_mutex_unlock(&m->lock);
    }

    return NULL;
}

void* supervise(void* arg) {
    while (1) {
        pthread_mutex_lock(&lockRest);
        // Salir del while si el programa va a terminar
        if (perRest >= N) { 
            pthread_mutex_unlock(&lockRest);
            break; 
        }
        pthread_mutex_unlock(&lockRest);

        for (int i = 0; i < M; i++) { // Revisa los mostradores constantemente, si encuentra uno en break, le da su tiempo y lo devuelve al trabajo
            pthread_mutex_lock(&mostradores[i].lock);
            if (mostradores[i].estado == ON_BREAK) {
                // Simular tiempo de descanso (entre 50-150ms)
                usleep((50 + rand() % 101) * 1000);
                mostradores[i].estado = OPEN;
                printf("SUPERVISOR: Mostrador %d ha sido REABIERTO\n", mostradores[i].id);
                
                // Despertar al hilo del mostrador que está en cond_wait
                pthread_cond_signal(&mostradores[i].cond);
            }
            pthread_mutex_unlock(&mostradores[i].lock);
        }
        usleep(10000); // Pausa para no saturar el CPU
    }
    return NULL;
}

void* balance(void* arg) {
    Fila* filas = (Fila*) arg;

    while (1) {
        // Revisa si la simulación ya terminó
        pthread_mutex_lock(&lockRest);
        if (perRest >= N) {
            pthread_mutex_unlock(&lockRest);
            break;
        }
        pthread_mutex_unlock(&lockRest);

        // Revisar si Economy (0) o Business (1) superan Q (El maximo por fila)
        for (int i = 0; i < 2; i++) {
            while (1) {
                Nodo* nodo_a_mover = NULL;
                pthread_mutex_lock(&filas[i].lock);
                if (filas[i].cantidad > Q) {
                    nodo_a_mover = filas[i].inicio;             // Sacar al primer pasajero de la fila saturada
                    filas[i].inicio = filas[i].inicio->sig;
                    if (filas[i].inicio == NULL) filas[i].fin = NULL;
                    filas[i].cantidad--;
                }
                pthread_mutex_unlock(&filas[i].lock);

                // Si hay alguien, lo mete a International al final
                if (nodo_a_mover != NULL) {
                    pthread_mutex_lock(&filas[2].lock);
                    nodo_a_mover->sig = NULL;
                    if (filas[2].fin == NULL) {
                        filas[2].inicio = nodo_a_mover;
                        filas[2].fin = nodo_a_mover;
                    } else {
                        filas[2].fin->sig = nodo_a_mover;
                        filas[2].fin = nodo_a_mover;
                    }
                    filas[2].cantidad++;
                    printf("BALANCER: Fila %s supero Q=%d. Movio pasajero a International\n", clase[i], Q);
                    pthread_cond_signal(&filas[2].cond); // Avisar a mostradores internacionales
                    pthread_mutex_unlock(&filas[2].lock);
                } else {
                    break; // La fila ya no supera Q
                }
            }
        }

        //  Revisar el primer pasajero de Business que es el que mas lleva esperando, si excede T_max, lo manda a International
        Nodo* pasajero_prioridad = NULL;
        pthread_mutex_lock(&filas[1].lock);
        if (filas[1].inicio != NULL) {
            struct timespec ahora;
            clock_gettime(CLOCK_MONOTONIC, &ahora);
            long espera = (ahora.tv_sec - filas[1].inicio->tiempoEspera.tv_sec) * 1000
                        + (ahora.tv_nsec - filas[1].inicio->tiempoEspera.tv_nsec) / 1000000;
            
            if (espera > T_max) {
                pasajero_prioridad = filas[1].inicio;
                filas[1].inicio = filas[1].inicio->sig;
                if (filas[1].inicio == NULL) filas[1].fin = NULL;
                filas[1].cantidad--;
            }
        }
        pthread_mutex_unlock(&filas[1].lock);

        // Si hay alguien que esperó demasiado, meterlo al FRENTE de International (2)
        if (pasajero_prioridad != NULL) {
            pthread_mutex_lock(&filas[2].lock);
            pasajero_prioridad->sig = filas[2].inicio;
            filas[2].inicio = pasajero_prioridad;
            if (filas[2].fin == NULL) {
                filas[2].fin = pasajero_prioridad;
            }
            filas[2].cantidad++;
            printf("BALANCER: Pasajero Business supero T_max (%d ms). Movido al frente de International\n", T_max);
            pthread_cond_signal(&filas[2].cond);
            pthread_mutex_unlock(&filas[2].lock);
        }

        // Dormir un poco para no consumir el 100% de la CPU (revisa cada 20 ms)
        usleep(20000); 
    }
    return NULL;
}