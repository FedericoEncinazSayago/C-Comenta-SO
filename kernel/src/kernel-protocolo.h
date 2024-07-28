#ifndef KERNEL_PROTOCOLO_H
#define KERNEL_PROTOCOLO_H

// Incluyendo bibliotecas externas:
#include <commons/collections/list.h>
#include <utils/protocolo.h>
#include <utils/estructuras_compartidas.h>

// Incluyendo bibliotecas internas:
#include "kernel-estructuras.h"
#include "kernel-utils-interfaces.h"

// Funciones para enviar mensajes a la interfaz:
void send_message_to_interface(interface_io *interface, t_list *args, int *response, int socket);
void send_message_to_generic_interface(int socket, t_list *args, int *response);
void send_message_to_std_interface(int socket, t_list *args, int *response);
void send_message_to_dialfs_interface(int socket, t_list *args, int *response);
void send_message_to_dialfs_create_o_delete(int socket, t_list *args, int *response);
void send_message_to_dialfs_read_o_write(int socket, t_list *args, int *response);
void send_message_to_dialfs_truncate(int socket, t_list *args, int *response);

// Funciones para recibir mensajes de interfaces:
t_list * recv_interfaz_y_argumentos(int socket, int pid_proceso);

// Funciones auxiliares:
int parsear_int(void *buffer, int *desplazamiento);
char *parsear_string(void *buffer, int *desplazamiento);
t_list *obtener_argumentos(void *buffer, int *desplazamiento, int size, int operacion_a_realizar, int pid_proceso);
void obtener_argumentos_generica(t_list *argumentos, void *buffer, int *desplazamiento);
void obtener_argumentos_std(t_list *argumentos, void *buffer, int *desplazamiento, int size);
void obtener_argumentos_dialfs_create_o_delete(t_list *argumentos, void *buffer, int *desplazamiento);
void obtener_argumentos_dialfs_read_o_write(t_list *argumentos, void *buffer, int *desplazamiento, int size);
void obtener_argumentos_dialfs_truncate(t_list *argumentos, void *buffer, int *desplazamiento);
        
#endif // KERNEL_PROTOCOLO_H