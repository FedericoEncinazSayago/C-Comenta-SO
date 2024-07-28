#ifndef IO_ARCHIVOS_H
#define IO_ARCHIVOS_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <commons/string.h>
#include <commons/collections/queue.h>

#include "io-estructuras.h"
#include "io-utils.h"
#include "io-variables-globales.h"

// Funciones para abrir archivos:
void *mapear_archivo_bloques(t_interfaz *interfaz, char *modo_de_apertura);
FILE *abrir_archivo_metadata(t_interfaz *interfaz, char *name_file, char *modo_de_apertura);
FILE *abrir_archivo_bitmap(t_interfaz *interfaz, char *modo_de_apertura);

// Funciones auxiliares:
FILE *abrir_archivo(char *path, char *modo_de_apertura);
FILE *persistir_archivo(t_interfaz *interfaz, char *name_file, char *modo_de_apertura, int tamanio_archivo);

// Funcion para escribir en un archivo:
void escribir_contenido_en_bloques(void *bloques, t_queue *buffers);

// Funcion para liberar region de memoria mapeada:
void liberar_region_memoria_bloques(void *region, t_interfaz *interfaz);

#endif