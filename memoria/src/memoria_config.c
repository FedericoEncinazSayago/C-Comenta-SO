#include "memoria_config.h"

t_config_memoria* inicializar_config_memoria(void){

    t_config_memoria* config_memoria = malloc(sizeof(t_config_memoria));
    config_memoria->puerto_escucha = 0;
    config_memoria->tam_memoria = 0;
    config_memoria->tam_pagina = 0;
    config_memoria->path_instrucciones = 0;
    config_memoria->retardo_respuesta = 0;
    return config_memoria;
}


int cargar_configuraciones_memoria(t_config_memoria* config_memoria) { //(char *path_config_memoria

    t_config* config = config_create("memoria.config"); // creamos momentaneamente un t_config para "leer" las cosas del .config

    if(config_memoria == NULL) {
        log_error(logger, "No se pudo cargar la configuracio memoria");
        config_destroy(config);
        return -1;
    }

    char* configuraciones[] = {
        "PUERTO_ESCUCHA",
        "TAM_MEMORIA",
        "TAM_PAGINA",
        "PATH_INSTRUCCIONES",
        "RETARDO_RESPUESTA",
        NULL
    };

    if(!tiene_todas_las_configuraciones(config, configuraciones)) { 
        log_error(logger, "No se pudo cargar la configuracion de la memoria");
        config_destroy(config);
        return -1;
    }
    
    copiar_valor(&config_memoria->path_instrucciones, config_get_string_value(config, "PATH_INSTRUCCIONES"));

    config_memoria->puerto_escucha = config_get_int_value(config, "PUERTO_ESCUCHA");
    config_memoria->tam_memoria = config_get_int_value(config, "TAM_MEMORIA");
    config_memoria->tam_pagina = config_get_int_value(config, "TAM_PAGINA");
    config_memoria->retardo_respuesta = config_get_int_value(config, "RETARDO_RESPUESTA");

    log_info(logger, "Configuraciones cargadas correctamente");
    config_destroy(config); // Destruir el config usado para copiar desde el .config al config_memoria
    return 1;
}

void config_destroy_version_memoria(t_config_memoria* config_memoria) {
    free(config_memoria->path_instrucciones);
    free(config_memoria);
}