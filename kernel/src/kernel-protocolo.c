#include "kernel-protocolo.h"

// 1. Funciones para enviar mensajes a interfaces:

void send_message_to_interface(interface_io *interface, t_list *args, int *response, int socket) {
    switch(interface->tipo) {
        case GENERICA:
            send_message_to_generic_interface(socket, args, response);
            break;
        case STDIN:
        case STDOUT:
            send_message_to_std_interface(socket, args, response);
            break;
        case DIALFS:
            send_message_to_dialfs_interface(socket, args, response);
            break;
        default:
            // Manejo de error o caso por defecto
            break;
    }

    // Liberamos la memoria de los argumentos:
    list_destroy(args);
}

void send_message_to_generic_interface(int socket, t_list *args, int *response) {

    // Obtenemos los argumentos:
    int *pid_proceso_ptr = list_remove(args, 0);
    int *operacion_a_realizar_ptr = list_remove(args, 0);
    int *tiempo_sleep_ptr = list_remove(args, 0);

    // Enviamos la cantidad de bytes a enviar:
    int cant_de_bytes = sizeof(int) * 3;
    send(socket, &cant_de_bytes, sizeof(int), 0);

    // Enviamos el pid del proceso:
    int pid_proceso = *pid_proceso_ptr;
    if (send(socket, &pid_proceso, sizeof(int), 0) == -1) {
        perror("Error al enviar el pid del proceso");
        return;
    }

    // Enviamos la operación a realizar:
    int operacion_a_realizar = *operacion_a_realizar_ptr;
    if (send(socket, &operacion_a_realizar, sizeof(int), 0) == -1) {
        perror("Error al enviar la operacion_a_realizar");
        return;
    }

    // Enviamos el tiempo de sleep:
    int tiempo_sleep = *tiempo_sleep_ptr;
    if (send(socket, &tiempo_sleep, sizeof(int), 0) == -1) {
        perror("Error al enviar el mensaje");
        return;
    }

    // Liberamos la memoria de los argumentos:
    free(pid_proceso_ptr);
    free(operacion_a_realizar_ptr);
    free(tiempo_sleep_ptr);

    if (recv(socket, response, sizeof(int), 0) == -1) {
        perror("Error al recibir la respuesta");
        return;
    }
}

void send_message_to_std_interface(int socket, t_list *args, int *response) {
    int *pid_proceso_ptr = list_remove(args, 0);
    int *operacion_a_realizar_ptr = list_remove(args, 0);
    
    // Enviamos la cantidad de bytes a enviar:
    int cant_de_bytes = sizeof(int) * list_size(args) + sizeof(int) * 2;
    if(send(socket, &cant_de_bytes, sizeof(int), 0) == -1) {
        perror("Error al enviar la cantidad de bytes a enviar");
        return;
    }

    // Enviamos el pid del proceso:
    int pid_proceso = *pid_proceso_ptr;
    if (send(socket, &pid_proceso, sizeof(int), 0) == -1) {
        perror("Error al enviar el pid del proceso");
        return;
    }

    // Enviamos la operación a realizar:
    int operacion_a_realizar = *operacion_a_realizar_ptr;
    if (send(socket, &operacion_a_realizar, sizeof(int), 0) == -1) {
        perror("Error al enviar la operacion_a_realizar");
        return;
    }

    // Enviamos los argumentos:
    while(!list_is_empty(args)) {
        int *direccion_fisica = list_remove(args, 0);
        int *bytes_a_leer_o_escribir = list_remove(args, 0);

        // Enviamos la dirección física:
        if (send(socket, direccion_fisica, sizeof(int), 0) == -1) {
            perror("Error al enviar la dirección física");
            return;
        }

        // Enviamos los bytes a leer o escribir:
        if (send(socket, bytes_a_leer_o_escribir, sizeof(int), 0) == -1) {
            perror("Error al enviar los bytes a leer o escribir");
            return;
        }

        free(direccion_fisica);
        free(bytes_a_leer_o_escribir);
    }

    // Liberamos la memoria de los argumentos:
    free(pid_proceso_ptr);
    free(operacion_a_realizar_ptr);

    // Enviamos un -1 para indicar que terminamos de enviar las direcciones físicas y bytes a leer:
    if (recv(socket, response, sizeof(int), 0) == -1) {
        perror("Error al recibir la respuesta");
        return;
    }
}

