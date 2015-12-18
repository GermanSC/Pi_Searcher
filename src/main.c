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
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>

#define FILE_LENGTH 100000002
//#define FILE_LENGTH 2002

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

void printDiffTime(struct timespec tsin, struct timespec tsout, char* str)
{
	printf("Tiempo %s: %4u.%09u segundos\n",
				str,
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

	if (arglist[optind] == NULL)
	{
		printHelp(arglist[0]);
		return -1;
	}

	return optind;
}

void* loadFileToMem( void )
{
	void* file_memory;
	int fd = -1;

	fd = open("fpi.txt",O_RDONLY);
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
		printf("%*.*s ", nums+4, pos, (char *)(file_mem));

	}else{

	printf("... %.*s ", nums, (char *)(file_mem + pos - nums));

	}
	printf("< %.*s > ",	len, (char *)(file_mem + pos));

	if( pos + len + nums >= FILE_LENGTH)
	{
		printf("%.*s ", (FILE_LENGTH - pos - len -1), (char *)(file_mem + pos + len));
		printf("%*s", (nums + 5 - (FILE_LENGTH - pos - len)), " ");

	}else{

	printf("%.*s ... ", nums, (char *)(file_mem+pos+len));

	}

	printf("@ POS: %d\n", pos);

	return 0;
}

int lookFor(char * str, void* file_mem, unsigned int offset, unsigned int range)
{
	signed	 int cmp;
	unsigned int i;
	unsigned int cnt;
	unsigned int arg_len = strlen(str);
	FILE* temp_file, *temp_result;

	temp_file = fopen("/tmp/mitemp","w+b");
	if(temp_file == NULL )
	{
		printf("Error al abrir el archivo temporal.\n");
		return -1;

	}
	temp_result = fopen("/tmp/mitempres","w+b");
	if(temp_file == NULL )
	{
		printf("Error al abrir el archivo temporal.\n");
		return -1;

	}

	while(arg_len > 0)
	{
		cnt = 0;
		cmp = 0;
		i = 0;
		while( i != (range - 1) )
		{
			cmp = strncmp(str, (char*) (file_mem + offset + i), arg_len);
			if(cmp == 0)
			{
				cnt++;
				if(arg_len == strlen(str))
				{
					fprintf(temp_file,"%u\n",i);
				}
			}
			i++;
		}
		fprintf(temp_result,"%u\n",cnt);
//		printf("Apariciones de %*.*s: %u \n", (int)strlen(str), arg_len, str, cnt);
		arg_len--;
	}

	fclose(temp_file);
	fclose(temp_result);
	return 0;
}

int printOccur(unsigned int nums, char * str, void* file_mem)
{
	int read;
	unsigned int order = 0;
	unsigned int pos;
	FILE* tmp;
	unsigned int len = strlen(str);

	tmp = fopen("/tmp/mitempres","r");

	printf("\n");
	if(tmp == NULL )
	{
		printf("Error al abrir el archivo temporal.\n");
		return -1;
	}

	while(len > 0)
	{
		read = fscanf(tmp,"%u",&pos);
		printf("Apariciones de %*.*s: %u \n", (int)strlen(str), len, str, pos);
		len--;
	}

	fclose(tmp);
	unlink("/tmp/mitempres");

	printf("\n");
	tmp = fopen("/tmp/mitemps","r");
	if(tmp == NULL )
	{
		printf("Error al abrir el archivo temporal.\n");
		return -1;
	}

	while(1)
	{
		order++;
		read = fscanf(tmp,"%u",&pos);
		if(read != EOF)
		{
			printPos(order, pos, nums, strlen(str), file_mem );
		} else {
			break;
		}
	}

	fclose(tmp);
	printf("\n");
	return 0;
}

int sortResults(void)
{
	FILE* tmp;
	tmp = fopen("/tmp/mitemps","w");
	if(tmp == NULL )
	{
		printf("Error al abrir el archivo temporal.\n");
		return -1;
	}
	system("sort -n '/tmp/mitemp' > '/tmp/mitemps'");
	unlink("/tmp/mitemp");
	return 0;
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

	struct timespec ts_in,ts_out, ts_int;

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
	printf("Listo.\n\n");

	clock_gettime(CLOCK_MONOTONIC,&ts_int);

	printf("Ejecutando búsqueda...\n");

	/*	Busqueda e Impresión de resultados	*/
	ctrl = lookFor(argv[index],fm, 0, FILE_LENGTH);

	clock_gettime(CLOCK_MONOTONIC,&ts_out);
	printDiffTime(ts_in, ts_int, "de carga   ");
	printDiffTime(ts_int, ts_out, "de búsqueda");
	printDiffTime(ts_in, ts_out, "Total      ");

	ctrl = sortResults();
	ctrl = printOccur(N, argv[index],fm);

	/*----	Fin de Programa		----*/
	unlink("/tmp/mitemps");
	ctrl = munmap (fm, FILE_LENGTH);
	if (ctrl == -1)
	{
		printf("Error al desmontar la memoria\n");
		return -1;
	}
	return 0;
}
