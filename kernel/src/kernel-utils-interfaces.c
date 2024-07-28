#include "kernel-utils-interfaces.h"

t_dictionary *dictionary_interfaces; // Diccionario de interfaces

void inicializar_diccionario_interfaces() {
    dictionary_interfaces = dictionary_create();
}
// Funciones de manejo de interfaz desde el lado del kernel:

void set_name_interface(interface_io *interface, char *name) {
    interface->name = name;
}

int get_socket_interface(interface_io *interface) {
    return interface->socket_interface;
}

void set_socket_interface(interface_io *interface, int socket) {
    interface->socket_interface = socket;
}

void add_interface_to_dict(interface_io *interface, char *key) { 
    dictionary_put(dictionary_interfaces, key, interface);
}

interface_io *get_interface_from_dict(char *key) {
    return dictionary_get(dictionary_interfaces, key); 
}

int consulta_existencia_interfaz(interface_io *interface) {
    return interface != NULL;
}

void set_tipo_interfaz(interface_io *interface, tipo_interfaz tipo) {
    interface->tipo = tipo;
}

int ya_esta_conectada_interface(char *name) {
    return dictionary_has_key(dictionary_interfaces, name);
}

void liberar_interfaces() {
    log_info(logger, "Liberando interfaces");
    if (dictionary_interfaces) {
        dictionary_destroy_and_destroy_elements(dictionary_interfaces, liberar_interfaz);
        dictionary_interfaces = NULL;
    }
}

void liberar_interfaz(void *interface) {
    interface_io *interface_a_liberar = (interface_io *) interface;
    if (interface_a_liberar) {
        // Liberamos memoria:
        free(interface_a_liberar->name);
        interface_a_liberar->name = NULL;

        if (!queue_is_empty(interface_a_liberar->args_process)) {
            log_info(logger, "Liberando procesos bloqueados en la interfaz %s", interface_a_liberar->name);

            queue_destroy_and_destroy_elements(interface_a_liberar->args_process, liberar_argumentos_proceso);
        } else {
            log_info(logger, "No hay argumentos de procesos bloqueados en la interfaz %s", interface_a_liberar->name);

            queue_destroy(interface_a_liberar->args_process);
        }

        if (!queue_is_empty(interface_a_liberar->process_blocked)) {
            log_info(logger, "Liberando argumentos de procesos bloqueados en la interfaz %s", interface_a_liberar->name);

            queue_destroy_and_destroy_elements(interface_a_liberar->process_blocked, (void*) liberar_procesos);
        } else {
            log_info(logger, "No hay procesos bloqueados en la interfaz %s", interface_a_liberar->name);

            queue_destroy(interface_a_liberar->process_blocked);
        }

        // Liberamos semaforos:
        sem_destroy(&interface_a_liberar->semaforo_used);
        sem_destroy(&interface_a_liberar->size_blocked);

        // Liberamos la interfaz:
        free(interface_a_liberar);
        interface_a_liberar = NULL;
    }
}

void liberar_argumentos_proceso(void *argumentos) {
    t_list *argumentos_a_liberar = (t_list *) argumentos;
    // Liberamos memoria:
    list_destroy_and_destroy_elements(argumentos_a_liberar, free);
}

// Funciones para pedir operaciones a la interfaz:

int acepta_operacion_interfaz(interface_io *interface, tipo_operacion operacion) {
    tipo_operacion operaciones[] = {
        IO_GEN_SLEEP_INT,
        IO_STDIN_READ_INT,
        IO_STDOUT_WRITE_INT,
        IO_FS_CREATE_INT,
        IO_FS_DELETE_INT,
        IO_FS_TRUNCATE_INT,
        IO_FS_WRITE_INT,
        IO_FS_READ_INT
    };

    tipo_interfaz tipo = interface->tipo;
    
    if(tipo == GENERICA)
        return operacion == operaciones[0];
    else if(tipo == STDIN)
        return operacion == operaciones[1];
    else if(tipo == STDOUT)
        return operacion == operaciones[2];
    else if(tipo == DIALFS)
        return operacion == operaciones[3] || operacion == operaciones[4] || operacion == operaciones[5] || operacion == operaciones[6] || operacion == operaciones[7];
    else
        return 0; // Operacion invalida
}

void set_estado_de_conexion_interface(interface_io *interface, int estado) {
    interface->esta_conectado = estado;
}

int estado_de_conexion_interface(interface_io *interface) {
    return interface->esta_conectado;
}