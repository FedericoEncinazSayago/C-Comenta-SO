#ifndef IO_BITMAP_H
#define IO_BITMAP_H

#include <commons/bitarray.h>

#include "io-estructuras.h"
#include "io-archivos.h"
#include "io-utils.h"
#include "io-variables-globales.h"

// Funcion para crear un bitmap:
t_bitarray *crear_bitmap(t_interfaz *interfaz, char *modo_de_apertura);

// Funcion para inicializar un bitmap:
void inicializar_bitmap(t_bitarray *bitmap);

// Funcion para obtener un bloque libre:
int obtener_bloque_libre(t_bitarray *bitmap, t_interfaz *interfaz);

// Funciones para settear bloques como ocupados:
void set_bloque_ocupado(t_bitarray *bitmap, int bloque_inicial);

// Funcion para averiguar si hay los suficientes bloques libres:
int hay_suficientes_bloques_libres(t_bitarray *bitmap, t_interfaz *interfaz, int bloques_necesarios);

// Funcion para contar bloques libres:
int contar_bloques_libres(t_bitarray *bitmap, int inicio, int fin);

// Funcion para averiguar si hay los suficientes bloques libres:
int hay_suficientes_bloques_libres(t_bitarray *bitmap, t_interfaz *interfaz, int bloques_necesarios);

// Funcion para averiguar si hay bloques contiguos libres:
int no_hay_bloques_contiguos_libres(t_bitarray *bitmap, int bloque_final, int bloques_necesarios);

// Funcion para setear bloques como ocupados:
void set_bloques_como_ocupados(t_bitarray *bitmap, int bloque_final, int bloques_necesarios);

// Funcion para liberar bloques asignados:
void liberar_bloques_asignados(t_bitarray *bitmap, int bloque_inicial, int bloques_necesarios);

// Funcion para setear bloques como ocupados desde un bloque inicial:
void set_bloques_como_ocupados_desde(t_bitarray *bitmap, int bloque_inicial, int bloques_necesarios);

// Funcion para cerrar el bitmap:
void cerrar_bitmap(t_bitarray *bitmap);

#endif