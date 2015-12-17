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

void printDiffTime(struct timespec tsin, struct timespec tsout)
{
	printf("\nTiempo de inicio: %u.%09u segundos\n\n",
				(unsigned int)(tsout.tv_sec - tsin.tv_sec),
				(unsigned int)(tsout.tv_nsec - tsin.tv_nsec));
}

int getOptions(int argcount, char *arglist[], unsigned int* thread_num, unsigned int* n_offset)
{
	int opt_sig;
	signed int temp = 0;
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
			*n_offset = (unsigned int)temp;
			break;
		case 'T':
			temp = atoi(optarg);
			if(temp < 1)
			{
				printf("Valor de T inválido, se utilizará T = 4.\n");
				break;
			}
			*thread_num = (unsigned int)temp;
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
	printf("Aparición %d: ",order);

	if(nums > pos)
	{
		printf("%.*s ", pos, (char *)(file_mem));

	}else{

	printf("... %.*s ", nums, (char *)(file_mem + pos - nums));

	}
	printf("< %.*s > %.*s ... @ POS: %d\n",
			len, (char *)(file_mem + pos),
			nums, (char *)(file_mem+pos+len),
			pos);

	return 0;
}

void lookFor(char * str, void* file_mem)
{
	signed	 int cmp;
	unsigned int i;
	unsigned int cnt;
	unsigned int arg_len = strlen(str);

	while(arg_len > 0)
	{
		cnt = 0;
		cmp = 0;
		i = 0;
		while( i != (FILE_LENGTH - 1) )
		{
			cmp = strncmp(str, (char*) file_mem+i, arg_len);
			if(cmp == 0)
			{
				cnt++;
			}
			i++;
		}
		printf("Apariciones de %.*s: %u \n", arg_len, str, cnt);
		arg_len--;
	}
	printf("\n");
}


int main(int argc, char *argv[])
{
	/*	Variables y valores por defecto	*/
	signed	 int index		= 1;
	unsigned int ctrl		= 0;
	unsigned int N			= 10;
	unsigned int T			= 4;

	/*	Punteros	*/
	void* fm	= NULL;

	struct timespec ts_in,ts_out;

	/*	Inicio del programa	*/

	clock_gettime(CLOCK_MONOTONIC,&ts_in);

	index = getOptions(argc, argv, &T, &N);
	if(index == -1)	/*	ERROR DE OPCIONES	*/
	{
		return -1;
	}

	/*	Inicio	*/
	printf( "\nPi Searcher:\n--------------\n"
			"N = %d | T = %d\n--------------\n"
			"\nCargando archivo...",N,T);

	fm = loadFileToMem();
	if(ctrl != 0)	/*	ERROR DE CARGA	*/
	{
		printf("ERROR");
		return -1;
	}
	printf("Listo:\n");

	clock_gettime(CLOCK_MONOTONIC,&ts_out);
	printDiffTime(ts_in, ts_out);

	lookFor(argv[index],fm);

	/*----	Fin de Programa		----*/
	ctrl = munmap (fm, FILE_LENGTH);
	if (ctrl == -1)
	{
		printf("Error al desmontar la memoria\n");
		return -1;
	}
	return 0;
}
