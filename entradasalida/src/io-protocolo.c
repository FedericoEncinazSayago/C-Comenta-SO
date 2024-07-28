
#include "io-protocolo.h"

// Funciones enviar mensajes a kernel:

void send_respuesta_a_kernel(int respuesta, t_interfaz * interfaz) {
    int socket_kernel = get_socket_kernel(interfaz);
    send(socket_kernel, &respuesta, sizeof(int), 0);

    return;
}

void send_interfaz_a_kernel(t_interfaz * interfaz) {
    t_paquete *paquete = crear_paquete(CREAR_INTERFAZ);
    int socket_cliente = get_socket_kernel(interfaz);

    agregar_a_paquete_string(paquete, interfaz->nombre, strlen(interfaz->nombre) + 1);
    agregar_a_paquete(paquete, &interfaz->tipo, sizeof(tipo_interfaz));
    enviar_paquete(paquete, socket_cliente);

    eliminar_paquete(paquete);
}

// Funciones recibir mensajes de kernel:

t_list *recibir_argumentos(t_interfaz *interfaz, int socket_kernel) {
    int size;
    int desplazamiento = 0;
    void *buffer = recibir_buffer(&size, socket_kernel);

    // Parseamos el buffer para obtener los argumentos:
    t_list *argumentos = list_create();

    // Obtenemos el PID del proceso:
    int *pid = malloc(sizeof(int));
    *pid = parsear_int(buffer, &desplazamiento);
    list_add(argumentos, pid);

    // Obtenemos el tipo de operación:
    int *tipo_operacion = malloc(sizeof(int));
    *tipo_operacion = parsear_int(buffer, &desplazamiento);
    list_add(argumentos, tipo_operacion);

    tipo_interfaz tipo_interfaz_actual = get_tipo_interfaz_to_int(interfaz);

    if(tipo_interfaz_actual == GENERICA) {
        int *tiempo_sleep = malloc(sizeof(int));
        *tiempo_sleep = parsear_int(buffer, &desplazamiento);

        list_add(argumentos, tiempo_sleep);
    } else {
        t_list *direcciones = obtener_direcciones_fisicas(size, &desplazamiento, buffer);
        list_add(argumentos, direcciones);
    }

    //Liberamos el buffer recibido:
    free(buffer);

    log_info(logger, "Se recibieron los argumentos");

    return argumentos;
}

t_list *recibir_argumentos_para_dial(t_interfaz * interfaz, tipo_operacion tipo) {
    int size;
    int desplazamiento = 0;
    int socket_kernel = get_socket_kernel(interfaz);

    t_list *argumentos = list_create();

    // Recibimos el buffer:
    void *buffer = recibir_buffer(&size, socket_kernel);

    // Obtenemos el PID del proceso:
    int *pid = malloc(sizeof(int));
    *pid = parsear_int(buffer, &desplazamiento);
    list_add(argumentos, pid);

    // Parseamos el buffer para obtener los argumentos:
    char *file = parsear_string(buffer, &desplazamiento);
    list_add(argumentos, file); // Agregamos el archivo a la lista de argumentos

    if(tipo == IO_FS_WRITE_INT || tipo == IO_FS_READ_INT || tipo == IO_FS_TRUNCATE_INT) {
        int offset = parsear_int(buffer, &desplazamiento);
        int *offset_puntero = malloc(sizeof(int));
        *offset_puntero = offset;
        list_add(argumentos, offset_puntero); // Agregamos el offset a la lista de argumentos
    }

    if (tipo == IO_FS_WRITE_INT || tipo == IO_FS_READ_INT) {
        t_list *direcciones = obtener_direcciones_fisicas(size, &desplazamiento, buffer);
        list_add(argumentos, direcciones); // Agregamos las direcciones físicas a la lista de argumentos
    }

    // Liberamos el buffer recibido:
    free(buffer);

    return argumentos;
}

t_list *obtener_direcciones_fisicas(int size, int *desplazamiento, void *buffer) {

    // Creamos la lista de direcciones físicas:
    t_list *direccciones_fisicas = list_create();

    int direccion_fisica;
    int tamanio;

    while(*desplazamiento < size) {
        // Obtenemos la dirección física:
        memcpy(&direccion_fisica, buffer + *desplazamiento, sizeof(int));
        *desplazamiento += sizeof(int);

        // Obtenemos el tamaño:
        memcpy(&tamanio, buffer + *desplazamiento, sizeof(int));
        *desplazamiento += sizeof(int);

        // Creamos la dirección física:
        t_direccion_fisica *direccion = malloc(sizeof(t_direccion_fisica));
        direccion->direccion_fisica = direccion_fisica;
        direccion->tamanio = tamanio;

        // Agregamos la dirección física a la lista:
        list_add(direccciones_fisicas, direccion);
    }

    return direccciones_fisicas;
}

// Funciones enviar mensajes a memoria:

void send_bytes_a_grabar(t_interfaz * interfaz, int direccion_fisica, char *bytes, int bytes_a_leer) {
    int socket_memoria = get_socket_memory(interfaz);
    t_paquete *paquete = crear_paquete(ESCRIBIR_MEMORIA);

    agregar_a_paquete(paquete, &direccion_fisica, sizeof(int));
    agregar_a_paquete(paquete, &bytes_a_leer, sizeof(int));
    agregar_a_paquete_string(paquete, bytes, bytes_a_leer);

    enviar_paquete(paquete, socket_memoria);
}

