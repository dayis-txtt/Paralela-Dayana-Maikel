#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int **crearMatriz(int n);
void liberarMatriz(int **matriz, int n);
void llenarMatrizAleatoria(int **matriz, int n);
void imprimirMatriz(int **matriz, int n, const char *nombre);
void sumarMatriz(int **A, int **B, int **R, int n);
void restarMatriz(int **A, int **B, int **R, int n);
void strassenSecuencial(int **A, int **B, int **C, int n);
int esPotenciaDeDos(int n);

int main(int argc, char *argv[]) {
	int n;
	n = atoi(argv[1]);

	if (!esPotenciaDeDos(n)) {
		fprintf(stderr, "N debe ser potencia de 2 \n");
		return 1;
	}

	srand((unsigned int)time(NULL));

	int **A = crearMatriz(n);
	int **B = crearMatriz(n);
	int **C = crearMatriz(n);

	if (A == NULL || B == NULL || C == NULL) {
		liberarMatriz(A, n);
		liberarMatriz(B, n);
		liberarMatriz(C, n);
		return 1;
	}

	llenarMatrizAleatoria(A, n);
	llenarMatrizAleatoria(B, n);

	strassenSecuencial(A, B, C, n);

	imprimirMatriz(A, n, "A");
	imprimirMatriz(B, n, "B");
	imprimirMatriz(C, n, "C = A x B");

	liberarMatriz(A, n);
	liberarMatriz(B, n);
	liberarMatriz(C, n);

	return 0;
}

int **crearMatriz(int n) {
	int **matriz = (int **)malloc((size_t)n * sizeof(int *));
	if (matriz == NULL) {
		return NULL;
	}

	for (int i = 0; i < n; i++) {
		matriz[i] = (int *)calloc((size_t)n, sizeof(int));
		if (matriz[i] == NULL) {
			for (int j = 0; j < i; j++) {
				free(matriz[j]);
			}
			free(matriz);
			return NULL;
		}
	}

	return matriz;
}

void liberarMatriz(int **matriz, int n) {
	if (matriz == NULL) {
		return;
	}

	for (int i = 0; i < n; i++) {
		free(matriz[i]);
	}
	free(matriz);
}

void llenarMatrizAleatoria(int **matriz, int n) {
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			matriz[i][j] = rand() % 10;
		}
	}
}

void imprimirMatriz(int **matriz, int n, const char *nombre) {
	printf("\nMatriz %s:\n", nombre);
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			printf("%d ", matriz[i][j]);
		}
		printf("\n");
	}
}

void sumarMatriz(int **A, int **B, int **R, int n) {
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			R[i][j] = A[i][j] + B[i][j];
		}
	}
}

void restarMatriz(int **A, int **B, int **R, int n) {
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			R[i][j] = A[i][j] - B[i][j];
		}
	}
}

int esPotenciaDeDos(int n) {
	return n > 0 && (n & (n - 1)) == 0;
}

