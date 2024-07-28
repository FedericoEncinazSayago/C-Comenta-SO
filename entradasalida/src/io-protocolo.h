#ifndef IO_PROTOCOLOS_H
#define IO_PROTOCOLOS_H

// Incluye las biblotecas externas necesarias:
#include <commons/collections/list.h>
#include <utils/estructuras_compartidas.h>
#include <utils/protocolo.h>
#include <utils/logger.h>

// Incluye las estructuras necesarias:
#include "io-config.h"
#include "io-estructuras.h"
#include "io-utils.h"

// Funciones para enviar mensajes al kernel
void send_respuesta_a_kernel(int respuesta, t_interfaz * interfaz);
void send_interfaz_a_kernel(t_interfaz * interfaz);

// Funciones para recibir mensajes del kernel
int recibir_entero(int socket);
t_list *recibir_argumentos(t_interfaz *interfaz, int socket_kernel);
t_list *recibir_argumentos_para_dial(t_interfaz * interfaz, tipo_operacion tipo);
t_list *obtener_direcciones_fisicas(int size, int *desplazamiento, void *buffer);

// Funciones para enviar mensajes a la memoria:
void send_mensaje_a_memoria(t_interfaz * interfaz, char *mensaje);
void send_bytes_a_leer(t_interfaz *interfaz, int pid, t_list *direcciones, void *input, int bytes_leidos);
void send_bytes_a_grabar(t_interfaz * interfaz, int direccion_fisica, char *bytes, int bytes_a_leer);

// Funciones para recibir mensajes de la memoria
char *rcv_contenido_a_mostrar(t_interfaz *interfaz, t_list *direcciones_fisicas, int pid_proceso);

// Funciones auxiliares:
char *parsear_string(void *buffer, int *desplazamiento);
int parsear_int(void *buffer, int *desplazamiento);

#endif // IO_PROTOCOLOS_H