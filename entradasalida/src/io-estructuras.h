#ifndef IO_ESTRUCTURAS_H
#define IO_ESTRUCTURAS_H

#include <utils/estructuras_compartidas.h>
#include <commons/config.h>

typedef struct {
    int TIEMPO_UNIDAD_TRABAJO;
    char *IP_KERNEL;
    char *PUERTO_KERNEL;
    char *IP_MEMORIA;
    char *PUERTO_MEMORIA;
    char *PATH_BASE_DIALFS;
    int BLOCK_SIZE;
    int BLOCK_COUNT;
    int RETRASO_COMPACTACION;
} t_config_io;

typedef struct {
    int socket_with_kernel;
    int socket_with_memoria;
    char *nombre;
    tipo_interfaz tipo;
    t_config_io *config;
} t_interfaz;

typedef struct {
    int direccion_fisica;
    int tamanio;
} t_direccion_fisica;

typedef struct {
    t_config *archivo_metadata;
    char *name_file;
} t_archivo_abierto;

#endif //IO_ESTRUCTURAS_H