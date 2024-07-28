#include "memoria_conexiones.h"


int crear_servidores(t_config_memoria* config_memoria, int *md_generico) {
    char* puerto_memoria = string_itoa(config_memoria->puerto_escucha);
    *md_generico = iniciar_servidor("MEMORIA", NULL, puerto_memoria);
    free(puerto_memoria); // Liberamos la memoria utilizada por string_itoa
    return (*md_generico != 0) ? 1 : -1;
}

int iniciar_modulo(t_config_memoria* config_memoria) {
    int md_generico = 0;
    if (crear_servidores(config_memoria, &md_generico) != 1) {
        log_error(logger, "No se pudo crear los servidores de escucha");
        return -1;
    }

    while (1) {
        int socket_cliente = esperar_cliente("MEMORIA", md_generico);
        if (socket_cliente != -1) {
            
            pthread_t hilo_memoria;
            int* args_hilo = malloc(sizeof(int)); // Creamos espacio para los argumentos
            *args_hilo = socket_cliente; // Asignamos el socket cliente a los argumentos
            //log_info(logger, "Se creo el hilo en %i" ,socket_cliente);
            pthread_create(&hilo_memoria, NULL, escuchar_peticiones, (void*) args_hilo);
            pthread_detach(hilo_memoria);
        }
    }
    return md_generico;
}

void cerrar_programa(t_config_memoria* config_memoria, int socket_server) {
    config_destroy_version_memoria(config_memoria);
    close(socket_server);
}
