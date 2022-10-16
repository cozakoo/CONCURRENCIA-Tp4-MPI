#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define ROOT 0 // identificar quien sera el proceso raiz

static long num_steps = 100000;

double step; //cantidad de triangulos

int main(int argc, char *argv[])
{
    int i, my_rank, size;
    
    double x[num_steps], pi=0.0;
    double y[num_steps];
    
    //double piFinal[num_steps];

    double sum=0.0;

    //step = 1.0/(double) num_steps;

    MPI_Status status;

    MPI_Init( &argc, &argv );
  
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    step = 1.0/(double) num_steps;

    if (my_rank == ROOT )
    {    
        for (i=0; i< num_steps; i++)
            x[i] = (i+0.5)*step;
    }

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Scatter(
                &x,// lo que se va a distribuir entre los procesos
                num_steps/size,  // numero de elementos que se envia a cada proceso
                MPI_DOUBLE,// tipo de dato de lo que se va a enviar
                &y, //en donde cada proceso hace la recepcion
                num_steps/size,  // numero de elementos a recibir
                MPI_DOUBLE,// tipo de dato de lo que se va a recibir
                ROOT,   // quien realiza el envio
                MPI_COMM_WORLD  //los procesos quienes participaban en la distribucion
    );

    printf("Soy el rank %d y estoy calculando mi pi parcial \n ", my_rank);

    for (i = 0; i < num_steps/size; i++)
        y[i] += 4.0/(1.0+y[i]*y[i]);
    

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Gather(
                &y,             //lo que se va a emitir
                num_steps/size,      // cantidad de elementos a emitir
                MPI_DOUBLE,     // tipo de dato de lo que se ca a emitir
                &x,       // en donde se recibe los elementos
                num_steps/size,      // cantidad de elementos a recibir
                MPI_DOUBLE,     // tipo de dato de lo que se va a recibir
                ROOT,           // quien lo va a recibir
                MPI_COMM_WORLD  // entre quienes se hace la reunion de datos
    );

    if (my_rank == ROOT)
    {
        double acum = 0.0;
               
        for (i = 0; i <num_steps; i++)
            acum += x[i];
        
        pi = step * acum;
        
        printf ("VALOR DE PI: %f \n", pi);
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
}