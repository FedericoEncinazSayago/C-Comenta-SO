#ifndef IO_VARIABLES_GLOBALES_H_
#define IO_VARIABLES_GLOBALES_H_

// Incluimos las bibliotecas necesarias:
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <commons/collections/list.h>
#include <commons/bitarray.h>
#include <utils/socket.h>

// Incluimos bibliotecas propias:
#include "io-archivos-abiertos.h"
#include "io-archivos.h"
#include "io-estructuras.h"
#include "io-bitmap.h"
#include "io-utils.h"

// Definimos las variables globales:
extern t_list *archivos_ya_abiertos;
extern t_bitarray *bitmap;
extern FILE *archivo_bitmap;
extern FILE *archivo_bloque;
extern t_interfaz *interfaz;
extern char *name_interfaz;
extern char *config_path;
extern void *bitmap_data;
extern void *region_bloques;

// Funcion para cerrar programa:
void cerrar_programa(int signal);

// Funcion para configurar la senial de cierre:
void configurar_senial_cierre();

#endif 

