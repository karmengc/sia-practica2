#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
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
    // Suspendemos el reloj durante el tiempo especificado
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wake_up_time, NULL);
}


int main(int argc, char *argv[]) {
	long long tm;
	uint32_t slot_id = 5001; // Identificador del Slot de memoria compartida
	void* slot;
	uint32_t dato=0;
	uint32_t lectura;
	int shm_fd;
	// Creamos el slot de memoria compartida
	shm_fd = crearSHM(slot_id,sizeof(uint16_t));
	// Mapeamos el slot de memoria compartida
	slot = mapearSHM(slot_id,sizeof(uint16_t));
	puts("Hello World!!!"); /* prints Hello World!!! */
	while(1)
	{
		// Bucle para 5 iteraciones
		for(int i=0; i<=5; i++){
			// Espera de 1s
			timer(&tm,1000000);
			// Escribimos en memoria compartida el dato del numero de iteracion
			escribirSHM(slot,&dato,sizeof(dato));
			// Leemos de memoria compartida el dato del numero de iteracion
			leerSHM(slot,&lectura,sizeof(dato));
			// Imprimimos por consola el numero de iteracion que se ha leido
			printf("Lectura = %d\n",lectura);
			dato++;
		}
		//Borramos el slot de memoria compartida
		borrarSHM(slot_id, slot, shm_fd, sizeof(uint16_t));
		return 0;
	}
}
