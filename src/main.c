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
	int temp = 0;
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
			temp = atoi(optarg);
			if(temp < 1)
			{
				printf("Valor de N inválido, se utilizará N = 10.\n");
				break;
			}
			*n_offset = temp;
			break;
		case 'T':
			temp = atoi(optarg);
			if(temp < 1)
			{
				printf("Valor de T inválido, se utilizará T = 4.\n");
				break;
			}
			*thread_num = temp;
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

int printPos(unsigned int order, unsigned int pos, unsigned int nums, unsigned int len, void* file_mem)
{
	printf("Aparición %d: ... %.*s < %.*s > %.*s ... @ POS: %d\n",
			order,
			nums, (char *)(file_mem+pos-nums),
			len, (char *)(file_mem + pos),
			nums, (char *)(file_mem+pos+len),
			pos);

	return 0;
}

int main(int argc, char *argv[])
{
	/*	Variables y valores por defecto	*/
	int ctrl	= 0;
	int N		= 10;
	int T		= 4;
	int index	= 1;
	int arg_len = 0;
	int i		= 0;
	int cnt		= 0;

	/*	Punteros	*/
	void* fm	= NULL;

	struct timespec ts_in,ts_out;

	/*	Inicio del programa	*/

	clock_gettime(CLOCK_MONOTONIC,&ts_in);

	index = getOptions(argc, argv, &T, &N);
	if(index == -1)
	{
		return -1;
	}
	arg_len = strlen(argv[index]);
	/*	Inicio	*/
	printf("\nPi Searcher:\n--------------\n");
	printf("N = %d | T = %d\n--------------\n\n	Cargando archivo...",N,T);

	fm = loadFileToMem();
	if(ctrl != 0)
	{
		printf("ERROR");
		return -1;
	}
	printf("Listo:\n");

	clock_gettime(CLOCK_MONOTONIC,&ts_out);
	printf("\nTiempo de inicio: %u.%09u segundos\n\n",
			(unsigned int)(ts_out.tv_sec - ts_in.tv_sec),
			(unsigned int)(ts_out.tv_nsec - ts_in.tv_nsec));


	while( i != (FILE_LENGTH - arg_len) )
	{
		ctrl = strncmp(argv[index], (char*) fm+i, arg_len);
		if(ctrl == 0)
		{
			cnt++;
			printPos(cnt, i, N, arg_len, fm);
		}
		i++;
	}

	printf("\nEncontré: %d apariciones. El valor de i es %d \n", cnt, i);



	/*--	Fin de Programa		--*/
	ctrl = munmap (fm, FILE_LENGTH);
	if (ctrl == -1)
	{
		printf("Error al desmontar la memoria\n");
		return -1;
	}
	return 0;
}
