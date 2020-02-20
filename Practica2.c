#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <sys/sched.h>
#include <sys/types.h>
#include <process.h>
#include "sharedmem.h"

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
void readLoop(read_params rp)
{
	long long tm;
	uint32_t lectura;
	uint32_t dato=0;
	int pid;
	//int prio;
	pid = getpid();
    printf( "My pid is %d\n", pid );
    printf("prioridad hilo2");
    //prio=getprio(pid);
    //printf("hilo2 = %d\n",prio);
	for(int i=0; i<=100; i++){
		// Espera de 1s = 1000000 microsg
		timer(&tm,1000000);
		// Leemos de memoria compartida el dato del numero de iteracion
		leerSHM(rp.slot,&lectura,rp.size);
		printf("Lectura = %d\n",lectura);
		dato++;
	}
}
void writeLoop(write_params wp)
{
	long long tm;
	int pid;
	//int prio;
	pid = getpid();
    printf( "My pid is %d\n", pid );
    //printf("prioridad hilo1");
    //prio=getprio(pid);
    //printf("hilo1 = %d\n",prio);
	// Bucle para 100 iteraciones
	for(int i=0; i<=100; i++){
		// Espera de 1s = 1000000 microsg
		timer(&tm,1000000);
		escribirSHM(wp.slot,&wp.data,wp.size);
	}
}

int main(int argc, char *argv[]) {

	write_params wp;
	read_params rp;
	uint32_t dato=0;
	uint32_t slot_id = 5005; // Identificador del Slot de memoria compartida
	void* slot;
	int shm_fd;

	// Creamos el slot de memoria compartida
	shm_fd = crearSHM(slot_id,sizeof(uint16_t));
	slot = mapearSHM(slot_id,sizeof(uint16_t));
	wp.slot = slot;
	wp.data = dato;
	wp.size = sizeof(dato);
	rp.data = 0;
	rp.slot = slot;
	rp.size = sizeof(dato);
	pthread_t hilo1;
	pthread_t hilo2;

	// Creamos el slot de memoria compartida
	//shm_fd = crearSHM(slot_id,sizeof(uint16_t));
	// Mapeamos el slot de memoria compartida
	//slot = mapearSHM(slot_id,sizeof(uint16_t));
	puts("Hello World!!!"); /* prints Hello World!!! */
	while(1)
	{

			//Creamos el hilo que se encargará de iterar 100 veces, esperando 100ms
			//entre cada iteración y escribiendo el dato en el slot
			if (pthread_create(&hilo1, NULL, writeLoop, &wp)){
				fprintf(stderr, "Error creating thread\n");
				return 1;
			}


			//Creamos el hilo que se encargará de iterar cada segundo indefinidamente
			//hasta que el contador del primer hilo pare y mientras lee dato del slot
			if (pthread_create(&hilo2, NULL, readLoop, &rp)){
				fprintf(stderr, "Error creating thread\n");
				return 1;
			}

			//Hacemos que el proceso principal espere por el hilo 1 de escritura
			pthread_join(hilo1,NULL);
			pthread_join(hilo2,NULL);
			//Borramos el slot de memoria compartida
			borrarSHM(slot_id, slot, shm_fd, sizeof(uint16_t));
			return 0;
	}
}

