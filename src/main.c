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

#include <sys/ipc.h>
#include <fcntl.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/sem.h>

#define FILE_LENGTH 100000002

/*	Estructura de lista enlazada	*/
struct dato{
	unsigned int posicion;
	struct dato* siguiente;
};

/*	GLOBAL VARIABLES	*/
char* lookforme = NULL;
void* fm = NULL;
unsigned int*  found = NULL;
unsigned int*  done = NULL;
unsigned int*  sorted = NULL;
unsigned int T = 4;
volatile int cancel = -1;
volatile int search_over = 0;

struct dato *first, *last;

/*	Semaforo	*/
int sem_id;
struct sembuf sbuf;

/*	Mutexs	*/
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/*	FUNCIONES	*/

/*	Imprime la ayuda y el modo de uso	*/
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

/*	Imprime la diferencia de tiempo entre tsin y tsout, con str como cadena de indicación	*/
void printDiffTime(struct timespec tsin, struct timespec tsout, char* str)
{
	printf("Tiempo %s: %4u.%09u segundos\n",
				str,
				(unsigned int)(tsout.tv_sec - tsin.tv_sec),
				(unsigned int)(tsout.tv_nsec - tsin.tv_nsec));
}

/*	Obtiene las opciones de la llamada y redirige a la impresion de ayuda.	*/
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

/*	Carga el archivo con las cifras de pi en memoria.	*/
void* loadFileToMem( void )
{
	void* file_memory;
	int fd = -1;

	fd = open("fpi.txt",O_RDONLY);
	if(fd == -1)
	{
		printf("Error al cargar el archivo de datos.\n");
		return (void *)-1;
	}
	/* mapeo en memoria */
	file_memory = mmap (NULL, FILE_LENGTH, PROT_READ, MAP_SHARED, fd, 0);
	close (fd);
	if(file_memory == (void *) -1)
	{
		printf("Error al mapear el archivo de datos en memoria.\n");
		return (void *) -1;
	}

	return file_memory;
}

/*	Imprime el contenido de la memoria file_mem a partir del offset indicada por pos,
 *  ademas de los num valores anteriores y posteriores.	*/
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

/*	Busca cadenas de texto en las direcciones de memoria de file_mem, a partir de
 *  offset y durante range iteraciones almacendando apariciones de cada subcadena posible	*/
int lookFor(char * str, void* file_mem, unsigned int offset, unsigned int range)
{
	signed	 int cmp;
	unsigned int i;
	unsigned int len = 1;
	unsigned int arg_len = strlen(str);
	unsigned int thread_num = 0;

	struct dato* nuevo;

	cmp = 1;
	i = 0;

	thread_num = (unsigned int) (offset / (FILE_LENGTH/T));

	while( i != (range - 1) && cancel == -1 )
	{
		for(len = 1; len <= arg_len && cancel == -1 ; len++)
		{
			cmp = strncmp(str, (char*) (file_mem + offset + i), len);
			if(cmp == 0)
			{
				pthread_mutex_lock(&lock);

				*(found + len -1) += 1;
				if(len == arg_len)
				{
					nuevo = (struct dato *) malloc(sizeof(struct dato));
					if(nuevo == NULL)
					{
						printf("Error: Memoria insuficiente.\n");
						cancel = 0;
						break;
					}
					/* Guardo la posición */

					nuevo->posicion = i + offset;
					nuevo->siguiente = NULL;

					if( *(found + len -1) == 1)
					{
						first = nuevo;
						last = first;

					}else{
						last->siguiente = nuevo;
						last = nuevo;
					}
				}
				pthread_mutex_unlock(&lock);

			}else{
				break;
			}
		}

		if(thread_num == 0)
		{
			*(done) = i;
		}

		i++;
	}
	return 0;
}

