#include "planificacion.h"

pthread_mutex_t mutex_estado_new;
pthread_mutex_t mutex_estado_block;
pthread_mutex_t mutex_estado_ready;
pthread_mutex_t mutex_estado_exec;
pthread_mutex_t mutex_cola_priori_vrr;
pthread_mutex_t mutex_proceso_exec;
pthread_mutex_t mutex_cola_block;
pthread_mutex_t mutex_exit;
sem_t sem_multiprogramacion;
sem_t habilitar_corto_plazo;
sem_t hay_en_estado_ready;
sem_t hay_en_estado_new;
sem_t desalojo_proceso;
sem_t hay_proceso_exec;
sem_t hay_proceso_en_bloq;
sem_t sem_vrr;
sem_t sem_proceso_create;

int pid = 0;
int pid_buscado_temporal = 0;
int quantum_restante;
int esta_finalizado;

t_pcb* proceso_en_exec = NULL;

t_list* cola_new;
t_list* cola_ready; 
t_list* cola_prima_VRR;
t_list* cola_block;
t_list* cola_exit;
t_recursos_pedidos* vector_recursos_pedidos;
int tam_vector_recursos_pedidos;


void inicializacion_semaforos() {
    pthread_mutex_init(&mutex_estado_ready, NULL);
    pthread_mutex_init(&mutex_estado_new, NULL);
    pthread_mutex_init(&mutex_estado_exec, NULL);
    pthread_mutex_init(&mutex_estado_block, NULL);
    pthread_mutex_init(&mutex_cola_priori_vrr, NULL);
    pthread_mutex_init(&mutex_proceso_exec, NULL);
    pthread_mutex_init(&mutex_cola_block, NULL);
    pthread_mutex_init(&mutex_exit, NULL);
    sem_init(&habilitar_corto_plazo, 0, 0);
    sem_init(&hay_en_estado_ready, 0, 0);
    sem_init(&hay_en_estado_new, 0, 0);
    sem_init(&sem_multiprogramacion, 0, config_kernel->GRADO_MULTIP);
    sem_init(&desalojo_proceso, 0, 0);
    sem_init(&hay_proceso_exec, 0, 1);
    sem_init(&hay_proceso_en_bloq, 0, 0);
    sem_init(&sem_vrr, 0, 0);
    sem_init(&sem_proceso_create, 0, 1);
}


int generar_pid_unico() {
    return pid++;
}


void informar_a_memoria_creacion_proceso(char* archivo_de_proceso, int pid) {
    t_paquete* paquete = crear_paquete(INICIAR_PROCESO);

    agregar_a_paquete(paquete, &pid, sizeof(int));
    agregar_a_paquete_string(paquete, archivo_de_proceso, strlen(archivo_de_proceso) + 1);

    enviar_paquete(paquete, config_kernel->SOCKET_MEMORIA);
    eliminar_paquete(paquete);
}


void creacion_proceso(char *archivo_de_proceso) {
    t_pcb* pcb = malloc(sizeof(t_pcb)); 
    
    if (!pcb) {
        log_error(logger, "Error al asignar memoria a la creación de PCB");
        return;
    }

    pcb->pid = generar_pid_unico();
    pcb->registros = malloc(sizeof(t_registro_cpu));
    if (!pcb->registros) {
        log_error(logger, "Error al asignar memoria a los registros del proceso");
        free(pcb);
        return;
    }

    pcb->quantum = config_kernel->QUANTUM;
    pcb->estado = NEW;
    pcb->registros->PC = 0;
    pcb->registros->AX = 0;
    pcb->registros->BX = 0;
    pcb->registros->CX = 0;
    pcb->registros->DX = 0;
    pcb->registros->EAX = 0;
    pcb->registros->EBX = 0;
    pcb->registros->ECX = 0;
    pcb->registros->EDX = 0;
    pcb->registros->SI = 0;
    pcb->registros->DI = 0;

    log_info(logger, "Se crea el proceso %i en NEW \n", pcb->pid);
    informar_a_memoria_creacion_proceso(archivo_de_proceso, pcb->pid);

    int response;
    int socket_memoria = config_kernel->SOCKET_MEMORIA;

    recv(socket_memoria, &response, sizeof(int), MSG_WAITALL);

    if (response == 1) {
        agregar_a_cola_estado_new(pcb);
    } else {
        log_error(logger, "Error al intentar agregar un nuevo proceso.");
        liberar_procesos(pcb); // Asegura que se libere la memoria en caso de error
    }
}


