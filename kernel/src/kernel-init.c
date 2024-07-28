#include "kernel-init.h"

volatile sig_atomic_t terminate_program = 0;

void iniciar_modulo_kernel(int socket_servidor) {
    hilo_motivo_de_desalojo();
    inicializar_lista();
    aceptar_interfaces(socket_servidor);
    inicializacion_semaforos();
    iniciar_planificacion();
}

void inicializar_lista(){
    cola_new = list_create();
    cola_ready = list_create();
    cola_prima_VRR = list_create();
    cola_block = list_create();
    cola_exit = list_create();
    inicializar_cola_resource_block();
    inicializar_vector_recursos_pedidos();
}

void aceptar_interfaces(int socket_servidor) {
    pthread_t aceptar_interfaces_thread;

    pthread_create(&aceptar_interfaces_thread, NULL, (void *) handle_new_interface, (void *) socket_servidor);
    pthread_detach(aceptar_interfaces_thread);   
}

void iniciar_planificacion() {
    pthread_t planificacion_corto_plazo_thread;
    pthread_t planificacion_largo_plazo_thread;

    //Planificación a corto plazo y largo plazo:
    if (pthread_create(&planificacion_corto_plazo_thread, NULL, elegir_algoritmo_corto_plazo, NULL) != 0) {
        perror("Error al crear el hilo de planificación a corto plazo");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&planificacion_largo_plazo_thread, NULL, agregar_a_cola_ready, NULL) != 0) {
        perror("Error al crear el hilo de planificación a largo plazo");
        exit(EXIT_FAILURE);
    }
    
    //Esperamos a que terminen los hilos de planificación:
    pthread_detach(planificacion_corto_plazo_thread);
    pthread_detach(planificacion_largo_plazo_thread);
}


void finalizar_programa() {
    destruir_semaforos();
    prevent_from_memory_leaks();
    log_destroy(logger);
    log_destroy2(logger2);
    sem_destroy(&semaforo_interfaces);
    pthread_mutex_destroy(&reanudar_block);
    pthread_mutex_destroy(&reanudar_ds);
    pthread_mutex_destroy(&reanudar_largo);
    pthread_mutex_destroy(&reanudar_plani);
    liberar_config_kernel(config_kernel);
    config_destroy(config);
}


void liberar_config_kernel(t_config_kernel *config_kernel) {

    // Liberamos memoria de memoria:
    free(config_kernel->ALGORITMO_PLANIFICACION);
    free(config_kernel->IP_MEMORIA);
    free(config_kernel->PUERTO_MEMORIA);

    // Liberamos memoria de recursos:
    free(config_kernel->INST_RECURSOS);
    string_array_destroy(config_kernel->RECURSOS);
    free(config_kernel->PUERTO_ESCUCHA);

    // Liberamos memoria de CPU:
    free(config_kernel->IP_CPU);
    free(config_kernel->PUERTO_CPU_DS);
    free(config_kernel->PUERTO_CPU_IT);

    free(config_kernel);
}


void cerrar_programa(int signal) {
    if (signal == SIGINT) {
        terminate_program = 1;
        log_fede(logger2, "Cerrando programa...");
        
        destruir_semaforos();
        prevent_from_memory_leaks();
        liberar_config_kernel(config_kernel);
        liberar_interfaces();
        liberar_procesos(proceso_en_exec);
        log_destroy(logger);
        log_destroy2(logger2);
        
        sem_destroy(&semaforo_interfaces);
        pthread_mutex_destroy(&reanudar_block);
        pthread_mutex_destroy(&reanudar_ds);
        pthread_mutex_destroy(&reanudar_largo);
        pthread_mutex_destroy(&reanudar_plani);
        
        exit(0);
    }
}


void close_signal() {
    struct sigaction sa;
    sa.sa_handler = cerrar_programa;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error al configurar la senial de cierre");
        exit(1);
    }
}


void liberar_lista_de_argumentos(t_list *args) {
    if (args != NULL) {
        list_destroy_and_destroy_elements(args, free);
    }
}


void liberar_interface_io(interface_io *interface) {
    if (interface == NULL) {
        return;
    }

    // Liberar la cadena name
    if (interface->name != NULL) {
        free(interface->name);
    }

    // Liberar la cola process_blocked
    if (interface->process_blocked != NULL) {
        while (!queue_is_empty(interface->process_blocked)) {
            t_pcb *pcb = queue_pop(interface->process_blocked);
            liberar_procesos(pcb);
        }
        queue_destroy(interface->process_blocked);
    }

    // Liberar la cola args_process
    if (interface->args_process != NULL) {
        while (!queue_is_empty(interface->args_process)) {
            t_list *args = queue_pop(interface->args_process);
            liberar_lista_de_argumentos(args);
        }
        queue_destroy(interface->args_process);
    }

    // Destruir los semáforos
    sem_destroy(&interface->semaforo_used);
    sem_destroy(&interface->size_blocked);

    // Liberar la estructura interface_io
    free(interface);
}


void liberar_array(char** array) {
    if (!array) return;
    for (int i = 0; array[i] != NULL; i++) {
        free(array[i]);
    }
    free(array);
}