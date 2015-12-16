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

void printHelp(char* Name)
{
	printf( "\nUso: %s [opciones] secuencia\n\n", Name);
	printf( "  -N	Número de elementos previos y posteriores a la secuencia que se desean\n"
			"	visualizar.\n"
			"  -T	Número de hilos a utilizar para realizar la búsqueda.\n\n");
	printf( "El programa busca cadenas numericas solicitadas como argumento en los "
			"100.000.000 primeros numeros de pi. Devuele el tiempo transcurrido, tiempo "
			"de busqueda y cantidad de apariciones de la secuencia, y posteriormete detalla "
			"cada una de estas. Se puede configurar la cantidad de hilos a realizar la busqueda "
			"y la cantidad de números a visualizar antes y despues de cada aparición.\n\n");

}

int main(int argc, char *argv[])
{
	printf("Pi Searcher:\n");
	printHelp(argv[0]);

	return EXIT_SUCCESS;
}
