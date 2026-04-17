#include <omp.h>
#include <stdio.h>

static long numsteps = 100000;
double step;

int main() {
long int i;
double x, pi, sum = 0.0;
int nthreads = 0;

step = 1.0 / (double) numsteps;

double tdata = omp_get_wtime();

//Paralelizar el cálculo de pi
#pragma omp parallel private(i, x) reduction(+:sum)
{
    int tid = omp_get_thread_num();
    if (tid == 0) {
        nthreads = omp_get_num_threads();
        printf("Number of threads = %d\n", nthreads);
    }

    //Calcular la suma de cada hilo
    #pragma omp for
    for (i=0; i <numsteps; i++){
        x = (i+0.5)*step;
        sum += 4.0/(1.0+x*x);
    }
}

pi = step * sum;
tdata = omp_get_wtime() - tdata; //Calcular el tiempo de ejecución

printf("pi = %.12f in %f secs with %d threads\n", pi, tdata, nthreads);
return 0;
}