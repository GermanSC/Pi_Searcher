/*
 ============================================================================
 Name        : Pi_Searcher.c
 Author      : German Sc.
 Version     :
 Copyright   : Todo sumamente copyrighteado 2015
 Description : Busca cadenas numericas especificas en los 100.000.000
 	 	 	   primeros decimales de PI.
 ============================================================================
 */

#include <fcntl.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>

//#define FILE_LENGTH 100000002
#define FILE_LENGTH 2002

void printHelp(char* Name)
{
	printf( "\nUso: %s [opciones] secuencia\n\n", Name);
	printf( "  -N x	Número de elementos previos y posteriores a la secuencia que se desean\n"
			"	visualizar.\n"
			"  -T x	Número de hilos a utilizar para realizar la búsqueda.\n\n");
	printf( "El programa busca cadenas numericas solicitadas como argumento en los "
			"100.000.000 primeros numeros de pi. Devuele el tiempo transcurrido, tiempo de "
			"busqueda y cantidad de apariciones de la secuencia, y posteriormete detalla cada "
			"una de estas. Se puede configurar la cantidad de hilos a realizar la busqueda y la "
			"cantidad de números a visualizar antes y despues de cada aparición.\n\n");

}

int getOptions(int argcount, char *arglist[], int* thread_num, int* n_offset)
{
	int opt_sig;
	const char* const opc_cort = "hT:N:";

	do
	{
		opt_sig = getopt (argcount, arglist, opc_cort);

		switch (opt_sig)
		{
		case 'h':
			printHelp(arglist[0]);
			return -1;
			break;
		case 'N':
			*n_offset = atoi(optarg);
			break;
		case 'T':
			*thread_num = atoi(optarg);
			break;
		case '?':
			printHelp(arglist[0]);
			return -1;
			break;

		case -1:
			break;

		default:
			return -1;
		}
	} while (opt_sig != -1);

	return optind;
}

void * loadFileToMem( void )
{
	void* file_memory;
	int fd = -1;

	fd = open("pitest.txt",O_RDONLY);
	if(fd == -1)
	{
		return (void *)-1;
	}
	/* Create the memory mapping. */
	file_memory = mmap (0, FILE_LENGTH, PROT_READ, MAP_SHARED, fd, 0);
	close (fd);
	if(file_memory == (void *) -1)
	{
		return (void *) -1;
	}

	return file_memory;
}

int main(int argc, char *argv[])
{
	int N = 10;
	int T = 4;
	int index = 1;
	int ctrl = 0;
	void* fm = NULL;

	struct timespec ts_in,ts_out;

	clock_gettime(CLOCK_MONOTONIC,&ts_in);

	index = getOptions(argc, argv, &T, &N);
	if(index == -1)
	{
		return -1;
	}

	/*	Inicio	*/
	printf("\nPi Searcher:\n------------\n\n");
	printf("Configuración: N = %d | T = %d\n\n Cargando archivo...",N,T);

	fm = loadFileToMem();
	if(ctrl != 0)
	{
		printf("ERROR");
		return -1;
	}
	printf("Listo:\n");

	clock_gettime(CLOCK_MONOTONIC,&ts_out);
	printf("\nTiempo de inicio: %u.%09u segundos\n",
			(unsigned int)(ts_out.tv_sec - ts_in.tv_sec),
			(unsigned int)(ts_out.tv_nsec - ts_in.tv_nsec));

	/*	FIXME - CONTINUAR ACA	*/
	char * ptr;

	ptr = strstr((char *)fm,"594553");

	printf("Lei: %d\n", ptr - (char *)fm);

	ctrl = munmap (fm, FILE_LENGTH);
	if (ctrl == -1)
	{
		printf("Error al desmontar la memoria\n");
		return -1;
	}
	return 0;
}
