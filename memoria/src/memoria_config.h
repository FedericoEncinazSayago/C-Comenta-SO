#ifndef MEMORIA_CONFIG_H
#define MEMORIA_CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <utils/estructuras_compartidas.h>
#include <utils/instrucciones.h>
#include <utils/protocolo.h>
#include <utils/socket.h>
#include <utils/shared.h>
#include <utils/logger.h>

#include "memoria_estructuras_compartidas.h"

// Funcion para cargar configuraciones de Memoria:
t_config_memoria* inicializar_config_memoria(void);
int cargar_configuraciones_memoria(t_config_memoria* config_memoria);
void config_destroy_version_memoria(t_config_memoria* config_memoria);

#endif // MEMORIA_CONFIG_H