#include "kernel-peticiones-dispatch.h"

int tam_cola_resource;
t_list** colas_resource_block;
int esta_block;
int proceso_finalizado_por_consola;

void hilo_motivo_de_desalojo() {
    pthread_t hilo_desalojo;

    // Crear el hilo de desalojo y verificar si hay errores
    if (pthread_create(&hilo_desalojo, NULL, escuchar_peticiones_dispatch, NULL) != 0) {
        log_error(logger, "ROMPIO HILO DE DESALOJO");
        return;
    }

    log_info(logger, "Hilo de desalojo creado con éxito!");

    // Detach el hilo para liberar recursos automáticamente cuando el hilo termine
    if (pthread_detach(hilo_desalojo) != 0) {
        log_error(logger, "No se pudo detach el hilo de desalojo");
    }
}

void* escuchar_peticiones_dispatch() {
    while (1) {
        pthread_mutex_lock(&reanudar_ds);
        pthread_mutex_unlock(&reanudar_ds);

        t_tipo_instruccion motivo_desalojo = recibir_operacion(config_kernel->SOCKET_DISPATCH);

        const char* tipo_de_exit = transformar_motivos_a_exit(&motivo_desalojo);

        switch (motivo_desalojo) {
            case FIN_QUANTUM:
                peticion_fin_quantum();
                break;
            case OPERACION_IO:
                peticion_IO();
                break;
            case WAIT:
                peticion_wait();
                break;
            case SIGNAL:
                peticion_signal();
                break;
            case EXIT:
                peticion_exit(tipo_de_exit);
                break;
            case -1:
                log_error(logger, "¡Cerrando Dispatch!");
                return;
            default:
                log_error(logger, "El motivo de desalojo no existe!");
                break;  
        }
    }
    return NULL;
}


const char* transformar_motivos_a_exit(t_tipo_instruccion* motivo_inicial) {

    switch (*motivo_inicial) {

        case EXIT:
            *motivo_inicial = EXIT;
            return "SUCCESS";

        case OUT_OF_MEMORY:
            *motivo_inicial = EXIT;
            return "OUT_OF_MEMORY";

        case INTERRUPTED_BY_USER:
            *motivo_inicial = EXIT;
            return "INTERRUPTED_BY_USER";

        default:
            return "UNKNOWN_ERROR";
    }
}


void peticion_fin_quantum() {
    t_pcb_cpu *pcb_temp = rcv_contexto_ejecucion_cpu(config_kernel->SOCKET_DISPATCH);

    if (!pcb_temp) {
        log_error(logger, "Dispatch acaba de recibir algo inexistente!");
        return;
    }

    pthread_mutex_lock(&mutex_proceso_exec);
    t_pcb* pcb_quantum = proceso_en_exec;
    actualizar_pcb(pcb_quantum, pcb_temp);
    pthread_mutex_unlock(&mutex_proceso_exec);

    if (proceso_finalizado_por_consola == 1) {
        proceso_finalizado_por_consola = 0;
        liberar_procesos(pcb_quantum);
        proceso_en_exec = NULL; // Reiniciar el puntero después de liberar
        liberar_procesos(pcb_temp); // Liberar pcb_temp ya que no se necesita más
        return;
    }

    pthread_mutex_lock(&mutex_estado_ready);
    pthread_mutex_lock(&mutex_proceso_exec);
    pcb_quantum->quantum = config_kernel->QUANTUM;
    list_add(cola_ready, pcb_quantum);
    log_info(logger, "PID: %i - Estado Anterior: EXEC - Estado Actual: READY \n", pcb_quantum->pid);
    pthread_mutex_unlock(&mutex_proceso_exec);
    pthread_mutex_unlock(&mutex_estado_ready);

    sem_post(&desalojo_proceso);

    log_warning(logger, "PID: %i - Desalojado por fin de Quantum", pcb_quantum->pid);

    sem_post(&hay_en_estado_ready);
    liberar_procesos(pcb_temp); // Liberar pcb_temp ya que no se necesita más
    puede_ejecutar_otro_proceso();
}


