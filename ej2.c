#include <stdio.h>
#include<time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mpi.h>

#define ROOT 0
#define TOPE 100

int main(int argc, char* argv[])
{
    
    int         my_rank;       
    int         size;    
    int         dest;          // Rank del que recibe
    int         tag = 0;       // Tag del mensaje
    char        message[TOPE];  // Mensaje
    char        bufferRecieve[TOPE];              //caracter que va a recibir el resto de los procesos              
    MPI_Status  status;

    srand(time(NULL));

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    

    if (my_rank == ROOT)
    {   //lleno el arreglo
        for (int i=0; i < TOPE; i++)
            message[i]= 'A' + (random() % 26);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    if (my_rank == ROOT){
        for (dest=1; dest < size; dest++)
            MPI_Send(&message, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
    }else{
        MPI_Recv(&bufferRecieve, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        printf("Soy el proceso %d y he recibido informacion del proceso %d el siguiente mensaje:   %c \n",
                        my_rank,
                        status.MPI_SOURCE,
                        bufferRecieve[my_rank]
        );
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();

}