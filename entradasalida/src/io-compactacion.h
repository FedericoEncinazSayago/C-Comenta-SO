#ifndef IO_COMPACTACION_H
#define IO_COMPACTACION_H

// Incluyo las bibliotecas necesarias:
#include <commons/bitarray.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <utils/logger.h>

// Incluyo las bibliotecas propias:
#include "io-estructuras.h"
#include "io-utils.h"
#include "io-archivos-metadata.h"
#include "io-archivos-abiertos.h"
#include "io-bitmap.h"

// Funcion para compactar el FS:
void compactar_fs(t_interfaz  *interfaz, void *bloques, t_bitarray *bitmap, t_list *archivos_ya_abiertos, t_config *archivo_metadata, int cantidad_bloques_asignados_a_archivo_compactar, int bloque_inicial_archivo_a_compactar, int tam_nuevo_archivo_a_compactar);

#endif