void peticion_exit(const char *tipo_de_exit) {


    t_pcb_cpu* contexto = rcv_contexto_ejecucion_cpu(config_kernel->SOCKET_DISPATCH);

    if (!contexto) {
        log_error(logger, "Dispatch acaba de recibir algo inexistente!");
        return;
    }

    pthread_mutex_lock(&mutex_proceso_exec);
    t_pcb* pcb_quantum = proceso_en_exec;
    actualizar_pcb(pcb_quantum, contexto);
    pthread_mutex_unlock(&mutex_proceso_exec);

    sem_post(&desalojo_proceso);

    log_warning(logger, "Finaliza el proceso %i - Motivo: %s", pcb_quantum->pid, tipo_de_exit);

    pthread_mutex_lock(&mutex_proceso_exec);
    pcb_quantum->estado = EXITT;
    pcb_quantum->quantum = 0;

    pthread_mutex_lock(&mutex_exit);
    list_add(cola_exit, pcb_quantum);
    pthread_mutex_unlock(&mutex_exit);
    pthread_mutex_unlock(&mutex_proceso_exec);

    log_info(logger, "Se manda a Memoria para liberar el Proceso");
    informar_a_memoria_liberacion_proceso(pcb_quantum->pid);
    log_info(logger, "Aumentamos Grado de Multiprogramacion por EXIT \n");

    pthread_mutex_lock(&mutex_proceso_exec);
    liberar_recurso_por_exit(pcb_quantum);
    pthread_mutex_unlock(&mutex_proceso_exec);
    liberar_procesos(contexto);
    puede_ejecutar_otro_proceso();
}


void peticion_wait() {

    char* recurso;
    t_pcb_cpu* contexto = recibir_contexto_y_recurso(&recurso);
    if (!contexto) {
        log_error(logger, "Dispatch acaba de recibir algo inexistente!");
        puede_ejecutar_otro_proceso();
        free(recurso);
        return;
    }

    pthread_mutex_lock(&mutex_proceso_exec);
    t_pcb* pcb = proceso_en_exec;
    actualizar_pcb(pcb, contexto);
    pthread_mutex_unlock(&mutex_proceso_exec);
    liberar_procesos(contexto);

    if (!recurso_existe(recurso)) {
        log_error(logger, "El recurso solicitado no existe!");
        finalizar_por_invalidacion(pcb, "INVALID_RESOURCE");
        free(recurso);
        puede_ejecutar_otro_proceso();
        return;
    }
    
    int indice_recurso = obtener_indice_recurso(recurso);
    
    if (config_kernel->INST_RECURSOS[indice_recurso] > 0) {
        config_kernel->INST_RECURSOS[indice_recurso]--;
        sem_post(&desalojo_proceso);

        // Guardar el pid y el recurso en el vector
        for (int i = 0; i < tam_vector_recursos_pedidos; i++) {
            if (vector_recursos_pedidos[i].PID == -1) {
                vector_recursos_pedidos[i].PID = pcb->pid;
                vector_recursos_pedidos[i].recurso = strdup(recurso);
                log_facu(logger2, "PID: %i - Asignado: %s", pcb->pid, recurso);
                log_facu(logger2, "¡WAIT exitoso!\n");
                break;
            }
        }
        
        int wait_exitoso = 1;
        send(config_kernel->SOCKET_DISPATCH, &wait_exitoso, sizeof(int),  MSG_WAITALL);

    } else {
        log_leo(logger2, "Recurso no disponible actualmente");
        log_leo(logger2, "Moviendo proceso a BLOCK");

        int wait_fallido = 0;
        send(config_kernel->SOCKET_DISPATCH, &wait_fallido, sizeof(int),  MSG_WAITALL);
        
        pthread_mutex_lock(&mutex_proceso_exec);
        pcb->quantum = config_kernel->QUANTUM;
        pthread_mutex_unlock(&mutex_proceso_exec);

        sem_post(&desalojo_proceso);
        esta_block = 1;

        pthread_mutex_lock(&mutex_proceso_exec);
        if(strcmp(config_kernel->ALGORITMO_PLANIFICACION, "VRR") == 0) {
            sem_wait(&sem_vrr);
            pthread_mutex_lock(&mutex_proceso_exec);
            pcb->quantum = quantum_restante;
            pthread_mutex_unlock(&mutex_proceso_exec);
        }
        mover_a_bloqueado_por_wait(pcb, recurso);
        pthread_mutex_unlock(&mutex_proceso_exec);

        puede_ejecutar_otro_proceso();
    }
    free(recurso);
}


