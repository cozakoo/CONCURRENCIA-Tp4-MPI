/* Introducción a la Concurrencia
 * Licenciatura en Informática
 * UNPSJB - Sede Trelew 
 */

/* Juego de la vida resuelto con MPI.
 * Dado que se cuenta con un archivo Makefile,
 * para compilar:
 *
 * make
 *
 * Para ejecutar, el modo de uso es:
 *
 * mpiexec -np <n_procesos> ./jv_mpi <n_iteraciones> <mostrarCada> [-i] <bloques>
 *
 * n_procesos:		 Cantidad de procesos (debe tener raíz cuadrada entera)
 * n_iteraciones:	 Cantidad de iteraciones
 * mostrarCada:		 Número que indica cada cuántas iteraciones mostrar la grilla
 * -i:				 Indica que se cargarán los valores iniciales manualmente (si no
 *					 se utiliza esta opción, se inicializará aleatoriamente)
 * <bloques>		 Cantidad de bloques que va a tener el programa
 */

#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define N 128

int grilla[N][N];
int grillaAux[N][N];

int n; // La grilla sera de n x n (y su valor se determina a partir de size, asi: n = sqrt(size))

void iniGrillaAlt(int g[][N], int len)
{
	int i, j, val = 1;

	for(i = 0; i < len; ++i)
		for(j = 0; j < len; ++j)
			g[i][j] = (val ^= 1);
}

void iniGrillaAlt2(int g[][N], int len)
{
	int i, j, val = 0;

	for(i = 0; i < len; ++i)
		for(j = 0; j < len; ++j)
			g[i][j] = (val ^= 1);
}

void iniGrillaManual(int g[][N], int len)
{
	int i, j, val;

	printf("Ingrese valores 1 o 0:\n");
	for(i = 0; i < len; ++i) {
		for(j = 0; j < len; ++j) {
			printf("grilla[%d][%d]: ", i, j);
			scanf("%d", &val);
			// Me aseguro que sean 1 o 0 (0 es 0. Cualquier otro entero es 1)
			val = !val;
			g[i][j] = !val;
		}
		printf("\n");
	}
	printf("\n");
}

void iniGrillaRandom(int g[][N], int len)
{
	int i, j, val = 0;

	srand(getpid());
	for(i = 0; i < len; ++i)
		for(j = 0; j < len; ++j)
			g[i][j] = rand() % 2;
}

void printGrilla(int mat[][N], int len)
{
	int i, j;

	for(i = 0; i < len; ++i)
	{
		for(j = 0; j < len; ++j)
			printf( "%d ", mat[i][j]);
		printf("\n");
	}
	printf("\n");
}

int fila(int rank)
{
	return rank / n;
}

int columna(int rank)
{
	return rank % n;
}

int getRank(int x, int y)
{
	return x * n + y;
}