void send_message_to_dialfs_interface(int socket, t_list *args, int *response) {

    // Obtenemos el operacion_a_realizar:
    int *operacion_a_realizar = list_get(args, 1);

    log_info(logger, "Operacion: %i", *operacion_a_realizar);

    // Enviamos el mensaje a la interfaz correspondiente:
    switch(*operacion_a_realizar) {
        case IO_FS_CREATE_INT:
        case IO_FS_DELETE_INT:
            send_message_to_dialfs_create_o_delete(socket, args, response);
            break;
        case IO_FS_READ_INT:
        case IO_FS_WRITE_INT:
            send_message_to_dialfs_read_o_write(socket, args, response);
            break;
        case IO_FS_TRUNCATE_INT:
            send_message_to_dialfs_truncate(socket, args, response);
            break;
        default:
            // Manejo de error o caso por defecto
            break;
    }
}

void send_message_to_dialfs_create_o_delete(int socket, t_list *args, int *response) {

    // Obtenemos los argumentos:
    int *pid_proceso_ptr = list_remove(args, 0);
    int *operacion_a_realizar_ptr = list_remove(args, 0);
    char *nombre_archivo = list_remove(args, 0);

    // Enviamos el paquete:
    t_paquete *paquete = crear_paquete(*operacion_a_realizar_ptr);
    agregar_a_paquete(paquete, pid_proceso_ptr, sizeof(int));
    agregar_a_paquete_string(paquete, nombre_archivo, strlen(nombre_archivo) + 1);
    enviar_paquete(paquete, socket);
    eliminar_paquete(paquete);

    // Liberamos la memoria de los argumentos:
    free(pid_proceso_ptr);
    free(operacion_a_realizar_ptr);
    free(nombre_archivo);

    // Recibimos la respuesta:
    if (recv(socket, response, sizeof(int), 0) == -1) {
        perror("Error al recibir la respuesta");
        return;
    }
}

void send_message_to_dialfs_read_o_write(int socket, t_list *args, int *response) {

    // Obtenemos los argumentos:
    int *pid_proceso_ptr = list_remove(args, 0);
    int *operacion_a_realizar_ptr =list_remove(args, 0);
    char *nombre_archivo = list_remove(args, 0);
    int *offset = list_remove(args, 0);

    // Enviamos el paquete:
    t_paquete *paquete = crear_paquete(*operacion_a_realizar_ptr);
    agregar_a_paquete(paquete, pid_proceso_ptr, sizeof(int));
    agregar_a_paquete_string(paquete, nombre_archivo, strlen(nombre_archivo) + 1);
    agregar_a_paquete(paquete, offset, sizeof(int));

    while(!list_is_empty(args)) {
        int *direccion_fisica = list_remove(args, 0);
        int *bytes_a_leer_o_escribir = list_remove(args, 0);

        log_info(logger, "Direccion fisica: %i", *direccion_fisica);
        log_info(logger, "Bytes a leer o escribir: %i", *bytes_a_leer_o_escribir);

        agregar_a_paquete(paquete, direccion_fisica, sizeof(int)); 
        agregar_a_paquete(paquete, bytes_a_leer_o_escribir, sizeof(int));

        free(direccion_fisica);
        free(bytes_a_leer_o_escribir);
    }

    enviar_paquete(paquete, socket);
    eliminar_paquete(paquete);

    // Liberamos la memoria de los argumentos:
    free(pid_proceso_ptr);
    free(operacion_a_realizar_ptr);
    free(offset);
    free(nombre_archivo);

    // Recibimos la respuesta:
    if (recv(socket, response, sizeof(int), 0) == -1) {
        perror("Error al recibir la respuesta");
        return;
    }
}

