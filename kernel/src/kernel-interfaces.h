#ifndef KERNEL_INTERFACES_H
#define KERNEL_INTERFACES_H

#include <commons/collections/dictionary.h>
#include <utils/socket.h>
#include <utils/protocolo.h>

#include "kernel-estructuras.h"
#include "kernel-protocolo.h"
#include "kernel-utils-interfaces.h"
#include "planificacion.h"

// Variables globales:
extern sem_t semaforo_interfaces;
// Funciones de manejo de interfaz desde el lado del kernel:
void handle_new_interface(void* arg);
void manage_interface(void *socket_cliente);

// Funciones de consumidores:
void create_consumer_thread(char *interface_name);
void consumers_pcbs_blockeds(void *args);

// Funciones manipuladoras de interfaz:
interface_io *initialize_interface();
void create_interface(int socket);

#endif // KERNEL_INTERFACES_H