void peticion_signal() {
    int signal_fallido = 0;
    char* recurso;
    
    t_pcb_cpu* contexto = recibir_contexto_y_recurso(&recurso);
    if (!contexto) {
        log_error(logger, "Dispatch acaba de recibir algo inexistente!");
        send(config_kernel->SOCKET_DISPATCH, &signal_fallido, sizeof(int), MSG_WAITALL);
        puede_ejecutar_otro_proceso();
        return;
    }
    pthread_mutex_lock(&mutex_proceso_exec);
    t_pcb* pcb = proceso_en_exec;
    actualizar_pcb(pcb, contexto);
    pthread_mutex_unlock(&mutex_proceso_exec);

    sem_post(&desalojo_proceso);

    if (!recurso_existe(recurso)) {
        log_error(logger, "El recurso solicitado no existe!");
        finalizar_por_invalidacion(pcb, "INVALID_RESOURCE");
        send(config_kernel->SOCKET_DISPATCH, &signal_fallido, sizeof(int), MSG_WAITALL);
        free(recurso);
        puede_ejecutar_otro_proceso();
        return;
    }

    // for(int i = 0; i < tam_vector_recursos_pedidos; i++) {
    //     log_info(logger, "pid: %i",vector_recursos_pedidos[i].PID);
    //     log_info(logger, "rec: %s", vector_recursos_pedidos[i].recurso);
    // }

    int indice_recurso = obtener_indice_recurso(recurso);
    config_kernel->INST_RECURSOS[indice_recurso]++;
    // Liberar el pid y el recurso en el vector
    for (int i = 0; i < tam_vector_recursos_pedidos; i++) {
        int size = strlen(config_kernel->RECURSOS[indice_recurso]);
        if (vector_recursos_pedidos[i].PID == pcb->pid && strncmp(vector_recursos_pedidos[i].recurso, recurso, size) == 0) {
            log_facu(logger2, "Proceso: %i - Devuelve: %s", pcb->pid, recurso);
            vector_recursos_pedidos[i].PID = -1;
            free(vector_recursos_pedidos[i].recurso);
            vector_recursos_pedidos[i].recurso = NULL;
        }
    }

    log_facu(logger2, "¡SIGNAL exitoso!");
    int signal_exitoso = 1;
    send(config_kernel->SOCKET_DISPATCH, &signal_exitoso, sizeof(int), MSG_WAITALL);

    if (!list_is_empty(colas_resource_block[indice_recurso])) {
        pthread_mutex_lock(&mutex_estado_block);
        t_pcb* pcb_signal = list_remove(colas_resource_block[indice_recurso], 0);
        pthread_mutex_unlock(&mutex_estado_block);
        log_info(logger, "¡Desbloqueando Proceso por Recurso!");
        mover_procesos_de_bloqueado_a_ready(pcb_signal);
        //liberar_pcb(pcb_signal);
    }

    free(recurso);
    liberar_procesos(contexto);
    //sem_post(&pedidos);
}


void mover_a_bloqueado_por_wait(t_pcb* pcb, char* recurso) {

    int indice_recurso = obtener_indice_recurso(recurso);

    pthread_mutex_lock(&mutex_estado_block);
    pcb->estado = BLOCK;
    list_add(colas_resource_block[indice_recurso], pcb);
    pthread_mutex_unlock(&mutex_estado_block);

    mover_a_cola_block_general(pcb, recurso);
}


