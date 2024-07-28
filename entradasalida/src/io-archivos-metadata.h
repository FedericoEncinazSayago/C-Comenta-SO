#ifndef IO_ARCHIVOS_METADATA_H_
#define IO_ARCHIVOS_METADATA_H_ 

// Incluimos las librerias necesarias:
#include <stdio.h>
#include <commons/config.h>
#include <commons/string.h>
#include <utils/logger.h>
#include <utils/shared.h>

// Incluimoos las librerias propias:
#include "io-estructuras.h"
#include "io-utils.h"
#include "io-archivos.h"

// Funciones para abrir los archivos:
t_config *abrir_archivo_metadata_config(t_interfaz *interfaz, char *name_file, char *modo_de_apertura);

// Funciones para validar los archivos:
int es_un_archivo_valido(t_config *archivo_metadata);

// Funciones para setear los datos del archivo metadata:
void set_bloque_inicial_en_archivo_metadata(t_config *archivo_metadata, int bloque_inicial);
void set_tamanio_archivo_en_archivo_metadata(t_config *archivo_metadata, int tamanio_archivo);
void set_archivo_metada_en_fs(t_config *archivo_metadata);

// Funciones para gettear los datos del archivo metadata:
int get_bloque_inicial(t_config *archivo_metadata);
int get_tamanio_archivo(t_config *archivo_metadata);

// Funciones para cerrar el archivo metadata:
void cerrar_archivo_metadata(t_config *archivo_metadata);

#endif