/*	Recoge las apariciones de la cadena y las manda a imprimir mediante la funcion printPos */
int printOccur(unsigned int nums, char * str, void* file_mem)
{
	unsigned int len = strlen(str);
	unsigned int k;
	unsigned int t = 1;
	printf("\n");

	while(len > 0)
	{

		printf("Apariciones de %*.*s: %u \n", (int)strlen(str), len, str, *(found + len -1));
		len--;
	}
	printf("\n");

	if((*(found + strlen(str) -1)) != 0)
	{

		if((*(found + strlen(str) -1)) > 20)
		{
			printf("\nPresione ENTER para ver las apariciones de %s\n",lookforme);
			getchar();
		}

		for(k = 1 ; k <= *(found + strlen(str) -1); k++)
		{
			printPos(k, *(sorted + k -1), nums, strlen(str), file_mem );

			if(k > 20 * t)
			{
				t++;
				getchar();
			}

		}
	}
	return 0;
}

/* Ordena las apariciones de la cadena	*/
int sortResults(void)
{
	struct dato * ind = NULL;
	struct dato * temp = NULL;
	unsigned int j, aux;
	unsigned int n;
	unsigned int change = 0;

	n = *(found + strlen(lookforme) -1);

	sorted = malloc( n * sizeof(unsigned int) );

	ind = first;
	for(j = 0; j < (*(found + strlen(lookforme) -1)); j++ )
	{
		*(sorted + j ) = ind->posicion;
		temp = ind;
		ind = ind->siguiente;
		free(temp);
	}
	n++;
	do
	{
		change = 0;
		for(j = 1; j < n-1; j++)
		{
			if( *(sorted + j - 1) > *(sorted + j) )
			{
				aux = *(sorted + j);
				*(sorted + j) = *(sorted + j -1);
				*(sorted + j -1) = aux;
				change = 1;
			}
		}
		n--;
	}while(change != 0);

	return 0;
}

/* Funcion del hilo, calcula el rango que le correponde buscar y realiza la busqueda.	*/
void* threadFunc(void* parameter)
{
	unsigned int c = *(unsigned int *) parameter;
	unsigned int range;

	if( (c + (unsigned int) (FILE_LENGTH/T)) > FILE_LENGTH )
	{
		range = FILE_LENGTH - c;
	} else {
		range = (unsigned int)(FILE_LENGTH/T);
	}

	/*	Abro el semaforo	*/
	sbuf.sem_op = 1;
	semop(sem_id,&sbuf,1);

	lookFor(lookforme, fm, c, range);

	if(cancel != -1)
	{
		pthread_mutex_lock(&lock);
		cancel++;
		pthread_mutex_unlock(&lock);
		return NULL;
	}

	sbuf.sem_op = 1;
	semop(sem_id,&sbuf,1);

	return NULL;
}

/* Limpia y libera direcciones de memoria, semaforos y borra archivos temporales.	*/
int cleanUp(void * file_mem)
{
	int tmp;

	tmp = munmap (file_mem, FILE_LENGTH);
	if (tmp == -1)
	{
		printf("Error al desmontar la memoria\n");
	}

	tmp = semctl(sem_id, 0, IPC_RMID);
	if(tmp == -1)
	{
		printf("Error al eliminar el semaforo.\n");
	}

	free(found);
	free(done);
	free(sorted);

	return tmp;
}

void term(int signum)
{
	cancel = 0;
	printf("\nCerrando el programa, aguarde un momento.\n");

	while(cancel != T && search_over == 0 );

	printf("Limpiando archivos temporales...");
    cleanUp(fm);
    printf("Listo\n");

    exit(0);
}

void sigTermSetUp(void)
{
	struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = term;
    action.sa_flags = SA_RESTART;
    sigaction(SIGINT, &action, NULL);
}

/*	Inicializa el semaforo	*/
int semInit(void)
{
	int semid;
	key_t key;

	sbuf.sem_num = 0;
	sbuf.sem_op = -1;
	sbuf.sem_flg = SEM_UNDO;

	if ((key = ftok("fpi.txt", 'J')) == -1)
	{
		cleanUp(fm);
		printf("Error al crear la llave.\n");
		return -1;
	}

	semid = semget(key, 1, 0666 | IPC_CREAT);
	if(sem_id == -1)
	{
		cleanUp(fm);
		printf("Error al crear semaforo.\n");
		return -1;
	}

	return semid;
}

