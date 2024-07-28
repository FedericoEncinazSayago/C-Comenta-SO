#ifndef MEMORIA_GLOBALES_H
#define MEMORIA_GLOBALES_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <commons/collections/list.h>
#include <commons/bitarray.h>
#include <utils/socket.h>
#include <commons/config.h>
#include <commons/collections/dictionary.h>
#include "memoria_config.h"
#include "memoria_estructuras_compartidas.h"

extern t_dictionary *diccionario_de_instrucciones_porPID;
extern t_dictionary *diccionario_tabla_de_paginas_porPID;

extern t_tabla_de_paginas* tabla_de_paginas;
extern t_bitarray* bitmap;
extern void* espacio_de_usuario;
extern void* memoria_usuario_bitmap;

extern char* path_proceso;
extern t_config_memoria* config_memoria;
void free_parametro_array(void* array);
void crear_espacio_usuario();
void crear_bitmap();
void crear_config_memoria();
void crear_diccionario_tabla_de_paginas_porPID();
void inicializacion_diccionario();
void cerrar_programa_memoria(int signal);
void liberar_lista_instrucciones(void* lista);
void liberar_paginas_porPID(void *pagina);
void elemento_lista_instrucciones_destroyer(void* elemento);
void configurar_senial_cierre();
void liberar_lista_parametros(void* lista);
#endif