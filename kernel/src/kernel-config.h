#ifndef KERNEL_CONFIG_H
#define KERNEL_CONFIG_H

#include <commons/config.h>
#include <utils/shared.h>
#include <utils/logger.h>

#include "kernel-estructuras.h"

// Funciones para inicializar:
t_config_kernel* inicializar_config_kernel();
t_config_kernel *cargar_config_kernel(char *path_config);
extern t_config* config;
// Funciones para cargar configuraciones:
void cargar_configuraciones(t_config_kernel* config_kernel, char *path_config);

// Funciones para cargar valores:
void cargar_valores_de_memoria(t_config *config, t_config_kernel *config_kernel);
void cargar_valores_de_cpu(t_config *config, t_config_kernel *config_kernel);
void cargar_valores_de_planificacion(t_config *config, t_config_kernel *config_kernel);
void cargar_valores_de_recursos(t_config *config, t_config_kernel *config_kernel);
void cargar_valores_de_grado_multiprogramacion(t_config *config, t_config_kernel *config_kernel);

// Funciones para obtener valores:
int get_socket_memoria();
int get_socket_dispatch();
int get_socket_interrupt();

// Funciones para setear valores:
void set_socket_memoria(int socket);
void set_socket_dispatch(int socket);
void set_socket_interrupt(int socket);

#endif // KERNEL_CONFIG_H