int memSetUp(void)
{
	int tmp;

	found = malloc( strlen(lookforme) * sizeof(unsigned int));
	if(found == NULL)
	{
		printf("Error 1: Memoria insuficiente.\n");
		return -1;
	}

	for(tmp = 0; tmp < strlen(lookforme); tmp++)
	{
		*(found + tmp) = 0;
	}

	done = malloc(sizeof(unsigned int));
	if(found == NULL)
	{
		printf("Error 5: Memoria insuficiente.\n");
		return -1;
	}

	first = (struct dato *) malloc(sizeof(struct dato));
	if(first == NULL)
	{
		printf("Error 2: Memoria insuficiente.\n");
		return -1;
	}

	last = (struct dato *) malloc(sizeof(struct dato));
	if(last == NULL)
	{
		printf("Error 3: Memoria insuficiente.\n");
		return -1;
	}
	return 0;
}

/*	Funcion ppal.	*/
int main(int argc, char *argv[])
{
	/*	Variables y valores por defecto	*/
	signed	 int index		= 1;
	unsigned int ctrl		= 0;
	unsigned int N			= 10;
	unsigned int per		= 0;
	unsigned int per_ant	= 0;

	/*	Variables de hilos	*/
	static unsigned int thread_offset;
	pthread_t threadid;
	unsigned int thread_arg;

	struct timespec ts_in,ts_out, ts_int;

	/*	Inicio del programa	*/
	clock_gettime(CLOCK_MONOTONIC,&ts_in);

	index = getOptions(argc, argv, &T, &N);
	if(index == -1)	/*	ERROR DE OPCIONES	*/
	{
		return -1;
	}

	sigTermSetUp();

	/*	Inicio	*/
	printf( "\nPi Searcher: %u\n--------------\n"
			"N = %d | T = %d\n--------------\n"
			"\nCargando archivo...",thread_offset,N,T);

	lookforme = argv[index];

	ctrl = memSetUp();
	if(ctrl != 0)
	{
		return -1;
	}

	fm = loadFileToMem();
	if(fm == (void*) -1)	/*	ERROR DE CARGA	*/
	{
		return -1;
	}

	/*	Inicialización de semaforo	*/
	sem_id = semInit();
	if(sem_id == -1)
	{
		printf("Error de creación de Semaforo.\n");
		ctrl = cleanUp(fm);
		return -1;
	}

	printf("Listo.\n\n");
	clock_gettime(CLOCK_MONOTONIC,&ts_int);

	printf("Ejecutando búsqueda... 0%%\n");

	/*	Busqueda */

	for(ctrl = 0; ctrl < T; ctrl++ )
	{
		thread_arg = (FILE_LENGTH / T) * ctrl;
		pthread_create(&threadid, NULL, (void *)&threadFunc , &thread_arg);
		sbuf.sem_op = -1;
		semop(sem_id,&sbuf,1);
	}

	/*	Espero a que terminen los hilos.	*/
	while( semctl(sem_id, 0, GETVAL) != T)
	{
		per = (unsigned int) ( (float)*(done) * T * 100 / FILE_LENGTH);
		if(per > per_ant + 10 && per != 100)
		{
			printf("Ejecutando búsqueda... %u%%\n", per);
			per_ant = per;
		}
	}

	sbuf.sem_op = -T;
	semop(sem_id,&sbuf,1);

	printf("Ejecutando búsqueda... 100%%\n\n");
	search_over = 1;

	/* Impresión de resultados	*/

	clock_gettime(CLOCK_MONOTONIC,&ts_out);
	printDiffTime(ts_in, ts_int, "de carga   ");
	printDiffTime(ts_int, ts_out, "de búsqueda");
	printDiffTime(ts_in, ts_out, "Total      ");

	ctrl = sortResults();

	ctrl = printOccur(N, argv[index], fm);

	/*----	Fin de Programa		----*/
	ctrl = cleanUp(fm);

	return ((ctrl == -1) ?  -1 : 0);
}