void mover_a_cola_block_general(t_pcb* pcb, char* motivo) {

    pthread_mutex_lock(&mutex_cola_block);
    log_leo(logger2, "PID: %i - Bloqueado por: %s", pcb->pid, motivo);
    log_info(logger, "PID: %i - Estado Anterior: EXEC - Estado Actual: BLOCK", pcb->pid);
    //log_facu(logger2, "Quantum en Block: %i", pcb->quantum);
    pcb->estado = BLOCK;
    list_add(cola_block, pcb);
    pthread_mutex_unlock(&mutex_cola_block);

    sem_post(&hay_proceso_en_bloq);
}


int obtener_indice_recurso(char* recurso) {
    for (int i = 0; i < string_array_size(config_kernel->RECURSOS); i++) {
        int size = strlen(config_kernel->RECURSOS[i]);
        if (strncmp(config_kernel->RECURSOS[i], recurso, size) == 0) { //ACORDATE DE CAMBIARLOOOO!!! TODO
            return i;
        }
    }
    return -1;
}


bool recurso_existe(char* recurso) {
    return obtener_indice_recurso(recurso) != -1;
}


t_pcb_cpu* recibir_contexto_y_recurso(char** recurso) {
    t_pcb_cpu* proceso = rcv_contexto_ejecucion_cpu(config_kernel->SOCKET_DISPATCH);
    if (!proceso) {
        log_error(logger, "Error al recibir el contexto de ejecución");
        return NULL;
    }

    // Recibimos el nombre del recurso:
    rcv_nombre_recurso(recurso, config_kernel->SOCKET_DISPATCH);

    if (!*recurso) {
        log_error(logger, "Error al recibir el nombre del recurso");
        free(proceso->registros);
        free(proceso);
        return NULL;
    }

    return proceso;
}


void peticion_IO() {

    t_pcb_cpu* contexto = rcv_contexto_ejecucion_cpu(config_kernel->SOCKET_DISPATCH);

    if (!contexto) {
        log_error(logger, "Error al recibir el contexto de ejecución para IO");
        return;
    }

    // Actualizamos el PCB con el contexto recibido:
    pthread_mutex_lock(&mutex_proceso_exec);
    t_pcb* pcb = proceso_en_exec;
    actualizar_pcb(pcb, contexto);
    pcb->quantum = config_kernel->QUANTUM;
    pthread_mutex_unlock(&mutex_proceso_exec);
    
    // Desalojamos el proceso actual:
    esta_block = 1;
    sem_post(&desalojo_proceso);

    // Esperamos una respuesta de CPU para poder seguir con la ejecución:
    int response = 1;
    send(config_kernel->SOCKET_DISPATCH, &response, sizeof(int), 0);

    // Recibimos la interfaz y los argumentos:
    int pid = pcb->pid;
    t_list *interfaz_y_argumentos = recv_interfaz_y_argumentos(config_kernel->SOCKET_DISPATCH, pid);

    // Obtenemos los argumentos de la interfaz:
    tipo_operacion *operacion = list_remove(interfaz_y_argumentos, 0);
    char *nombre_interfaz = list_remove(interfaz_y_argumentos, 0); 
    t_list *args = list_remove(interfaz_y_argumentos, 0);

    // Logueamos:
    log_warning(logger, "¡Peticion I/O, desalojando Proceso: %i", contexto->pid);
    log_info(logger, "Operación: %i - Interfaz: %s", *operacion, nombre_interfaz);
    log_info(logger, "Interfaz: %s - PID: %i", nombre_interfaz, pcb->pid);

    interface_io* interface = get_interface_from_dict(nombre_interfaz);
    
    if (!interface || !acepta_operacion_interfaz(interface, *operacion) || !estado_de_conexion_interface(interface)) {
        // Logueamos el error:
        log_error(logger, "La interfaz %s no acepta la operación %i o no está conectada", nombre_interfaz, *operacion);
        puede_ejecutar_otro_proceso();

        // Finalizamos el proceso por invalidación:
        finalizar_por_invalidacion(pcb, "INVALID_INTERFACE");

        // Liberamos memoria de los elementos que ya no se necesitan:
        free(nombre_interfaz);
        free(operacion);
        list_destroy_and_destroy_elements(args, free);
        list_destroy(interfaz_y_argumentos);
        liberar_procesos(contexto);

        return;
    } 

    // Si el planificador es VRR, se le asigna el quantum restante al proceso:
    if (strcmp(config_kernel->ALGORITMO_PLANIFICACION, "VRR") == 0) {
        sem_wait(&sem_vrr);
        pthread_mutex_lock(&mutex_proceso_exec);
        pcb->quantum = quantum_restante;
        pthread_mutex_unlock(&mutex_proceso_exec);
    }

    // Movemos el proceso a la cola de bloqueados:
    pthread_mutex_lock(&mutex_proceso_exec);
    mover_a_cola_block_general(pcb, "INTERFAZ");
    pthread_mutex_unlock(&mutex_proceso_exec);

    // Agregamos el proceso junto a sus argumentos a la cola de bloqueados:
    pthread_mutex_lock(&interface->mutex_blocked);
    queue_push(interface->process_blocked, pcb);
    queue_push(interface->args_process, args);
    sem_post(&interface->size_blocked);
    pthread_mutex_unlock(&interface->mutex_blocked);

    puede_ejecutar_otro_proceso();
    // Liberamos memoria de los elementos que ya no se necesitan:
    free(nombre_interfaz);
    free(operacion);
    liberar_procesos(contexto);
    list_destroy(interfaz_y_argumentos); // TODO
    // Avisamos al planificador que puede ejecutar otro proceso:
}


