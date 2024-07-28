#ifndef KERNEL_UTILS_INTERFACES_H
#define KERNEL_UTILS_INTERFACES_H

// Incluyendo bibliotecas necesarias:
#include <commons/collections/dictionary.h>
#include <utils/estructuras_compartidas.h>

// Incluyendo bibliotecas necesarias internas:
#include "kernel-estructuras.h"
#include "kernel-peticiones-dispatch.h"

extern t_dictionary *dictionary_interfaces;

// Funciones de manejo de interfaz desde el lado del kernel
void set_name_interface(interface_io *interface, char *name);
int get_socket_interface(interface_io *interface);
void set_socket_interface(interface_io *interface, int socket);
void add_interface_to_dict(interface_io *interfaces, char *key);
interface_io *get_interface_from_dict(char *key);
int consulta_existencia_interfaz(interface_io *interface); 
void set_socket_interface(interface_io *interface, int socket);
void set_tipo_interfaz(interface_io *interface, tipo_interfaz tipo);
int ya_esta_conectada_interface(char *name);
void inicializar_diccionario_interfaces();
// Funciones para pedir operaciones a la interfaz
int consulta_interfaz_para_aceptacion_de_operacion(interface_io *interface);
int acepta_operacion_interfaz(interface_io *interface, tipo_operacion operacion);

void set_estado_de_conexion_interface(interface_io *interface, int estado);
int estado_de_conexion_interface(interface_io *interface);

void liberar_interfaces();
void liberar_interfaz(void *interface);
void liberar_proceso_bloqueado(void *proceso);
void liberar_argumentos_proceso(void *argumentos);

#endif // KERNEL_UTILS_INTERFACES_H