#include "kernel-interfaces.h"

sem_t semaforo_interfaces;

// Funciones de manejo de interfaz desde el lado del kernel:

void handle_new_interface(void *arg) {
    int socket_kernel = (int)arg;
    sem_init(&semaforo_interfaces, 0, 1);
    inicializar_diccionario_interfaces();

    while(1) {
        int socket_cliente_nueva_interfaz = esperar_cliente("KENEL", socket_kernel);

        if(socket_cliente_nueva_interfaz != -1) {
            pthread_t hilo_nueva_interfaz;

            pthread_create(&hilo_nueva_interfaz, NULL, (void *) manage_interface, (void *)socket_cliente_nueva_interfaz);
            pthread_detach(hilo_nueva_interfaz);
        }
    }
}

void manage_interface(void *socket_cliente) {
    int socket = (int) socket_cliente;
    int execute = 1;

    while(execute) {
        op_code operacion = recibir_operacion(socket);

        //log_info(logger, "Operacion recibida: %d", operacion);

        switch(operacion) {
            case HANDSHAKE:
                recibir_handshake(socket);
                break;
            case CREAR_INTERFAZ:
                create_interface(socket);
                execute = 0;
                break;
            default:
                log_error(logger, "Operacion invalida!");
                break;
        }
    }
}
// Funciones de consumidores:

void create_consumer_thread(char *interface_name) {
    pthread_t consumer_thread;

    // Crear un hilo consumidor y manejar cualquier error de creación
    if (pthread_create(&consumer_thread, NULL, (void*) consumers_pcbs_blockeds, interface_name) != 0) {
        perror("Error al crear el hilo consumidor");
        exit(EXIT_FAILURE);
    }
    pthread_detach(consumer_thread);
}

void consumers_pcbs_blockeds(void *args) {
    // Obtenemos la interfaz a la que se le asignará el hilo consumidor:
    char *interface_name = (char *) args;
    interface_io *interface = get_interface_from_dict(interface_name);

    while(1) {

        // Esperamos a que haya procesos en la cola de bloqueados, y que la interfaz no esté siendo usada:
        sem_wait(&interface->semaforo_used);
        sem_wait(&interface->size_blocked);

        // Initialize variables:
        int buffer;
        int response = 0;
        int socket_with_interface = get_socket_interface(interface);

        // Obtenemos el PCB y los argumentos del proceso bloqueado:
        pthread_mutex_lock(&interface->mutex_blocked);
        t_pcb *pcb = queue_pop(interface->process_blocked);
        t_list *args_pcb = queue_pop(interface->args_process);
        pthread_mutex_unlock(&interface->mutex_blocked);

        // Verificamos si la interfaz sigue conectada:
        int sigue_conectado = recv(socket_with_interface, &buffer, sizeof(int), MSG_PEEK | MSG_DONTWAIT);

        // Si la interfaz se desconecta, movemos el proceso a la cola de bloqueados:
        if (!sigue_conectado) {
            set_estado_de_conexion_interface(interface, 0);
            log_warning(logger, "Interfaz: %s - Desconectada", interface_name);

            sem_post(&interface->size_blocked);
            queue_push(interface->process_blocked, pcb);
            queue_push(interface->args_process, args_pcb);

            break;
        }

        // Enviamos el PCB y los argumentos a la interfaz:
        log_info(logger, "Proceso: %d - Enviando a interfaz: %s", pcb->pid, interface_name);
        send_message_to_interface(interface, args_pcb, &response, socket_with_interface);

        // Esperamos la respuesta de la interfaz:
        if (response) {
            log_warning(logger, "Proceso: %d - Terminado por: %s", pcb->pid, interface_name);
            mover_procesos_de_bloqueado_a_ready(pcb);
        }

        // Libereamos recursos, y habilitamos a la interfaz para que pueda seguir consumiendo:
        sem_post(&interface->semaforo_used);
    }

    // Limpiar recursos cuando se desconecta
    pthread_exit(NULL); // Finaliza el hilo limpiamente
}

// Funciones manipuladoras de interfaz:

interface_io *initialize_interface() {
    interface_io *interface = malloc(sizeof(interface_io));
    interface->socket_interface = -1;
    interface->process_blocked = queue_create();
    interface->args_process = queue_create();
    sem_init(&interface->semaforo_used, 0, 1);
    sem_init(&interface->size_blocked, 0, 0);
    pthread_mutex_init(&interface->mutex_blocked, NULL);
    interface->esta_conectado = 1;

    return interface;
}

void create_interface(int socket) {
    // Recibimos el buffer:
    int size;
    int desplazamiento = 0;
    void *buffer = recibir_buffer(&size, socket);

    // Parseamos el nombre de la interfaz y el tipo:
    char *name_interface = parsear_string(buffer, &desplazamiento);
    int tipo_interfaz = parsear_int(buffer, &desplazamiento);

    // Verificamos si la interfaz ya está conectada:
    if (ya_esta_conectada_interface(name_interface)) {
        interface_io *interface_existente = get_interface_from_dict(name_interface);

        if(!estado_de_conexion_interface(interface_existente)) {
            set_socket_interface(interface_existente, socket);
            set_estado_de_conexion_interface(interface_existente, 1);
            create_consumer_thread(name_interface);
            log_info(logger, "Interfaz: %s - Reconectada", name_interface);
        } else {
            log_error(logger, "Interfaz: %s - Ya está conectada", name_interface);
            close(socket);
        }

        // Liberamos recursos:
        free(buffer);
        free(name_interface);

        return;
    }   

    // Creamos la interfaz:
    interface_io *interface = initialize_interface();

    // Seteamos el nombre y el socket de la interfaz:
    set_name_interface(interface, name_interface);
    set_socket_interface(interface, socket);
    set_tipo_interfaz(interface, tipo_interfaz);
    set_estado_de_conexion_interface(interface, 1);

    // Logeamos la creación de la interfaz:
    log_info(logger, "Interfaz: %s - Creada", name_interface);

    // Agregamos la interfaz al diccionario:
    sem_wait(&semaforo_interfaces);
    add_interface_to_dict(interface, name_interface);
    sem_post(&semaforo_interfaces);

    // Liberamos recursos:
    free(buffer);
    
    // Creamos el hilo consumidor:
    create_consumer_thread(name_interface);
}