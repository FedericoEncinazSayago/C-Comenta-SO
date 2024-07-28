#ifndef MEMORIA_INSTRUCCIONES_H
#define MEMORIA_INSTRUCCIONES_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <commons/collections/dictionary.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <utils/estructuras_compartidas.h>
#include <utils/instrucciones.h>
#include <utils/protocolo.h>
#include <utils/socket.h>
#include <utils/shared.h>
#include <utils/logger.h>
#include "memoria.h"


extern pthread_mutex_t mutex_instrucciones;

char* crear_path_instrucciones(char* archivo_path);
void leer_archivoPseudo(int socket_kernel);
void enviar_instruccion_a_cpu(int socket_cpu);
void agregar_a_diccionario_instrucciones(int pid, t_list *lista_de_instrucciones);
t_list* obtener_lista_instrucciones(int pid);
void inicializacion_semaforos();
//void recibir_archi_pid(int socket_kernel, int *tam, char **archivo_path, int *pid);


#endif // MEMORIA_INSTRUCCIONES_H