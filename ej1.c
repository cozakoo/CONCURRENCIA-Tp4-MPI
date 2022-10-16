#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mpi.h>

int main(int argc, char* argv[])
{
    int my_rank;
    int size;
    int tag = 0;
    char msj[255] = "ping-pong";
    MPI_Status  status;
    
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Barrier(MPI_COMM_WORLD);

    //Si soy proceso par envio un mensaje al siguiente proceso consecutivo
    if (my_rank %2 == 0) {
        MPI_Send(&msj, 1, MPI_INT, my_rank+1, tag, MPI_COMM_WORLD);
        printf("Soy el proceso %d y le mande un mensaje al proceso %d \n", my_rank, my_rank+1);
    }else{
        // Somos los porcesos impares
        int rankAnterior= my_rank-1; 
        MPI_Recv(&msj, 1, MPI_INT, rankAnterior, tag , MPI_COMM_WORLD, &status);
        printf("El proceso %d esta recibiendo del proceso %d el siguiente mensaje: %s. \n", my_rank, rankAnterior, msj);
    }

    MPI_Barrier(MPI_COMM_WORLD);  
    MPI_Finalize();
}