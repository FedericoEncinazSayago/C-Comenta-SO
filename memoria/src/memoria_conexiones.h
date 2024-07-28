#ifndef MEMORIA_CONEXIONES_H
#define MEMORIA_CONEXIONES_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <utils/estructuras_compartidas.h>
#include <utils/socket.h>
#include <utils/shared.h>
#include <utils/logger.h>
#include <utils/protocolo.h>
#include "memoria.h"

// Funcion para cargar configuraciones de Memoria:
t_config_memoria* inicializar_config_memoria(void);
int cargar_configuraciones_memoria(t_config_memoria* config_memoria);
void config_destroy_version_memoria(t_config_memoria* config_memoria);

// Funciones para manejo de clientes y conexiones del modulo:
int crear_servidores(t_config_memoria* config_memoria, int *md_generico);
int iniciar_modulo(t_config_memoria* config_memoria);
void cerrar_programa(t_config_memoria* config_memoria, int socket_server);

// Funciones de operaciones basicas del modulo:
t_procesar_conexion* crear_procesar_conexion(char* nombre, int socket_cliente);

#endif // MEMORIA_CONEXIONES_H