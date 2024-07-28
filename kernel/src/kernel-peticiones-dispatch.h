#ifndef KERNEL_PETICIONES_DISPATCH_H
#define KERNEL_PETICIONES_DISPATCH_H

#include <utils/estructuras_compartidas.h> 
#include <commons/collections/dictionary.h>
#include <utils/protocolo.h>
#include <errno.h>
#include <utils/instrucciones.h>
#include "kernel-config.h"
#include "kernel-protocolo.h"
#include "planificacion.h"

extern int tam_cola_resource;
extern int esta_block;
extern int proceso_finalizado_por_consola;
extern t_list** colas_resource_block;

void hilo_motivo_de_desalojo();
void* escuchar_peticiones_dispatch();
const char* transformar_motivos_a_exit(t_tipo_instruccion* motivo_inicial);
void peticion_fin_quantum();
void peticion_exit(const char *tipo_de_exit);
void peticion_wait();
void peticion_signal();
void mover_a_bloqueado_por_wait(t_pcb* pcb, char* recurso);
int obtener_indice_recurso(char* recurso);
bool recurso_existe(char* recurso);
t_pcb_cpu* recibir_contexto_y_recurso(char** recurso);
void peticion_IO();
void rcv_nombre_recurso(char** recurso, int socket);
void inicializar_vector_recursos_pedidos();
int calcular_total_instancias();
void finalizar_por_invalidacion(t_pcb* pcb, const char* tipo_invalidacion);
void liberar_recurso_por_exit(t_pcb* pcb);
void mostrar_pcb(t_pcb* pcb);
void inicializar_cola_resource_block();
int inicializar_tam_cola_resources();
void actualizar_pcb(t_pcb*, t_pcb_cpu*);
void liberar_procesos(t_pcb* pcb);
void liberar_interfaces_y_args(void *elemento);
void mover_a_cola_block_general(t_pcb* pcb, char* motivo);

#endif // KERNEL_PETICIONES_DISPATCH_H