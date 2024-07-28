#ifndef IO_ARCHIVOS_ABIERTOS_H_
#define IO_ARCHIVOS_ABIERTOS_H_

#include <dirent.h>
#include <commons/config.h>
#include <utils/logger.h>
#include <utils/shared.h>

#include "io-estructuras.h"
#include "io-archivos-metadata.h"
#include "io-archivos.h"
#include "io-utils.h"

// Funcione para traer los archivos ya abiertos:
t_list *obtener_archivos_ya_abiertos(t_interfaz *interfaz);

// Funciones para setear los datos del archivo abierto:
void set_archivo_metada_en_archivo_abierto(t_archivo_abierto *archivo_abierto, t_config *archivo_metadata);
void set_name_file_en_archivo_abierto(t_archivo_abierto *archivo_abierto, char *name_file);
void set_nuevo_archivo_abierto(t_list *archivos_abiertos, char *name_file, t_config *archivo_metadata);

// Funciones para gettear los datos del archivo abierto:
t_config *get_archivo_metadata(t_archivo_abierto *archivo_abierto);

// Funciones para verificar si existe ya un archivo:
int ya_esta_abierto(t_list *archivos_abiertos, char *nombre_archivo);

// Funciones para obtener un archivo abierto:
t_archivo_abierto *obtener_archivo_abierto(t_list *archivos_abiertos, char *nombre_archivo);

// Funciones para cerrar un archivo abierto:
void cerrar_archivo_abierto(t_list *archivos_abiertos, char *nombre_archivo);

// Funciones para cerrar todos los archivos abiertos:
void cerrar_todos_los_archivos_abiertos(t_list *archivos_abiertos);

// Funciones para liberar un archivo abierto:
void liberar_archivo_abierto(void *archivo_abierto);

#endif