void rcv_nombre_recurso(char** recurso, int socket) {
    int code = recibir_operacion(socket);
    int desplazamiento = 0;;
    int size;
    int tamanio = 0;

    void* buffer = recibir_buffer(&size, socket);
        if(!buffer){
            log_error(logger, "Error al recibir el bufer en WAIT");
            return;
        }
        
    memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);

    *recurso = malloc(tamanio);

    memcpy(*recurso, buffer + desplazamiento, tamanio);
    desplazamiento += tamanio;
        
    free(buffer);     
}


void inicializar_vector_recursos_pedidos() {
    tam_vector_recursos_pedidos = calcular_total_instancias();
    vector_recursos_pedidos = malloc(tam_vector_recursos_pedidos * sizeof(t_recursos_pedidos));

    for (int i = 0; i < tam_vector_recursos_pedidos; i++) {
        vector_recursos_pedidos[i].PID = -1;
        vector_recursos_pedidos[i].recurso = NULL;
    }

}


int calcular_total_instancias() {
    int suma = 0;
    for (int i = 0; i < string_array_size(config_kernel->RECURSOS); i++) {
        suma += config_kernel->INST_RECURSOS[i];
    }
    return suma;
}


int inicializar_tam_cola_resources() {
    tam_cola_resource = string_array_size(config_kernel->RECURSOS);
    //log_facu(logger2, "%i", tam_cola_resource);
    return tam_cola_resource;
}


void inicializar_cola_resource_block() {
    tam_cola_resource = inicializar_tam_cola_resources();
    
    // Asignar memoria para el array de punteros a colas
    colas_resource_block = malloc(tam_cola_resource * sizeof(t_list*));
    if (colas_resource_block == NULL) {
        // Manejar error de asignación de memoria
        log_error(logger, "No se pudo asignar memoria para colas_resource_block");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < tam_cola_resource; i++) {
        colas_resource_block[i] = list_create();
    }
}


void finalizar_por_invalidacion(t_pcb* pcb, const char* tipo_invalidacion) {
    
    log_warning(logger, "Finaliza el proceso: %i - Motivo: %s", pcb->pid, tipo_invalidacion);

    pthread_mutex_lock(&mutex_exit);
    pcb->estado = EXITT;
    list_add(cola_exit, pcb);
    pthread_mutex_unlock(&mutex_exit);

    informar_a_memoria_liberacion_proceso(pcb->pid);

    liberar_recurso_por_exit(pcb);
}