void agregar_a_cola_estado_new(t_pcb* proceso) { //NEW
    pthread_mutex_lock(&mutex_estado_new);
    list_add(cola_new, proceso);
    pthread_mutex_unlock(&mutex_estado_new);
    
    sem_post(&hay_en_estado_new);
}


void* agregar_a_cola_ready() {

    while (1) {
        pthread_mutex_lock(&reanudar_largo);
        pthread_mutex_unlock(&reanudar_largo);

        sem_wait(&hay_en_estado_new);
        sem_wait(&sem_multiprogramacion);

        t_pcb* proceso = obtener_siguiente_a_ready();

        pthread_mutex_lock(&mutex_estado_ready);
        list_add(cola_ready, proceso);
        pthread_mutex_unlock(&mutex_estado_ready);

        mostrar_lista_de_pids_ready(cola_ready);
        
        sem_post(&habilitar_corto_plazo);
        sem_post(&hay_en_estado_ready);

    }
    return NULL;
}


void mover_procesos_de_bloqueado_a_ready(t_pcb* proceso) {

    sem_wait(&hay_proceso_en_bloq);
    pthread_mutex_lock(&reanudar_block);
    pthread_mutex_unlock(&reanudar_block);

    pid_buscado_temporal = proceso->pid;

    pthread_mutex_lock(&mutex_cola_block);
    t_pcb* pcb_a_ready = pcb_encontrado(cola_block, pid_buscado_temporal);
    if(pcb_a_ready) {
        list_remove_element(cola_block, pcb_a_ready);
    }
    pthread_mutex_unlock(&mutex_cola_block);
    
    if (pcb_a_ready) {
        if (strcmp(config_kernel->ALGORITMO_PLANIFICACION, "VRR") == 0) {

            if(pcb_a_ready->quantum <= 0) {
                pthread_mutex_lock(&mutex_estado_ready);
                pcb_a_ready->estado = READY;
                log_info(logger, "PID: %i - Estado Anterior: BLOCK - Estado Actual: READY \n", pcb_a_ready->pid);
                log_facu(logger2, "Quantum Restante: %i", pcb_a_ready->quantum);
                list_add(cola_ready, pcb_a_ready);
                mostrar_lista_de_pids_ready(cola_ready);
                pthread_mutex_unlock(&mutex_estado_ready);
                sem_post(&hay_en_estado_ready);
            }
            else {
                pthread_mutex_lock(&mutex_cola_priori_vrr);
                pcb_a_ready->estado = READY;
                log_info(logger, "PID: %i - Estado Anterior: BLOCK - Estado Actual: READY \n", pcb_a_ready->pid);
                log_facu(logger2, "Quantum Restante: %i", pcb_a_ready->quantum);
                list_add(cola_prima_VRR, pcb_a_ready);
                mostrar_lista_de_pids_prima(cola_prima_VRR);
                pthread_mutex_unlock(&mutex_cola_priori_vrr);
                sem_post(&hay_en_estado_ready);
            }

        } else {
            pthread_mutex_lock(&mutex_estado_ready);
            log_info(logger, "PID: %i - Estado Anterior: BLOCK - Estado Actual: READY \n", pcb_a_ready->pid);
            pcb_a_ready->estado = READY;
            list_add(cola_ready, pcb_a_ready);
            mostrar_lista_de_pids_ready(cola_ready);
            pthread_mutex_unlock(&mutex_estado_ready);
            sem_post(&hay_en_estado_ready);
        }
        
    }
}


t_pcb* obtener_siguiente_a_ready() {
    pthread_mutex_lock(&mutex_estado_new);
    t_pcb* pcb = list_remove(cola_new, 0);
    pcb->estado = READY;
    log_info(logger, "PID: %i - Estado Anterior: NEW - Estado Actual: READY\n", pcb->pid);
    pthread_mutex_unlock(&mutex_estado_new);
    return pcb;
}


void informar_a_memoria_liberacion_proceso(int pid) {
    t_paquete* paquete = crear_paquete(FINALIZAR_PROCESO);
    agregar_a_paquete(paquete, &pid, sizeof(int));
    enviar_paquete(paquete, config_kernel->SOCKET_MEMORIA);
    eliminar_paquete(paquete); 
    //NO SE SI DEBERIA ESPERAR A QUE MEMORIA ME DE EL OK PARA BORRAR EL PROCESO
}


