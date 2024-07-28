#ifndef COMUNICACIONES_H
#define COMUNICACIONES_H

#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <utils/socket.h>
#include <utils/logger.h>
#include <utils/instrucciones.h>
#include <utils/protocolo.h>
#include <stdlib.h>
#include <unistd.h>
#include <commons/collections/list.h>
#include <commons/bitarray.h>
#include "cpu.h"
#include "cpu-estructuras.h"

char* filtrar_nueva_linea(char* cadena);
int recv_pagina(int socket);
t_pcb_cpu* rcv_contexto_ejecucion_cpu(int socket_cliente);
void enviar_pcb_a_kernel(t_paquete* paquete_a_kernel);
void mostrar_pcb(t_pcb_cpu* pcb);
int recv_pagina(int socket);
void send_agrandar_memoria (int pid , int tamanio);
int recv_agrandar_memoria();
void enviar_pcb_a_kernel(t_paquete* paquete_a_kernel);
void enviar_a_leer_memoria(int pid,int direccionFIsica, int tamanio);
void send_escribi_memoria(int pid, int direccionFIsica, int tamanio, int valor);
int recv_escribir_memoria();
void configurar_senial_cierre_cpu();
void solicitar_tablas_a_memoria(int numero_pagina);
t_tabla_de_paginas_cpu* recv_tablas();
void send_escribi_memoria_string(int pid,int direccionFIsica, int tamanio,char* valor);
char* recv_escribir_memoria_string(int tamanio);
int comunicaciones_con_memoria_escritura(int valor);
int comunicaciones_con_memoria_lectura();
void solicitar_a_kernel_std(char* interfaz ,t_paquete* solicitar_std);
void* recv_leer_memoria(int tamanio);
char* comunicaciones_con_memoria_lectura_copy_string();
int comunicaciones_con_memoria_escritura_copy_string(char* valor);
char* concatenar_lineas(char* cadena);

#endif