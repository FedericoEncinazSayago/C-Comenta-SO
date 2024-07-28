#ifndef IO_OPERACIONES_DIALFS_H
#define IO_OPERACIONES_DIALFS_H

#include <commons/bitarray.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <utils/logger.h>

#include "io-estructuras.h"
#include "io-protocolo.h"
#include "io-utils.h"
#include "io-archivos-metadata.h"
#include "io-archivos-abiertos.h"
#include "io-bitmap.h"
#include "io-compactacion.h"

// Funciones para ejecutar las operaciones de DialFS:
void operacion_create_file(t_interfaz *interfaz, t_bitarray *bitmap, t_list *argumentos, t_list *archivos_ya_abiertos);
void operacion_write_file(t_interfaz *interfaz, void *bloques, t_list *argumentos, t_list *archivos_ya_abiertos);
void operacion_read_file(t_interfaz *interfaz, void *bloques, t_list *argumentos, t_list *archivos_ya_abiertos);
void operacion_truncate_file(t_interfaz *interfaz, void *bloques, t_bitarray *bitmap, t_list *argumentos, t_list *archivos_ya_abiertos);
void operacion_delete_file(t_interfaz *interfaz, t_bitarray *bitmap, t_list *argumentos, t_list *archivos_ya_abiertos);

#endif