void send_message_to_dialfs_truncate(int socket, t_list *args, int *response) {

    // Obtenemos los argumentos:
    int *pid_proceso_ptr = list_remove(args, 0);
    int *operacion_a_realizar_ptr = list_remove(args, 0);
    char *nombre_archivo = list_remove(args, 0);
    int *tamanio = list_remove(args, 0);

    // Enviamos el paquete:
    t_paquete *paquete = crear_paquete(*operacion_a_realizar_ptr);
    agregar_a_paquete(paquete, pid_proceso_ptr, sizeof(int));
    agregar_a_paquete_string(paquete, nombre_archivo, strlen(nombre_archivo) + 1);
    agregar_a_paquete(paquete, tamanio, sizeof(int));
    enviar_paquete(paquete, socket);
    eliminar_paquete(paquete);

    // Liberamos la memoria de los argumentos:
    free(pid_proceso_ptr);
    free(operacion_a_realizar_ptr);
    free(nombre_archivo);
    free(tamanio);

    // Recibimos la respuesta:
    if (recv(socket, response, sizeof(int), 0) == -1) {
        perror("Error al recibir la respuesta");
        return;
    }
}

// 2. Funciones para recibir mensajes de interfaces:
t_list *recv_interfaz_y_argumentos(int socket, int pid_proceso) {
    int size;
    int desplazamiento = 0;

    // Creamos la lista de interfaz y argumentos:
    t_list *interfaz_y_argumentos = list_create();
    
    // Obtenemos el tipo de operación a realizar:
    int operacion_a_realizar = recibir_operacion(socket);
    int *operacion_a_realizar_ptr = malloc(sizeof(int));
    *operacion_a_realizar_ptr = operacion_a_realizar;
    list_add(interfaz_y_argumentos, operacion_a_realizar_ptr); 

    // Obtenemos el nombre de la interfaz y los argumentos:
    void *buffer = recibir_buffer(&size, socket);

    // Obtenemos el nombre de la interfaz:
    char *nombre_interfaz = parsear_string(buffer, &desplazamiento);
    list_add(interfaz_y_argumentos, nombre_interfaz);

    // Obtenemos los argumentos:
    t_list *argumentos = obtener_argumentos(buffer, &desplazamiento, size, operacion_a_realizar, pid_proceso);
    list_add(interfaz_y_argumentos, argumentos);

    // No liberamos `operacion_a_realizar` ni `nombre_interfaz` aquí porque ya están en la lista y se deben liberar al destruir la lista.
    free(buffer);
    
    return interfaz_y_argumentos;
}

// 3. Funciones auxiliares:

int parsear_int(void *buffer, int *desplazamiento) {
    int dato;

    memcpy(&dato, buffer + *desplazamiento, sizeof(int));
    *desplazamiento += sizeof(int);

    return dato;
}

char *parsear_string(void *buffer, int *desplazamiento) {   
    int tam;

    // Copiamos el tamaño del string:
    memcpy(&tam, buffer + *desplazamiento, sizeof(int));
    *desplazamiento += sizeof(int);

    // Copiamos el string:
    char *string = malloc(tam + 1);
    memcpy(string, buffer + *desplazamiento, tam);
    *desplazamiento += tam + 1;
    
    string[tam] = '\0';

    return string;
}

t_list *obtener_argumentos(void *buffer, int *desplazamiento, int size, int operacion_a_realizar, int pid_proceso) {

    // Creamos la lista de argumentos:
    t_list *argumentos = list_create();

    // Agregamos el pid del proceso a la lista de argumentos:
    int *pid_proceso_ptr = malloc(sizeof(int));
    *pid_proceso_ptr = pid_proceso;
    list_add(argumentos, pid_proceso_ptr);

    // Agregamos el operacion_a_realizar a la lista de argumentos:
    int *operacion_a_realizar_ptr = malloc(sizeof(int));
    *operacion_a_realizar_ptr = operacion_a_realizar;
    list_add(argumentos, operacion_a_realizar_ptr);

    switch(operacion_a_realizar) {
        case IO_GEN_SLEEP_INT:
            obtener_argumentos_generica(argumentos, buffer, desplazamiento);
            break;
        case IO_STDIN_READ_INT:
        case IO_STDOUT_WRITE_INT:
            obtener_argumentos_std(argumentos, buffer, desplazamiento, size);
            break;
        case IO_FS_CREATE_INT:
        case IO_FS_DELETE_INT:
            obtener_argumentos_dialfs_create_o_delete(argumentos, buffer, desplazamiento);
            break;
        case IO_FS_READ_INT:
        case IO_FS_WRITE_INT:
            obtener_argumentos_dialfs_read_o_write(argumentos, buffer, desplazamiento, size);
            break;
        case IO_FS_TRUNCATE_INT:
            obtener_argumentos_dialfs_truncate(argumentos, buffer, desplazamiento);
            break;
        default:
            // Manejo de error o caso por defecto
            break;
    }
    
    return argumentos;
}

