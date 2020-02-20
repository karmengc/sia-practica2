#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include "sharedmem.h"

int crearSHM(uint32_t slotId, uint32_t size)
{
    int shm_fd;
    char name[100];
    // Convertimos el Id del slot de memoria
    sprintf(name, "AR_SHM_SLOT_NAME_FORMAT", slotId);
    // Creamos el objeto de memoria compartida con acceso de lectura y escritura
    shm_fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, S_IRWXU);
    if( shm_fd == -1 ) {
        perror("shm_open"); // Si no se puede crear, se indica
        exit(1);
    }
    // Definimos el tamaño de la memoria compartida
    if(ftruncate(shm_fd, size) == -1){
        perror("ftruncate"); // Si no se puede definir el tamaño, se indica
        exit(1);
    }
    return shm_fd;
}

void* mapearSHM(uint32_t slotId, uint32_t size)
{
    int shm_fd;
    void *shared_memory = (void *)0;
    char name[100];
    // Convertimos el Id del slot de memoria
    sprintf(name, "AR_SHM_SLOT_NAME_FORMAT", slotId);
    // Abrimos el objeto de memoria compartida con acceso de lectura y escritura
    shm_fd = shm_open(name, O_RDWR | O_CREAT, S_IRWXU);
    if( shm_fd == -1 ) {
        perror("shm_open"); // Si no se puede abrir, se indica
        exit(1);
    }
    if( shm_fd > 0 ) {
        // Mapeamos el objeto de memoria compartida
        shared_memory = mmap( 0, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0 );
        if(shared_memory == MAP_FAILED){
            perror("mmap"); // Si no se puede mapear, se indica
            exit(1);
        }
        return shared_memory;
    }
    return 0;
}

//void leerSHM(void* slot,void* data,uint32_t size)
void leerSHM(read_params rp)
{
	//Realizamos una copia de los datos del slot a data
	//memcpy(data, slot, size);
	memcpy(rp.data, rp.slot, rp.size);
}

//void escribirSHM(void* slot,void* data,uint32_t size)
void escribirSHM(write_params wp)
{
	//Realizamos una copia de los data al slot de memoria
	//memcpy(slot, data, size);
	memcpy(wp.slot, wp.data, wp.size);
}

void borrarSHM(uint32_t slotId, void* slot, uint32_t desObject, uint32_t size)
{
    char name[100];
    // Convertimos el Id del slot de memoria
    sprintf(name, "AR_SHM_SLOT_NAME_FORMAT", slotId);
    //Cerramos el objeto de memoria compartida
    if (close(desObject) == -1){
        perror("close"); // Si no se puede cerrar el slot, se indica
        exit(1);
    }
    //Desmapeamos el objeto de memoria compartida
    if (munmap(slot,size) == -1){
        perror("munmap"); // Si no se puede "desmapear", se indica
        exit(1);
    }
    //Desvinculamos el objeto de memoria compartida
    if (shm_unlink(name) == -1){
        perror("shm_unlink"); // Si no se puede desvincular el objeto, se indica
        exit(1);
    }
}