void* elegir_algoritmo_corto_plazo() {
    if(terminate_program != 1) {
        if(strcmp(config_kernel->ALGORITMO_PLANIFICACION, "FIFO") == 0) {
            hilo_planificador_cortoplazo_fifo();
        } else if(strcmp(config_kernel->ALGORITMO_PLANIFICACION, "RR") == 0) {
            hilo_planificador_cortoplazo_RoundRobin();
        } else if(strcmp(config_kernel->ALGORITMO_PLANIFICACION, "VRR") == 0) {
            hilo_planificador_cortoplazo_VRR();
        } else {
            log_error(logger, "El algoritmo no coincide con los algoritmos" );
        }
    }
    else
        log_info(logger, "Cerrando Hilos ...");
    return NULL;
}


void hilo_planificador_largoplazo() {
    if(terminate_program != 1) {
        pthread_t hilo_p_largo;
        pthread_create(&hilo_p_largo, NULL, agregar_a_cola_ready, NULL);
        pthread_detach(hilo_p_largo); // Detach para evitar necesidad de join, hace q los hilos sean independientes y libera los recursos una vez terminada la ejecucion del mismo
    }
}


void hilo_planificador_cortoplazo_fifo() {
    if(terminate_program != 1) {
        pthread_t hilo_fifo;
        pthread_create(&hilo_fifo, NULL, planificador_cortoplazo_fifo, NULL);
        pthread_detach(hilo_fifo); // Detach para evitar necesidad de join, hace q los hilos sean independientes y libera los recursos una vez terminada la ejecucion del mismo
    }
}


void hilo_planificador_cortoplazo_RoundRobin() {
    if(terminate_program != 1) {
        pthread_t hilo_RoundRobin;
        pthread_create(&hilo_RoundRobin, NULL, planificador_corto_plazo_RoundRobin, NULL);
        pthread_detach(hilo_RoundRobin);
    }
}


void hilo_planificador_cortoplazo_VRR() {
    if(terminate_program != 1) {
        pthread_t hilo_VRR;
        pthread_create(&hilo_VRR, NULL, planificacion_cortoplazo_VRR, NULL);
        pthread_detach(hilo_VRR);
    }
}


void* planificador_cortoplazo_fifo(void* arg) {
    sem_wait(&habilitar_corto_plazo);
    while (1) {
        pthread_mutex_lock(&reanudar_plani);
        pthread_mutex_unlock(&reanudar_plani);

        sem_wait(&hay_en_estado_ready);
        sem_wait(&hay_proceso_exec);

        pthread_mutex_lock(&mutex_estado_ready);
        pthread_mutex_lock(&mutex_proceso_exec);
        proceso_en_exec = list_remove(cola_ready, 0);
        proceso_en_exec->estado = EXEC;
        pthread_mutex_unlock(&mutex_proceso_exec);
        log_info(logger, "PID: %i - Estado Anterior: READY - Estado Actual: EXEC", proceso_en_exec->pid);
        log_info(logger, "Enviando proceso %i a CPU\n", proceso_en_exec->pid);
        pthread_mutex_unlock(&mutex_estado_ready);

        enviar_proceso_a_cpu(proceso_en_exec);
        sem_wait(&desalojo_proceso);
        if(esta_finalizado == 1) {
            sem_wait(&hay_proceso_exec);
            esta_finalizado = 0;
        }
    }
    return NULL;
}


void puede_ejecutar_otro_proceso() {
    sem_post(&hay_proceso_exec);
}


