#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <pthread.h>
#include <sched.h>
#include "shared_mem.h"
#include <process.h>
#include "ini.h"

typedef struct
{
	int none;
	int shm_slot_1;
	int shm_slot_2;
	int priority;
    const char* shm_slot_contador;
	int priority2;
    const char* shm_slot_contador2;
} configuration;

static int handler(void* user, const char* section, const char* name,
                   const char* value)
{
    configuration* pconfig = (configuration*)user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("shm_index", "none")) {
        pconfig->none = atoi(value);
    } else if (MATCH("shm_index", "shm_slot_1")) {
        pconfig->shm_slot_1 = atoi(value);
    } else if (MATCH("shm_index", "shm_slot_2")) {
        pconfig->shm_slot_2 = atoi(value);
    } else if (MATCH("lector", "priority")) {
        pconfig->priority = atoi(value);
    } else if (MATCH("lector", "shm_slot_contador")) {
        pconfig->shm_slot_contador = strdup(value);
    } else if (MATCH("escritor", "priority2")) {
    	pconfig->priority2 = atoi(value);
    } else if (MATCH("escritor", "shm_slot_contador2")) {
        pconfig->shm_slot_contador2 = strdup(value);
    } else {
        return 0;  /* unknown section/name, error */
    }
    return 1;
}


void timer(long long *tm, int period_us)
{
	uint64_t time_now;
	uint64_t period_ns;
	struct timespec wake_up_time;
	long long *dummy;

	dummy = tm;

	// Leer el valor actual del reloj y se guarda
	ClockTime(CLOCK_MONOTONIC, NULL, &time_now);

	// Configuramos el timer para que salte a "X" nanosegundos
	period_ns = period_us*1000;
	time_now = time_now - time_now % period_ns + period_ns;
	wake_up_time.tv_sec = time_now/1000000000LL;
	wake_up_time.tv_nsec = time_now % 1000000000LL;

	// Suspendemos el relos durante el tiempo especificado
	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wake_up_time, NULL);
}

void *escribe(void *arg) {

	long long tm;
	uint32_t dato=0;
	void * slot= arg;
	while (1)
	{
		for(int i=0; i<=100; i++){
			// Espera de 1s
			timer(&tm,100000);
			// Escribimos en memoria compartida el dato del numero de iteracion
			escribirSHM(slot,&dato,sizeof(dato));
			dato++;
		}
		return 0;
	}

}

void *lee(void *arg) {

	long long tm;
	void * slot= arg;
	uint32_t lectura;
	while (lectura <= 100)
	{
		// Espera de 1s
		timer(&tm,1000000);
		// Escribimos en memoria compartida el dato del numero de iteracion
		leerSHM(slot,&lectura,sizeof(lectura));
		// Imprimimos por consola el numero de iteracion que se ha leido
		printf("Lectura = %d\n",lectura);
		if (lectura == 100) {
			lectura++;
		}
	}

	return 0;
}


int main(int argc, char *argv[]) {

	uint32_t slot_id = 5001; // Identificador del Slot de memoria compartida
	void* slot;
	int shm_fd;
	pthread_t lector;
	pthread_t escritor;
	pid_t idproceso;
	struct sched_param sp;
	struct sched_param idw;
	struct sched_param idr;
	int policy;


	configuration config;

	    if (ini_parse("test.ini", handler, &config) < 0) {
	        printf("Can't load 'test.ini'\n");
	        return 1;
	    }
	    printf("Constants config loaded from 'test.ini': NONE=%d, SHM_SLOT_1=%d, SHM_SLOT_2=%d\n",
	        config.none, config.shm_slot_1, config.shm_slot_2);

	    printf("Threads config from 'test.ini': priority=%d, shm_slot_contador=%s, priority2=%d\n , shm_slot_contador2=%s\n",
	        config.priority, config.shm_slot_contador, config.priority, config.shm_slot_contador2);



	    free((void*)config.shm_slot_contador);
	    free((void*)config.shm_slot_contador2);
	    //free((void*)config.shm_slot_2);




	// Creamos el slot de memoria compartida
	shm_fd = crearSHM(slot_id,sizeof(uint16_t));

	// Mapeamos el slot de memoria compartida
	slot = mapearSHM(slot_id,sizeof(uint16_t));

	//Control de prioridad del proceso
	idproceso = getpid();
	sched_getparam(idproceso, &sp);
	printf("La prioridad inicial del proceso principal es: %d\n", sp.sched_priority );

	//Cambiamos la prioridad
	sp.sched_priority = 30;
	sched_setparam(idproceso, &sp);

	//La siguiente línea no es realmetne necesaria pero la usamos para verificar que efectivamente se ha cambiado la prioridad.
	sched_getparam(idproceso, &sp);
	printf("Establecida prioridad del proceso principal a: %d\n\n", sp.sched_priority );

	puts("¡¡¡Comenzamos!!!");

	//Lanzamos ambos hilos y priorizamos
	pthread_create(&escritor, NULL, escribe, slot);
	pthread_getschedparam(escritor, &policy, &idw);
	printf("La prioridad inicial del hilo de ESCRITURA es: %d\n", idw.sched_priority );

	pthread_setschedprio(escritor, 35);

	//La siguiente línea no es realmetne necesaria pero la usamos para verificar que efectivamente se ha cambiado la prioridad.
	pthread_getschedparam(escritor, &policy, &idw);
	printf("Establecida prioridad del hilo de ESCRITURA a: %d\n\n", idw.sched_priority );


	pthread_create(&lector, NULL, lee, slot);
	pthread_getschedparam(lector, &policy, &idr);
	printf("La prioridad inicial del hilo de LECTURA es: %d\n", idr.sched_priority );

	pthread_setschedprio(lector, 35);

	//La siguiente línea no es realmetne necesaria pero la usamos para verificar que efectivamente se ha cambiado la prioridad.
	pthread_getschedparam(lector, &policy, &idr);
	printf("Establecida prioridad del hilo de LECTURA a: %d\n\n", idr.sched_priority );

	//Esperamos que acaben la ejecucuión los hilos.
	pthread_join(escritor, NULL);
	pthread_join(lector, NULL);

	//Borramos el slot de memoria compartida
	borrarSHM(slot_id, slot, shm_fd, sizeof(uint16_t));

	puts("¡¡¡Acabamos!!!");

	return 0;

}