int main(int argc, char *argv[])
{ 
	int i, j, valor, rank, size, iteraciones = 1;
	int mi_i, mi_j, rankVecino, cantVecinosVivos, mostrar, mostrarCada = 1;

	int iniManual = 0; // El usuario cargará los valores iniciales manualmente

	//[MARTIN]
	int cantidadBloques,bloques=1; 

	MPI_Init(&argc, &argv); 

	MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
	MPI_Comm_size(MPI_COMM_WORLD, &size); 

	n = (int) sqrt((double) size); // La grilla es de n x n

	// Comprobar size válido (debe tener raíz cuadrada entera)
	if(sqrt((double) size) > (double) n) {
		if(rank == 0)
			printf("Error: la cantidad de procesos debe tener raíz cuadrada entera.\n");
		MPI_Barrier(MPI_COMM_WORLD);
		return 0;
	}

	//[MARTIN]
	sscanf(argv[4], "%d", &bloques); //tomo la cantidad de bloques

	if (n % bloques != 0){
		printf("El numero de bloque tiene que ser homogeneo");
		MPI_Barrier(MPI_COMM_WORLD);
		return 0;
	}

	// Comprobar validez de argumentos
	if((argc != 3 && argc != 4) || (argc == 4 && !(iniManual = !strncmp(argv[3], "-i", 2)))) {
		if(rank == 0)
			printf("Uso: mpiexec -np <n_procesos> %s <n_iteraciones> <mostrarCada> [-i] <cantidadBloques>\n"
					"n_procesos:\tCantidad de procesos (debe tener raíz cuadrada entera)\n"
					"n_iteraciones:\tCantidad de iteraciones\n"
					"mostrarCada:\tNúmero que indica cada cuántas iteraciones mostrar la grilla\n"
					"-i:\t\tIndica que se cargarán los valores iniciales manualmente\n"
					"cantidadBloques:\t numero de bloques que se van a utilizar\n",
					argv[0]);
		MPI_Barrier(MPI_COMM_WORLD);
		return 0;
	}

	sscanf(argv[1], "%d", &iteraciones); // Tomo las iteraciones de la linea de comandos (parámetro del usuario)
	sscanf(argv[2], "%d", &mostrarCada); // Tomo el parámetro mostrarCada

	mostrar = mostrarCada;

	//cantidadBloques = bloques; 
	
	if (rank == 0) {
		printf("Tamaño de grilla: %d x %d\n", n, n);
		printf("Iteraciones: %d\n",iteraciones);
		printf("Bloques: %d\n",bloques);
		if(mostrarCada > 1)
			printf("Mostrar cada: %d iteraciones\n\n", mostrarCada);
		else {
			// Reaseguro para que mostrarCada no tome valores inválidos
			mostrarCada = 1;
			printf("Mostrar cada: %d iteración\n\n", mostrarCada);
		}

		if(iniManual)
			iniGrillaManual(grilla, n);
		else
			iniGrillaRandom(grilla, n);
		printf("Grilla inicial:\n");
		printGrilla(grilla, n);
	}

	//[MARTIN]
	cantidadBloques= n/bloques;

//cada proceso se encargara de un bloque de celda y no de una sola celda como esta en el for de mas abajo 
/*
    MPI_Scatter(
        &grilla,             //lo que se va a emitir
        cantidadBloques, // cantidad de elementos a emitir
        MPI_INT,     // tipo de dato de lo que se ca a emitir
        &grillaAux,       // en donde se recibe los elementos
        cantidadBloques,      // cantidad de elementos a recibir
        MPI_INT,     // tipo de dato de lo que se va a recibir
        0,           // quien lo va a recibir
        MPI_COMM_WORLD  // entre quienes se hace la reunion de datos
    ); */

	for(i = 0; i < cantidadBloques; ++i)
		MPI_Bcast(&grilla[i][0], n, MPI_INT, 0, MPI_COMM_WORLD); 

	mi_i = fila(rank);
	mi_j = columna(rank);
/*
	printf("rank = %d, fila = %d, columna = %d, getRank = %d\n",
			rank, mi_i, mi_j, getRank(mi_i, mi_j));
*/
	while(iteraciones--) {

		// Enviar a todos los vecinos el estado de mi celda
		for(i = mi_i - 1; i < mi_i + 2; ++i) {
			for(j = mi_j - 1; j < mi_j + 2; ++j) {
				if(i == mi_i && j == mi_j)
					continue; // Me salteo a mi mismo como destinatario
				if(i < 0 || i == n || j < 0 || j == n)
					continue; // No existe vecino fuera de la grilla
	
				rankVecino = getRank(i, j);
				MPI_Send(&grilla[mi_i][mi_j], 1, MPI_INT, rankVecino, 0, MPI_COMM_WORLD); 
			}
		}
	
		// Recibir de todos los vecinos el estado de sus celdas (una celda cada uno)
		cantVecinosVivos = 0;
		for(i = mi_i - 1; i < mi_i + 2; ++i) {
			for(j = mi_j - 1; j < mi_j + 2; ++j) {
				if(i == mi_i && j == mi_j)
					continue; // Me salteo a mi mismo como destinatario
				if(i < 0 || i == n || j < 0 || j == n)
					continue; // No existe vecino fuera de la grilla
	
				rankVecino = getRank(i, j);
				MPI_Recv(&valor, 1, MPI_INT, rankVecino, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
				if(valor == 1)
					++cantVecinosVivos;
			}
		}

		// Actualizar el estado de mi celda según los vecinos vivos
		if(cantVecinosVivos < 2)
			grilla[mi_i][mi_j] = 0; // Muere de soledad
		else if(cantVecinosVivos >= 4)
			grilla[mi_i][mi_j] = 0; // Muere por sobrepoblación
		else if(cantVecinosVivos == 3)
			grilla[mi_i][mi_j] = 1; // Nace (tal vez ya estaba viva)

	
//		printf("mi_i = %d, mi_j = %d, grilla[%d][%d] = %d\n", mi_i, mi_j, mi_i, mi_j, grilla[mi_i][mi_j]);
	
		// Mostrar la grilla cada ciertas iteraciones (elegido por el usuario)
		if(!--mostrar) {
			mostrar = mostrarCada;
			// Todos los procesos envían el valor de su celda al proceso 0
			if(rank == 0) {
				for(i = 0; i < n; ++i) {
					for(j = 0; j < n; ++j) {
						if(i == 0 && j == 0)
							continue;
						rankVecino = getRank(i, j);
						MPI_Recv(&grilla[i][j], 1, MPI_INT, rankVecino, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
					}
				}
			} else {
				int f, c;
		
				f = fila(rank);
				c = columna(rank);
				MPI_Send(&grilla[f][c], 1, MPI_INT, 0, 0, MPI_COMM_WORLD); 
			}
		
			if(rank == 0) {
				printGrilla(grilla, n);
			}
		}
	}

	MPI_Finalize(); 
}