void* planificador_corto_plazo_RoundRobin(void* arg) {
    sem_wait(&habilitar_corto_plazo);

    while (1) {
        pthread_mutex_lock(&reanudar_plani);
        pthread_mutex_unlock(&reanudar_plani);

        sem_wait(&hay_en_estado_ready);  // Espera hasta que haya procesos en READY
        sem_wait(&hay_proceso_exec);
        
        pthread_mutex_lock(&mutex_proceso_exec);
        proceso_en_exec = NULL;
        pthread_mutex_lock(&mutex_estado_ready);
        proceso_en_exec = list_remove(cola_ready, 0); 
        pthread_mutex_unlock(&mutex_estado_ready);
        pthread_mutex_unlock(&mutex_proceso_exec);
        
        if (proceso_en_exec) {

            pthread_mutex_lock(&mutex_proceso_exec);
            proceso_en_exec->estado = EXEC;
            proceso_en_exec->quantum = config_kernel->QUANTUM;
            pthread_mutex_unlock(&mutex_proceso_exec);

            enviar_proceso_a_cpu(proceso_en_exec);
            log_info(logger, "PID: %i - Estado Anterior: READY - Estado Actual: EXEC", proceso_en_exec->pid);
            log_info(logger, "Enviando proceso %i a CPU", proceso_en_exec->pid);

            pthread_t hilo_quantum; // Crear un hilo para manejar el quantum
            log_info(logger, "En ejecucion %i milisegundos ...", config_kernel->QUANTUM);
            if (pthread_create(&hilo_quantum, NULL, quantum_handler, (void*)proceso_en_exec) != 0) { //mandarle el contador global
                log_error(logger, "Error al crear el hilo del quantum para el proceso %d", proceso_en_exec->pid);
            }

            sem_wait(&desalojo_proceso); // Signeado por hilo del desalojo
            log_info(logger, "¡Se rompe hilo de Sleep!");
            pthread_cancel(hilo_quantum);
            pthread_join(hilo_quantum, NULL);  // Esperar a que el hilo del quantum termine y liberar sus recursos

            if(esta_finalizado == 1) {
                sem_wait(&hay_proceso_exec);
                esta_finalizado = 0;
            }
        } else {
            sem_post(&habilitar_corto_plazo);  // Rehabilitar el planificador si no se encontró un proceso
        }
    }
    return NULL;
}


void* planificacion_cortoplazo_VRR() {
    sem_wait(&habilitar_corto_plazo);

    while (1) {
        pthread_mutex_lock(&reanudar_plani);
        pthread_mutex_unlock(&reanudar_plani);
                                        
        sem_wait(&hay_en_estado_ready);
        sem_wait(&hay_proceso_exec);

        pthread_mutex_lock(&mutex_proceso_exec);
        proceso_en_exec = NULL;

        if (!list_is_empty(cola_prima_VRR)) {
            pthread_mutex_lock(&mutex_cola_priori_vrr);
            if (!list_is_empty(cola_prima_VRR)) {
                proceso_en_exec = list_remove(cola_prima_VRR, 0);
            }
            pthread_mutex_unlock(&mutex_cola_priori_vrr);
            
            if (proceso_en_exec) {
                log_info(logger, "¡Se encontró un proceso en Cola Prima!");
            } else {
                log_error(logger, "Error al obtener un proceso de la Cola Prima");
            }
            
        } else if (!list_is_empty(cola_ready)) {
            pthread_mutex_lock(&mutex_estado_ready);
            if (!list_is_empty(cola_ready)) {
                proceso_en_exec = list_remove(cola_ready, 0);
            }
            pthread_mutex_unlock(&mutex_estado_ready);
            
            if (proceso_en_exec) {
                log_info(logger, "¡Se encontró un proceso en Cola Normal!");
            } else {
                log_error(logger, "Error al obtener un proceso de la Cola Normal");
            }

        } else {
            log_error(logger, "No hay ningún proceso en la cola READY o PRIMA");
        }

        pthread_mutex_unlock(&mutex_proceso_exec);   
    
        pthread_mutex_lock(&mutex_proceso_exec);
        proceso_en_exec->estado = EXEC;
        log_info(logger, "PID: %i - Estado Anterior: READY - Estado Actual: EXEC", proceso_en_exec->pid);
        pthread_mutex_unlock(&mutex_proceso_exec);

        pthread_mutex_lock(&mutex_proceso_exec);
        if (proceso_en_exec->quantum <= 0) {
            proceso_en_exec->quantum = config_kernel->QUANTUM;
        }
        pthread_mutex_unlock(&mutex_proceso_exec);
        enviar_proceso_a_cpu(proceso_en_exec);

        t_temporal* tiempo_de_ejecucion = temporal_create();
        int64_t tiempo_inicial_de_exec = temporal_gettime(tiempo_de_ejecucion);
              
        pthread_t hilo_quantum; // Crear un hilo para manejar el quantum
        log_info(logger, "En ejecucion %i milisegundos ...", proceso_en_exec->quantum);
        if (pthread_create(&hilo_quantum, NULL, quantum_handler, (void*)proceso_en_exec) != 0) { //mandarle el contador global
            log_error(logger, "Error al crear el hilo del quantum para el proceso %d", proceso_en_exec->pid);
        }

        sem_wait(&desalojo_proceso);
        pthread_cancel(hilo_quantum);
        pthread_join(hilo_quantum, NULL);

        int64_t tiempo_ejecutado = temporal_gettime(tiempo_de_ejecucion) - tiempo_inicial_de_exec;
        temporal_destroy(tiempo_de_ejecucion);

        quantum_restante = proceso_en_exec->quantum - tiempo_ejecutado;

        if (esta_block == 1) {
            sem_post(&sem_vrr);
            esta_block = 0;
        }
    }
    return NULL;
}


