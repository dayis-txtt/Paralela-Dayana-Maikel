#ifndef COUNTER_H
#define COUNTER_H

#include "global.h" 

void push(Fila *fila, int clase);
Nodo* pop(Fila *fila);

void* counter_thread(void* arg);
void* supervise(void* arg);
void* balance(void* arg);

#endif