void liberar_recurso_por_exit(t_pcb* pcb) {
    for (int i = 0; i < tam_vector_recursos_pedidos; i++) {
        if (vector_recursos_pedidos[i].PID == pcb->pid) {
            int indice_recurso = obtener_indice_recurso(vector_recursos_pedidos[i].recurso);

            if (indice_recurso != -1) {
                config_kernel->INST_RECURSOS[indice_recurso]++;
                log_info(logger, "Liberado %s - PID: %i", vector_recursos_pedidos[i].recurso, pcb->pid);

                if (!list_is_empty(colas_resource_block[indice_recurso])) {
                    pthread_mutex_lock(&mutex_estado_block);
                    t_pcb* pcb_bloqueado = list_remove(colas_resource_block[indice_recurso], 0);
                    pthread_mutex_unlock(&mutex_estado_block);

                    mover_procesos_de_bloqueado_a_ready(pcb_bloqueado);
                }
            } else {
                log_error(logger, "¡Hay una descoordinación, recurso no coincide con PID: %i", pcb->pid);
                continue;
            }

            vector_recursos_pedidos[i].PID = -1;
            free(vector_recursos_pedidos[i].recurso);
            vector_recursos_pedidos[i].recurso = NULL;
        }
    }
    if(proceso_finalizado_por_consola == 1) {
        sem_post(&desalojo_proceso);
        proceso_finalizado_por_consola = 0;
    }
    //sem_post(&desalojo_proceso); TODO
    sem_post(&sem_multiprogramacion);
}


void liberar_procesos(t_pcb* pcb) {
    if (pcb != NULL) {
        if (pcb->registros != NULL) {
            free(pcb->registros);
            pcb->registros = NULL;  // Previene la doble liberación
        }
        free(pcb);
        pcb = NULL;  // Previene la doble liberación
    }
}


void mostrar_pcb(t_pcb* pcb){
    log_info(logger,"PID: %i", pcb->pid);
    log_info(logger,"Reg PC:%i",pcb->registros->PC);
    log_info(logger,"Reg AX:%i",pcb->registros->AX);
    log_info(logger,"Reg BX:%i",pcb->registros->BX);
    log_info(logger,"Reg CX:%i",pcb->registros->CX);
    log_info(logger,"Reg DX:%i",pcb->registros->DX);
    log_info(logger,"Reg EAX:%i",pcb->registros->EAX);
    log_info(logger,"Reg EBX:%i",pcb->registros->EBX);
    log_info(logger,"Reg ECX:%i",pcb->registros->ECX);
    log_info(logger,"Reg EDX:%i",pcb->registros->EDX);
    log_info(logger,"Reg SI:%i",pcb->registros->SI);
    log_info(logger,"Reg DI:%i",pcb->registros->DI);
}


void actualizar_pcb(t_pcb* proceso_a_actualizar, t_pcb_cpu* contexto) {
    if(contexto) {
        proceso_a_actualizar->pid = contexto->pid;
        if(contexto->registros) {
            proceso_a_actualizar->registros->PC = contexto->registros->PC;

            proceso_a_actualizar->registros->AX = contexto->registros->AX;
            proceso_a_actualizar->registros->BX = contexto->registros->BX;
            proceso_a_actualizar->registros->CX = contexto->registros->CX;
            proceso_a_actualizar->registros->DX = contexto->registros->DX;

            proceso_a_actualizar->registros->EAX = contexto->registros->EAX;
            proceso_a_actualizar->registros->EBX = contexto->registros->EBX;
            proceso_a_actualizar->registros->ECX = contexto->registros->ECX;
            proceso_a_actualizar->registros->EDX = contexto->registros->EDX;

            proceso_a_actualizar->registros->SI = contexto->registros->SI;
            proceso_a_actualizar->registros->DI = contexto->registros->DI;
        }
        else 
            log_error(logger, "¡Registros nulos!");
      
    }
    else
        log_error(logger, "¡PCB Nula!");
}
