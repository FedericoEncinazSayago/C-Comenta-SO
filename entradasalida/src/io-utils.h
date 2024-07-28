#ifndef IO_UTILS_H
#define IO_UTILS_H

// Incluimos las librerias externas:
#include <stdio.h>
#include <math.h>
#include <utils/socket.h>

// Incluimos las librerias necesarias:
#include "io-config.h"
#include "io-estructuras.h"
#include "io-archivos-metadata.h"
#include "io-archivos-abiertos.h"

// Setea el socket con la memoria:
void set_socket_memory(int socket, t_interfaz * interfaz);

// Devuelve el socket con la memoria:
int get_socket_memory(t_interfaz * interfaz);

// Setea el socket con el kernel:
void set_socket_kernel(int socket, t_interfaz * interfaz) ;

// Devuelve el socket con el kernel:
void set_socket_kernel(int socket, t_interfaz * interfaz);

// Setea el nombre de la interfaz:
int get_socket_kernel(t_interfaz * interfaz);

// Devuelve el nombre de la interfaz:
void set_interfaz_name(char* name, t_interfaz * interfaz) ;

// Devuelve el nombre de la interfaz:
char *get_interfaz_name(t_interfaz * interfaz);

// Devuelve el tiempo de una interfaz:
int get_tiempo_unidad(t_interfaz * interfaz);

// Devuelve el path de una interfaz:
char *get_path_dial_fs(t_interfaz * interfaz);

// Devuelve el tamaño del bitmap:
size_t get_tamanio_bitmap(t_interfaz *interfaz);

// Devuelve la cantidad de bloques:
int get_block_count(t_interfaz *interfaz);

// Devuelve el tamaño de un bloque:
int get_block_size(t_interfaz *interfaz);

// Devuelve el total de bytes de una lista de direcciones:
int get_total_de_bytes(t_list *direcciones);

// Devuelve el retardo de compactacion:
int get_retardo_compactacion(t_interfaz *interfaz);

// Devuelve el nombre de una operación:
char *get_nombre_operacion(tipo_operacion operacion);

// Devuelve el tipo de interfaz:
tipo_interfaz get_tipo_interfaz_to_int(t_interfaz *interfaz);

// Devuelve si una dirección es mayor, menor o igual a otra:
int ordenar_direcciones_por_tamanio(void *direccion1, void *direccion2);

// Devuelve el modo de apertura:
char *get_modo_de_apertura(int size); 

// Devuelve el path de un archivo:
char *get_path_archivo(t_interfaz *interfaz, char *name_file);

// Devuelve los bloques necesarios para un archivo:
int get_bloques_necesarios(t_interfaz *interfaz, int nuevo_tamanio);

// Devuelve el bloque final de un archivo:
int calcular_bloque_final(t_interfaz *interfaz, int bloque_inicial, int tamanio_archivo);

// Funcion para calcular la cantidad de bloques asignados a un archivo:
int calcular_cantidad_bloques_asignados(t_interfaz *interfaz, int tamanio_archivo);

// Funcion para comparar bloques iniciales:
int comparar_bloque_inicial(void *archivo1, void *archivo2);

// Funcion para eliminar un archivo del sistema:
void eliminar_archivo_en_fs(t_interfaz *interfaz, char *nombre_archivo);

// Funcion para cerrar los sockets:
void cerrar_sockets(t_interfaz *interfaz);

// Funcion para liberar la memoria de una interfaz:
void liberar_interfaz(t_interfaz *interfaz);

// Funcion para liberar una direccion fisica:
void liberar_direccion_fisica(void *direccion);

// Funcion para retardo de compactacion:
void retardo_compactacion(t_interfaz *interfaz);

// Funcion para aplicar unidad de trabajo:
void aplicar_unidad_trabajo(t_interfaz *interfaz);

#endif // IO_UTILS_H