all: ej1 ej2 ej3 ej4

ej1: ej1.c
	mpicc ej1.c -o ej1

ej2: ej2.c
	mpicc ej2.c -o ej2

ej3: ej3.c
	mpicc ej3.c -o ej3

ej4: ej4.c
	mpicc ej4.c -o ej4 -lm

clean:
	rm ej1 ej2 ej3 ej4

