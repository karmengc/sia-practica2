#ifndef SHAREDMEM_H_INCLUDED
#define SHAREDMEM_H_INCLUDED
typedef struct s_write_params {
		void* slot;
		void* data;
		uint32_t size;
} write_params;
typedef struct s_read_params {
		void* slot;
		void* data;
		uint32_t size;
} read_params;
int crearSHM(uint32_t slotId, uint32_t size);
void* mapearSHM(uint32_t slotId, uint32_t size);
void leerSHM(void* slot,void* data,uint32_t size);
void escribirSHM(void* slot,void* data,uint32_t size);
void borrarSHM(uint32_t slotId, void* slot, uint32_t desObject, uint32_t size);
#endif