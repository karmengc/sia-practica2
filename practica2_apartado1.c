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
#include "shared_mem.h"

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

	// Creamos el slot de memoria compartida
	shm_fd = crearSHM(slot_id,sizeof(uint16_t));

	// Mapeamos el slot de memoria compartida
	slot = mapearSHM(slot_id,sizeof(uint16_t));
	puts("¡¡¡Comenzamos!!!");

	//Lanzamos ambos hilos
	pthread_create(&escritor, NULL, escribe, slot);
	pthread_create(&lector, NULL, lee, slot);

	//Esperamos que acaben la ejecucuión los hilos.
	pthread_join(escritor, NULL);
	pthread_join(lector, NULL);

	//Borramos el slot de memoria compartida
	borrarSHM(slot_id, slot, shm_fd, sizeof(uint16_t));
	return 0;

	puts("¡¡¡Acabamos!!!");

}