void send_mensaje_a_memoria(t_interfaz * interfaz, char *mensaje) {
    int socket_memoria = get_socket_memory(interfaz);
    t_paquete *paquete = crear_paquete(MENSAJE);

    agregar_a_paquete_string(paquete, mensaje, strlen(mensaje) + 1);

    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);
}

void send_bytes_a_leer(t_interfaz *interfaz, int pid, t_list *direcciones, void *input, int bytes_leidos) {

    // Obtenemos socket con memoria:
    int socket_memoria = get_socket_memory(interfaz);

    // Inicializamos las variables:
    int bytes_mandados = 0;
    int respuesta;

    // Enviamos el input a memoria:
    while(bytes_mandados < bytes_leidos) {
        t_direccion_fisica *direccion = list_remove(direcciones, 0);
        int direccion_fisica = direccion->direccion_fisica;
        int tamanio = direccion->tamanio;

        // Enviamos el input a memoria:
        t_paquete *paquete = crear_paquete(ACCESO_A_ESCRITURA);
        agregar_a_paquete(paquete, &pid, sizeof(int));
        agregar_a_paquete(paquete, &direccion_fisica, sizeof(int));
        agregar_a_paquete(paquete, &tamanio, sizeof(int));

        if(bytes_mandados + tamanio > bytes_leidos) {
            tamanio = bytes_leidos - bytes_mandados;
        }

        // Creamos el buffer a enviar:
        char *buffer = malloc(tamanio + 1);
        buffer[tamanio] = '\0';
        memcpy(buffer, input + bytes_mandados, tamanio);

        // Logueamos el buffer:
        agregar_a_paquete(paquete, buffer, tamanio);
        enviar_paquete(paquete, socket_memoria);
        eliminar_paquete(paquete);

        // Esperamos la respuesta de memoria:
        respuesta = recibir_entero(socket_memoria);

        if(respuesta == -1) {
            log_error(logger, "Error al escribir en memoria");
            exit(EXIT_FAILURE);
        }
        
        log_info(logger, "Se manda el buffer: %s", buffer);

        // Liberamos la memoria usada para el buffer y el paquete:
        free(buffer);
        free(direccion);

        // Actualizamos las variables:
        bytes_mandados += tamanio;
    }

    // Logueamos la cantidad de bytes escritos:
    log_info(logger, "Se escribieron %d bytes en memoria", bytes_leidos);

    // Liberamos la memoria usada para las direcciones físicas:
    list_destroy(direcciones);
}

// Funciones recibir mensajes de memoria:

char *rcv_contenido_a_mostrar(t_interfaz *interfaz, t_list *direcciones_fisicas, int pid_proceso) {

    // Inicializamos las variables:
    int size = list_size(direcciones_fisicas);
    int socket_memoria = get_socket_memory(interfaz);
    int cantidad_bytes = get_total_de_bytes(direcciones_fisicas);
    int index = 0;

    // Creamos el buffer a mostrar:
    char *contenido_a_mostrar = malloc(cantidad_bytes + 1);
    int desplazamiento_interno = 0;

    while(index < size) {
        t_direccion_fisica *direccion = list_remove(direcciones_fisicas, 0);
        int direccion_fisica = direccion->direccion_fisica;
        int tamanio = direccion->tamanio;

        // Enviamos la dirección física a memoria:
        t_paquete *paquete = crear_paquete(ACCESO_A_LECTURA);
        agregar_a_paquete(paquete, &pid_proceso, sizeof(int));
        agregar_a_paquete(paquete, &direccion_fisica, sizeof(int));
        agregar_a_paquete(paquete, &tamanio, sizeof(int));

        // Asegúrate de enviar el paquete a la memoria usando socket_memoria
        enviar_paquete(paquete, socket_memoria);

        // Liberar la memoria usada para el paquete
        eliminar_paquete(paquete);

        // Recibimos la respuesta de memoria:
        int respuesta = recibir_entero(socket_memoria);

        if(respuesta == -1) {
            log_error(logger, "Error al leer en memoria");
            exit(EXIT_FAILURE);
        }

        // Recibimos el buffer de memoria:
        int size;
        void *buffer = recibir_buffer(&size, socket_memoria);

        // Copiamos el contenido al buffer a mostrar:
        memcpy(contenido_a_mostrar + desplazamiento_interno, buffer, tamanio);
        desplazamiento_interno += tamanio;

        // Liberamos la memoria usada para el contenido y el buffer:
        free(buffer);
        free(direccion);

        // Actualizamos el índice:
        index++;
    }

    // Liberamos la memoria usada para las direcciones físicas:
    list_destroy(direcciones_fisicas);

    return contenido_a_mostrar;
}


// Funciones auxiliares:

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

int parsear_int(void *buffer, int *desplazamiento) {
    int dato;

    // Copiamos el dato:
    memcpy(&dato, buffer + *desplazamiento, sizeof(int));
    *desplazamiento += sizeof(int);

    return dato;
}

int recibir_entero(int socket) {
    int dato;
    recv(socket, &dato, sizeof(int), 0);

    return dato;
}