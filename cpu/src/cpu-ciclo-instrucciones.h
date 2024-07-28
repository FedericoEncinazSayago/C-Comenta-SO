
#ifndef CPU_CICLO_INSTRUCCIONES_H
#define CPU_CICLO_INSTRUCCIONES_H

#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <utils/socket.h>
#include <utils/logger.h>
#include <utils/instrucciones.h>
#include <utils/protocolo.h>
#include <utils/estructuras_compartidas.h>
#include "cpu.h"
#include "cpu-estructuras.h"
#include "comunicaciones.h"
#include "mmu.h"

int encontrar_int(void* registro, int tamanio);
void iniciar_ciclo_de_ejecucion(int socket_server ,int socket_cliente);
void* obtener_registro (char *registro);
void seguir_ciclo();
int espacio_de_registro(char* registro);
void tengoAlgunaInterrupcion();
void ejecutar_ciclo_instrucciones();
void fecth(int socket_server);
void ejecutar_instruccion(int socket_cliente);
void ejecutar_set(char* registro, char* valor);
void ejecutar_sum(char* registro_origen_char, char* registro_desitino_char);
void operar_con_registros(void* registro_destino, void* registro_origen, char* registro, char* operacion, int valor);
void ejecutar_sub(char* registro_origen_char, char* registro_desitino_char);
void ejecutar_JNZ(char* registro, char* valor);
void ejecutar_IO_GEN_SLEEP(char* interfazAUsar, char* tiempoDeTrabajo);
void liberar_pcb();
void liberar_mmu();
void liberar_elemento(void* elemento);
void liberar_lista(t_list* lista);
void liberar_instrucciones(t_instruccion* instruccion);

void ejecutar_MOV_IN(char* registro_Datos, char* registro_Direccion);
void ejecutar_MOV_OUT(char* Registro_Datos, char* Registro_Direccion);
void ejecutar_COPY_STRING(char* tam);
void ejecutar_COPY_STRING(char* tamanio);
void ejecutar_WAIT(char* recurso);
void ejecutar_SINGAL(char* recurso);
void ejecutar_RESIZE(char* tam);
void ejecutar_IO_STDIN_READ(char* interfaz, char* registro_direccion, char* registro_tamanio);
void ejecutar_IO_STDOUT_WRITE(char* interfaz, char* registro_direccion, char* registro_tamanio);
void ejecutar_IO_FS_CREATE(char* interfaz, char* nombre_archivo);
void ejecutar_IO_FS_DELETE(char* interfaz, char* nombre_archivo);
void ejecutar_IO_FS_TRUNCATE(char* intefaz, char* nombre_archivo, char* registro_tamanio);
void ejecutar_IO_FD_WRITE(char* intefaz, char* nombre_archivo, char* registro_direccion, char* registro_tamanio, char* puntero_archivo);
void ejecutar_IO_FS_READ(char* intefaz, char* nombre_archivo, char* registro_direccion, char* registro_tamanio, char* puntero_archivo);



#endif