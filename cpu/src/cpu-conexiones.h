#ifndef CPU_CONEXIONES_H
#define CPU_CONEXIONES_H

#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <utils/socket.h>
#include <utils/logger.h>
#include <stdatomic.h>
#include <utils/protocolo.h>

#include "cpu.h"
#include "cpu-estructuras.h"
#include "cpu-ciclo-instrucciones.h"
#include "comunicaciones.h"

extern atomic_int interrupt_flag;

// Funciones para manejo de clientes y conexiones del modulo:
void crear_servidores_cpu(int* md_cpu_ds, int* md_cpu_it);
void* generar_conexion_a_memoria();
int generar_servidor_cpu_dispatch();
int generar_servidor_cpu_interrupt();
void* server_interrupt(void* args);
void generar_handshake_para_pagina(int socket, char *server_name, char *ip, char *puerto);



#endif