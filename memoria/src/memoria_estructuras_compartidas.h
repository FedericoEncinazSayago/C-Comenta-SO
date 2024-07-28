#ifndef MEMORIA_ESTRUCTURAS_COMPARTIDAS_H
#define MEMORIA_ESTRUCTURAS_COMPARTIDAS_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <commons/collections/dictionary.h>
#include <commons/bitarray.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <utils/estructuras_compartidas.h>
#include <utils/protocolo.h>
#include <utils/socket.h>
#include <utils/shared.h>
#include <utils/logger.h>
#include <utils/cositas.h>

typedef struct 
{
    int puerto_escucha;
    int tam_memoria;
    int tam_pagina;
    char* path_instrucciones;
    int retardo_respuesta;

} t_config_memoria;

typedef struct {
    int nro_pagina;
    int marco;
    int bit_validez;
} t_tabla_de_paginas;


#endif // MEMORIA_ESTRUCTURAS_COMPARTIDAS_H