void obtener_argumentos_generica(t_list *argumentos, void *buffer, int *desplazamiento) {
    int *tiempo_sleep = malloc(sizeof(int));
    *tiempo_sleep = parsear_int(buffer, desplazamiento);

    list_add(argumentos, tiempo_sleep);
}

void obtener_argumentos_std(t_list *argumentos, void *buffer, int *desplazamiento, int size) {

    // Obtenemos el desplazamiento inicial:
    int desplazamiento_inicial = *desplazamiento;

    // Obtenemos las direcciones físicas y los bytes a leer o escribir:
    while (desplazamiento_inicial < size) {
        int *direccion_fisica_ptr = malloc(sizeof(int));
        int *bytes_a_leer_ptr = malloc(sizeof(int));

        *direccion_fisica_ptr = parsear_int(buffer, &desplazamiento_inicial);
        *bytes_a_leer_ptr = parsear_int(buffer, &desplazamiento_inicial);

        list_add(argumentos, direccion_fisica_ptr);
        list_add(argumentos, bytes_a_leer_ptr);
    }

    *desplazamiento = desplazamiento_inicial;
}


void obtener_argumentos_dialfs_create_o_delete(t_list *argumentos, void *buffer, int *desplazamiento) {
    char *nombre_archivo = parsear_string(buffer, desplazamiento);
    list_add(argumentos, nombre_archivo);
}

void obtener_argumentos_dialfs_read_o_write(t_list *argumentos, void *buffer, int *desplazamiento, int size) {

    // Obtenemos el desplazamiento inicial:
    int desplazamiento_inicial = *desplazamiento;

    // Obtenemos el nombre del archivo:
    char *nombre_archivo = parsear_string(buffer, &desplazamiento_inicial);
    list_add(argumentos, nombre_archivo);

    // Obtenemos el offset:
    int *offset = malloc(sizeof(int));
    *offset = parsear_int(buffer, &desplazamiento_inicial);
    list_add(argumentos, offset);

    // Obtenemos la dirección física y los bytes a leer o escribir:
    while(desplazamiento_inicial < size) {
        int *direccion_fisica = malloc(sizeof(int));
        int *bytes_a_leer_o_escribir = malloc(sizeof(int));

        *direccion_fisica = parsear_int(buffer, &desplazamiento_inicial);
        *bytes_a_leer_o_escribir = parsear_int(buffer, &desplazamiento_inicial);

        log_info(logger, "Direccion fisica: %i", *direccion_fisica);
        log_info(logger, "Bytes a leer o escribir: %i", *bytes_a_leer_o_escribir);

        list_add(argumentos, direccion_fisica);
        list_add(argumentos, bytes_a_leer_o_escribir);
    }

    *desplazamiento = desplazamiento_inicial;
}

void obtener_argumentos_dialfs_truncate(t_list *argumentos, void *buffer, int *desplazamiento) {
    // Obtenemos el nombre del archivo:
    char *nombre_archivo = parsear_string(buffer, desplazamiento);
    list_add(argumentos, nombre_archivo);

    // Obtenemos el tamaño:
    int *tamanio = malloc(sizeof(int));
    *tamanio = parsear_int(buffer, desplazamiento);
    list_add(argumentos, tamanio);
}
