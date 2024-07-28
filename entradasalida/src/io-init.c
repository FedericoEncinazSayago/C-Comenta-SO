#include "io-init.h"

t_interfaz *inicializar_interfaz(char *name_interfaz, char *config_path) {
    t_interfaz *interfaz = malloc(sizeof(t_interfaz));

    if (interfaz == NULL) {
        log_error(logger, "Error al asignar memoria para la interfaz");
        exit(EXIT_FAILURE);
    }

    interfaz->nombre = name_interfaz;
    interfaz->config = cargar_configuraciones(config_path, interfaz);

    return interfaz;
}

t_config_io *cargar_configuraciones(char *config_path, t_interfaz *interfaz) {
    t_config *config = config_create(config_path);

    if (config == NULL) {
        log_error(logger, "No se pudo cargar el archivo de configuración");
        exit(EXIT_FAILURE);
    }

    get_tipo_interfaz_with_config(config, interfaz);

    t_config_io *config_io = inicializar_config_io();

    if (config_io == NULL) {
        log_error(logger, "Error al asignar memoria para config_io");
        exit(EXIT_FAILURE);
    }

    switch (interfaz->tipo) {
        case GENERICA:
            cargar_configuraciones_generica(config, config_io);
            break;
        case STDIN:
        case STDOUT:
            cargar_configuraciones_std(interfaz->tipo, config, config_io);
            break;
        case DIALFS:
            cargar_configuraciones_dialfs(config, config_io);
            break;
        default:
            log_error(logger, "Tipo de interfaz desconocido");
            config_destroy(config);
            free(config_io);
            exit(EXIT_FAILURE);
    }

    config_destroy(config);

    return config_io;
}

void cargar_configuraciones_generica(t_config *config, t_config_io *config_io) {
    char *configuraciones[] = {
        "TIEMPO_UNIDAD_TRABAJO",
        "IP_KERNEL",
        "PUERTO_KERNEL",
        NULL
    };

    validar_configuraciones(config, configuraciones);
    configurar_valores_kernel(config_io, config);
    configurar_tiempo_unidad(config_io, config);
}

void cargar_configuraciones_std(tipo_interfaz tipo, t_config *config, t_config_io *config_io) {
    char *configuraciones[] = {
        "IP_KERNEL",
        "PUERTO_KERNEL",
        "IP_MEMORIA",
        "PUERTO_MEMORIA",
        NULL
    };

    validar_configuraciones(config, configuraciones);
    configurar_valores_kernel(config_io, config);
    configurar_valores_memoria(config_io, config);

    if (tipo == STDIN && !config_has_property(config, "TIEMPO_UNIDAD_TRABAJO")) {
        log_error(logger, "Falta la configuración TIEMPO_UNIDAD_TRABAJO en el archivo de configuración");
        exit(EXIT_FAILURE);
    }

    configurar_tiempo_unidad(config_io, config);
}

void cargar_configuraciones_dialfs(t_config *config, t_config_io *config_io) {
    char *configuraciones[] = {
        "TIEMPO_UNIDAD_TRABAJO",
        "IP_KERNEL",
        "PUERTO_KERNEL",
        "IP_MEMORIA",
        "PUERTO_MEMORIA",
        "PATH_BASE_DIALFS",
        "BLOCK_SIZE",
        "BLOCK_COUNT",
        NULL,
    };

    validar_configuraciones(config, configuraciones);
    configurar_tiempo_unidad(config_io, config);
    configurar_valores_kernel(config_io, config);
    configurar_valores_memoria(config_io, config);
    configurar_valores_dialfs(config_io, config);
}

void get_tipo_interfaz_with_config(t_config *config, t_interfaz *interfaz) {
    char *tipo = config_get_string_value(config, "TIPO_INTERFAZ");
    interfaz->tipo = get_tipo_interfaz(tipo);
}

tipo_interfaz get_tipo_interfaz(char *tipo) {
    if (string_equals_ignore_case(tipo, "GENERICA")) {
        return GENERICA;
    } else if (string_equals_ignore_case(tipo, "STDIN")) {
        return STDIN;
    } else if (string_equals_ignore_case(tipo, "STDOUT")) {
        return STDOUT;
    } else if (string_equals_ignore_case(tipo, "DIALFS")) {
        return DIALFS;
    }

    return -1;
}