void* quantum_handler(void* arg) {
    t_pcb* proceso = (t_pcb*)arg;
    usleep(proceso->quantum * 1000); // Dormir por el tiempo del quantum
    int codigo = FINQUANTUM;
    send(config_kernel->SOCKET_INTERRUPT, &codigo, sizeof(int), MSG_WAITALL); // Enviar interrupción al final del quantum
    return NULL;
}


void enviar_proceso_a_cpu(t_pcb* pcbproceso) {
    t_paquete* paquete_cpu = crear_paquete(RECIBIR_PROCESO); // Tipo de paquete que indica envío a CPU

    // Agregar información del PCB al paquete
    agregar_a_paquete(paquete_cpu, &pcbproceso->pid, sizeof(int));

    // Agregar los registros de la CPU al paquete individualmente
    agregar_a_paquete(paquete_cpu, &pcbproceso->registros->PC, sizeof(uint32_t));
    agregar_a_paquete(paquete_cpu, &pcbproceso->registros->AX, sizeof(uint8_t));
    agregar_a_paquete(paquete_cpu, &pcbproceso->registros->BX, sizeof(uint8_t));
    agregar_a_paquete(paquete_cpu, &pcbproceso->registros->CX, sizeof(uint8_t));
    agregar_a_paquete(paquete_cpu, &pcbproceso->registros->DX, sizeof(uint8_t));
    agregar_a_paquete(paquete_cpu, &pcbproceso->registros->EAX, sizeof(uint32_t));
    agregar_a_paquete(paquete_cpu, &pcbproceso->registros->EBX, sizeof(uint32_t));
    agregar_a_paquete(paquete_cpu, &pcbproceso->registros->ECX, sizeof(uint32_t));
    agregar_a_paquete(paquete_cpu, &pcbproceso->registros->EDX, sizeof(uint32_t));
    agregar_a_paquete(paquete_cpu, &pcbproceso->registros->SI, sizeof(uint32_t));
    agregar_a_paquete(paquete_cpu, &pcbproceso->registros->DI, sizeof(uint32_t));

    // Enviar el paquete a la CPU
    enviar_paquete(paquete_cpu, config_kernel->SOCKET_DISPATCH); //SOCKET MAL
   // log_info(logger,"%i", config_kernel->SOCKET_DISPATCH);

    // Liberar recursos del paquete
    eliminar_paquete(paquete_cpu);
}


void prevent_from_memory_leaks() {

    if (cola_prima_VRR) {
        list_destroy_and_destroy_elements(cola_prima_VRR, (void*)liberar_procesos); // Usar liberar_pcb en lugar de free
        cola_prima_VRR = NULL;
    }

    if (cola_block) {
        list_destroy_and_destroy_elements(cola_block, (void*)liberar_procesos); // Usar liberar_pcb en lugar de free
        cola_block = NULL;
    }

    if (cola_new) {
        list_destroy_and_destroy_elements(cola_new, (void*)liberar_procesos); // Usar liberar_pcb en lugar de free
        cola_new = NULL;
    }

    if (cola_ready) {
        list_destroy_and_destroy_elements(cola_ready, (void*)liberar_procesos); // Usar liberar_pcb en lugar de free
        cola_ready = NULL;
    }

    if (cola_exit) {
        list_destroy_and_destroy_elements(cola_exit, (void*)liberar_procesos); // Usar liberar_pcb en lugar de free
        cola_exit = NULL;
    }

    if (colas_resource_block) {
        for (int i = 0; i < string_array_size(config_kernel->RECURSOS); i++) {
            if (colas_resource_block[i]) {
                list_destroy_and_destroy_elements(colas_resource_block[i], (void*)liberar_procesos); // Usar liberar_pcb en lugar de free
                colas_resource_block[i] = NULL;
            }
        }
        free(colas_resource_block);
        colas_resource_block = NULL;
    }

    liberar_vector_recursos_pedidos();
}

