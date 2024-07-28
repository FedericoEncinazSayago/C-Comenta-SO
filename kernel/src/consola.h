#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdlib.h> 
#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <utils/shared.h>
#include "planificacion.h"
#include "kernel-peticiones-dispatch.h"
#include "kernel-estructuras.h"


#define CENTINELA "\n"
#define MAX_COMMAND_LETTERS 100

extern int pid_buscado_global;

typedef struct {
    char* nombre;
    void* (*funcion)(void*); // Firma de las funciones!
} COMMAND;



// Consola:
void iniciar_consola();
// Readline:
void iniciar_readline();
char** completar_CComenta(const char* texto, int inicio, int fin);
char* generador_de_comandos(const char* texto, int estado);

// Ejecución de comandos:
int ejecutar_comando(char* linea);
COMMAND* encontrar_comando(char* nombre);

// Comandos
void *iniciar_proceso(void* args);
void *proceso_estado(void *args); 
char *estado_proceso(int estado);
void imprimir_proceso_exec();
void imprimir_procesos_en_cola(char* estado, t_list* cola);
void *multiprogramacion(void *args);
void *ejecutar_script(void *args);
void *finalizar_proceso(void *pid);
bool es_el_proceso_buscado(void* elemento);
bool existe_proceso_con_pid_ingresado(t_list* cola_a_buscar_pid, int pid_buscado);
t_pcb* pcb_encontrado(t_list* cola_a_buscar_pid, int pid_buscado);
void *detener_planificacion(void* args);
void *iniciar_planif(void* args);
void inicializar_mutex_consola();
void mover_todos_los_procesos_bloqueados_a_ready();
void eliminar_proceso_de_cola_recursos(int pid_buscado);

#endif