//Multiplicación de matrices usando el algoritmo de Strassen en secuencial
void strassenSecuencial(int **A, int **B, int **C, int n) {
	if (n == 1) {
		C[0][0] = A[0][0] * B[0][0];
		return;
	}

	int mitad = n / 2;

	int **A11 = crearMatriz(mitad);
	int **A12 = crearMatriz(mitad);
	int **A21 = crearMatriz(mitad);
	int **A22 = crearMatriz(mitad);

	int **B11 = crearMatriz(mitad);
	int **B12 = crearMatriz(mitad);
	int **B21 = crearMatriz(mitad);
	int **B22 = crearMatriz(mitad);

	int **M1 = crearMatriz(mitad);
	int **M2 = crearMatriz(mitad);
	int **M3 = crearMatriz(mitad);
	int **M4 = crearMatriz(mitad);
	int **M5 = crearMatriz(mitad);
	int **M6 = crearMatriz(mitad);
	int **M7 = crearMatriz(mitad);

	int **T1 = crearMatriz(mitad);
	int **T2 = crearMatriz(mitad);

	if (A11 == NULL || A12 == NULL || A21 == NULL || A22 == NULL ||
		B11 == NULL || B12 == NULL || B21 == NULL || B22 == NULL ||
		M1 == NULL || M2 == NULL || M3 == NULL || M4 == NULL ||
		M5 == NULL || M6 == NULL || M7 == NULL || T1 == NULL || T2 == NULL) {
		liberarMatriz(A11, mitad); liberarMatriz(A12, mitad); liberarMatriz(A21, mitad); liberarMatriz(A22, mitad);
		liberarMatriz(B11, mitad); liberarMatriz(B12, mitad); liberarMatriz(B21, mitad); liberarMatriz(B22, mitad);
		liberarMatriz(M1, mitad); liberarMatriz(M2, mitad); liberarMatriz(M3, mitad); liberarMatriz(M4, mitad);
		liberarMatriz(M5, mitad); liberarMatriz(M6, mitad); liberarMatriz(M7, mitad);
		liberarMatriz(T1, mitad); liberarMatriz(T2, mitad);
		return;
	}

	for (int i = 0; i < mitad; i++) {
		for (int j = 0; j < mitad; j++) {
			A11[i][j] = A[i][j];
			A12[i][j] = A[i][j + mitad];
			A21[i][j] = A[i + mitad][j];
			A22[i][j] = A[i + mitad][j + mitad];

			B11[i][j] = B[i][j];
			B12[i][j] = B[i][j + mitad];
			B21[i][j] = B[i + mitad][j];
			B22[i][j] = B[i + mitad][j + mitad];
		}
	}

	// M1 = (A11 + A22) * (B11 + B22)
	sumarMatriz(A11, A22, T1, mitad);
	sumarMatriz(B11, B22, T2, mitad);
	strassenSecuencial(T1, T2, M1, mitad);

	// M2 = (A21 + A22) * B11
	sumarMatriz(A21, A22, T1, mitad);
	strassenSecuencial(T1, B11, M2, mitad);

	// M3 = A11 * (B12 - B22)
	restarMatriz(B12, B22, T2, mitad);
	strassenSecuencial(A11, T2, M3, mitad);

	// M4 = A22 * (B21 - B11)
	restarMatriz(B21, B11, T2, mitad);
	strassenSecuencial(A22, T2, M4, mitad);

	// M5 = (A11 + A12) * B22
	sumarMatriz(A11, A12, T1, mitad);
	strassenSecuencial(T1, B22, M5, mitad);

	// M6 = (A21 - A11) * (B11 + B12)
	restarMatriz(A21, A11, T1, mitad);
	sumarMatriz(B11, B12, T2, mitad);
	strassenSecuencial(T1, T2, M6, mitad);

	// M7 = (A12 - A22) * (B21 + B22)
	restarMatriz(A12, A22, T1, mitad);
	sumarMatriz(B21, B22, T2, mitad);
	strassenSecuencial(T1, T2, M7, mitad);

	for (int i = 0; i < mitad; i++) {
		for (int j = 0; j < mitad; j++) {
			// C11 = M1 + M4 - M5 + M7
			C[i][j] = M1[i][j] + M4[i][j] - M5[i][j] + M7[i][j];

			// C12 = M3 + M5
			C[i][j + mitad] = M3[i][j] + M5[i][j];

			// C21 = M2 + M4
			C[i + mitad][j] = M2[i][j] + M4[i][j];

			// C22 = M1 - M2 + M3 + M6
			C[i + mitad][j + mitad] = M1[i][j] - M2[i][j] + M3[i][j] + M6[i][j];
		}
	}

	liberarMatriz(A11, mitad); liberarMatriz(A12, mitad); liberarMatriz(A21, mitad); liberarMatriz(A22, mitad);
	liberarMatriz(B11, mitad); liberarMatriz(B12, mitad); liberarMatriz(B21, mitad); liberarMatriz(B22, mitad);
	liberarMatriz(M1, mitad); liberarMatriz(M2, mitad); liberarMatriz(M3, mitad); liberarMatriz(M4, mitad);
	liberarMatriz(M5, mitad); liberarMatriz(M6, mitad); liberarMatriz(M7, mitad);
	liberarMatriz(T1, mitad); liberarMatriz(T2, mitad);
}

