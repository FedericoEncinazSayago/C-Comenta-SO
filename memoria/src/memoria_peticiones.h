#ifndef MEMORIA_PETICIONES_H
#define MEMORIA_PETICIONES_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <utils/socket.h>
#include <utils/shared.h>
#include <utils/estructuras_compartidas.h>
#include <utils/protocolo.h>
#include <utils/logger.h>
#include <commons/memory.h>
#include "memoria.h"

extern sem_t enviar_instruc;
extern sem_t sem_lectura_archivo;

//void hacer_signal();
void inicializar_semaforo();
void* escuchar_peticiones(void* args);
void handshake_desde_memoria(int socket_cliente);
void crear_proceso(int socket_cliente);
void terminar_proceso(int socket_cliente);
void obtener_marco(int socket_cliente);
void resize_proceso(int socket_cliente);
void acceso_lectura(int socket_cliente);
void acceso_escritura(int socket_cliente);
int obtener_marco_libre(t_bitarray* bitmap);
void liberar_marco(int marco);
void enviar_respuesta_a_kernel(int socket_cliente);
void inicializar_bitmap();

#endif //MEMORIA_PETICIONES_H