void liberar_vector_recursos_pedidos() {
    for (int i = 0; i < tam_vector_recursos_pedidos; i++) {
        if (vector_recursos_pedidos[i].recurso != NULL) {
            free(vector_recursos_pedidos[i].recurso);
            vector_recursos_pedidos[i].recurso = NULL;
        }
    }
    free(vector_recursos_pedidos);
    vector_recursos_pedidos = NULL;
}



void destruir_semaforos() {
    pthread_mutex_destroy(&mutex_estado_ready);
    pthread_mutex_destroy(&mutex_estado_new);
    pthread_mutex_destroy(&mutex_estado_exec);
    pthread_mutex_destroy(&mutex_estado_block);
    pthread_mutex_destroy(&mutex_cola_priori_vrr);
    pthread_mutex_destroy(&mutex_proceso_exec);
    pthread_mutex_destroy(&mutex_cola_block);
    pthread_mutex_destroy(&mutex_exit);
    sem_destroy(&habilitar_corto_plazo);
    sem_destroy(&hay_en_estado_ready);
    sem_destroy(&hay_en_estado_new);
    sem_destroy(&sem_multiprogramacion);
    sem_destroy(&desalojo_proceso);
    sem_destroy(&hay_proceso_en_bloq);
    sem_destroy(&hay_proceso_exec);
    sem_destroy(&sem_vrr);
}


void mostrar_lista_de_pids_ready(t_list* lista) { 
    list_iterate(lista, (void (*)(void *))mostrar_pid);  // Iterar sobre cada elemento de la lista y aplicar la función mostrar_pid
}

void mostrar_lista_de_pids_prima(t_list* lista) { 
    list_iterate(lista, (void (*)(void *))mostrar_pid_prima);  // Iterar sobre cada elemento de la lista y aplicar la función mostrar_pid
}

void mostrar_pid (t_pcb* pcb) {
    log_mati(logger, "Ready Normal: %i\n", pcb->pid);
}

void mostrar_pid_prima (t_pcb* pcb) {
    log_mati(logger, "Ready Prima: %i\n", pcb->pid);
}


t_pcb_cpu* rcv_contexto_ejecucion_cpu(int socket_cliente) {
    t_pcb_cpu* proceso = (t_pcb_cpu*)malloc(sizeof(t_pcb_cpu));
    if (proceso == NULL) {
        log_error(logger, "Error al asignar memoria para el proceso");
        return NULL;
    }

    proceso->registros = (t_registro_cpu*)malloc(sizeof(t_registro_cpu));
    if (proceso->registros == NULL) {
        log_error(logger, "Error al asignar memoria para los registros del proceso");
        free(proceso);
        return NULL;
    }

    int size;
    int desplazamiento = 0;

    void* buffer = recibir_buffer(&size, socket_cliente);
    if (buffer == NULL) {
        log_error(logger, "Error al recibir el buffer del socket");
        free(proceso->registros);
        free(proceso);
        return NULL;
    }

    memcpy(&proceso->pid, buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);

    memcpy(&proceso->registros->PC, buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(&proceso->registros->AX, buffer + desplazamiento, sizeof(uint8_t));
    desplazamiento += sizeof(uint8_t);

    memcpy(&proceso->registros->BX, buffer + desplazamiento, sizeof(uint8_t));
    desplazamiento += sizeof(uint8_t);

    memcpy(&proceso->registros->CX, buffer + desplazamiento, sizeof(uint8_t));
    desplazamiento += sizeof(uint8_t);

    memcpy(&proceso->registros->DX, buffer + desplazamiento, sizeof(uint8_t));
    desplazamiento += sizeof(uint8_t);

    memcpy(&proceso->registros->EAX, buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(&proceso->registros->EBX, buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(&proceso->registros->ECX, buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(&proceso->registros->EDX, buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(&proceso->registros->SI, buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(&proceso->registros->DI, buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    free(buffer);
    return proceso;
}