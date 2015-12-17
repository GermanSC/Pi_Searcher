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

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

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

int main(int argc, char *argv[])
{
	int N = 10;
	int T = 4;
	int index = 1;

	printf("\nPi Searcher:\n------------\n\n");

	index = getOptions(argc, argv, &T, &N);
	if(index == -1)
	{
		return -1;
	}
	printf("Configuración: N = %d | T = %d\n",N,T);

	return EXIT_SUCCESS;
}
