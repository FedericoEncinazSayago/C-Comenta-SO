#include "io-config.h"

t_config_io* inicializar_config_io() {
    t_config_io* config_io = malloc(sizeof(t_config_io));
    config_io->TIEMPO_UNIDAD_TRABAJO = 0;
    config_io->IP_KERNEL = NULL;
    config_io->PUERTO_KERNEL = NULL;
    config_io->IP_MEMORIA = NULL;
    config_io->PUERTO_MEMORIA = NULL;
    config_io->PATH_BASE_DIALFS = NULL;
    config_io->BLOCK_SIZE = 0;
    config_io->BLOCK_COUNT = 0;
    config_io->RETRASO_COMPACTACION = 0;
    
    return config_io;
}

// Funciones para configurar:

void configurar_valores_kernel(t_config_io *config_io, t_config *config) {
    copiar_valor(&config_io->IP_KERNEL, config_get_string_value(config, "IP_KERNEL"));
    copiar_valor(&config_io->PUERTO_KERNEL, config_get_string_value(config, "PUERTO_KERNEL"));
}

void configurar_valores_memoria(t_config_io *config_io, t_config *config) {
    copiar_valor(&(config_io->IP_MEMORIA), config_get_string_value(config, "IP_MEMORIA"));
    copiar_valor(&(config_io->PUERTO_MEMORIA), config_get_string_value(config, "PUERTO_MEMORIA"));
}

void configurar_tiempo_unidad(t_config_io *config_io, t_config *config) {
    config_io->TIEMPO_UNIDAD_TRABAJO = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
}

void configurar_valores_dialfs(t_config_io *config_io, t_config *config){
    copiar_valor(&(config_io->PATH_BASE_DIALFS), config_get_string_value(config, "PATH_BASE_DIALFS"));
    config_io->BLOCK_SIZE = config_get_int_value(config, "BLOCK_SIZE");
    config_io->BLOCK_COUNT = config_get_int_value(config, "BLOCK_COUNT");
    config_io->RETRASO_COMPACTACION = config_get_int_value(config, "RETRASO_COMPACTACION");
}

// Funciones para liberar:
void liberar_config_io(t_interfaz *interfaz, t_config_io *config_io) {

    // Liberamos valores de kernel:
    free(config_io->IP_KERNEL);
    free(config_io->PUERTO_KERNEL);

    // Liberamos valores de memoria:
    if(interfaz-> tipo == STDIN || interfaz->tipo == STDOUT || interfaz->tipo == DIALFS) {
        free(config_io->IP_MEMORIA);
        free(config_io->PUERTO_MEMORIA);
    }

    // Liberamos valores de dialfs:
    if(interfaz->tipo == DIALFS) 
        free(config_io->PATH_BASE_DIALFS);

    free(config_io);
}