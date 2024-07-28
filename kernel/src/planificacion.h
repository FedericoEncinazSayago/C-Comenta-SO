#ifndef PLANIFICACION_H
#define PLANIFICACION_H

#include <pthread.h>
#include <semaphore.h>
#include <utils/logger.h>
#include <utils/estructuras_compartidas.h>
#include <unistd.h>
#include <stdbool.h>
#include <commons/temporal.h>
#include "kernel-init.h"
#include <utils/protocolo.h>
#include <utils/cositas.h>

extern pthread_mutex_t mutex_estado_new;
extern pthread_mutex_t mutex_estado_block;
extern pthread_mutex_t mutex_estado_ready;
extern pthread_mutex_t mutex_estado_exec;
extern pthread_mutex_t mutex_cola_priori_vrr;
extern pthread_mutex_t mutex_proceso_exec;
extern pthread_mutex_t mutex_cola_block;
extern pthread_mutex_t mutex_exit;
extern sem_t sem_proceso_exec;
extern sem_t sem_multiprogramacion;
extern sem_t habilitar_corto_plazo;
extern sem_t hay_en_estado_ready;
extern sem_t hay_en_estado_new;
extern sem_t desalojo_proceso;
extern sem_t hay_proceso_en_bloq;
extern sem_t sem_vrr;

extern int esta_finalizado;
extern t_pcb* proceso_en_exec;
extern int quantum_restante;

void inicializacion_semaforos();
int generar_pid_unico();
void informar_a_memoria_creacion_proceso(char* archivo_de_proceso, int pid);
void creacion_proceso(char *archivo_de_proceso);
void agregar_a_cola_estado_new(t_pcb* proceso);
void* agregar_a_cola_ready();
// void mover_procesos_de_ready_a_bloqueado(t_pcb* proceso);
// void mover_procesos_a_bloqueado(t_pcb* proceso);
void mover_procesos_de_bloqueado_a_ready(t_pcb* proceso);
t_pcb* obtener_siguiente_a_ready();
void informar_a_memoria_liberacion_proceso(int pid);
void* elegir_algoritmo_corto_plazo();
void hilo_planificador_largoplazo();
void hilo_planificador_cortoplazo_fifo();
void hilo_planificador_cortoplazo_RoundRobin();
void hilo_planificador_cortoplazo_VRR();
void* planificador_cortoplazo_fifo(void* arg);
void* planificador_corto_plazo_RoundRobin(void* arg);
void* planificacion_cortoplazo_VRR();
void* quantum_handler(void* arg); 
void enviar_proceso_a_cpu(t_pcb* pcbproceso);
void prevent_from_memory_leaks();
void destruir_semaforos(); 
void mostrar_lista_de_pids_ready(t_list* lista);
void mostrar_pid_prima (t_pcb* pcb);
void mostrar_lista_de_pids_prima(t_list* lista);
void mostrar_pid (t_pcb* pcb);
void ingresar_a_exec(t_pcb* pcb);
void puede_ejecutar_otro_proceso();
void liberar_vector_recursos_pedidos();
t_pcb_cpu* rcv_contexto_ejecucion_cpu(int socket_cliente);


#endif