#include "consola.h"

int pid_buscado_global;
int esta_pausada;

pthread_mutex_t reanudar_ds;
pthread_mutex_t reanudar_plani;
pthread_mutex_t reanudar_largo;
pthread_mutex_t reanudar_block;


COMMAND comandos[] = {
    {"INICIAR_PROCESO", iniciar_proceso},
    {"PROCESO_ESTADO", proceso_estado},
    {"MULTIPROGRAMACION", multiprogramacion},
    {"EJECUTAR_SCRIPT", ejecutar_script},
    {"FINALIZAR_PROCESO", finalizar_proceso},
    {"DETENER_PLANIFICACION", detener_planificacion},
    {"INICIAR_PLANIFICACION", iniciar_planif},
    {NULL, NULL}
};


void iniciar_consola() {
    char* linea;
    char* operacion;

    inicializar_mutex_consola();

    iniciar_readline();

    while (1) {
        linea = readline("C-Comenta: ");

        if (!linea)
            break;

        operacion = eliminar_espacios(linea);

        if (operacion != NULL) {
            add_history(linea);
            ejecutar_comando(linea);
        }

        free(linea);
        //free(operacion); //cambiado
    }
}


int ejecutar_comando(char* linea) {
    int i = 0;
    char* palabra;
    COMMAND* comando;

    while (linea[i] && isspace(linea[i]))
        i++;

    palabra = linea + i;

    while (linea[i] && !isspace(linea[i]))
        i++;

    if (linea[i])
        linea[i++] = '\0';

    comando = encontrar_comando(palabra);

    if (!comando) {
        printf("Comando desconocido: %s\n", palabra);
        return -1;
    }

    while (linea[i] && isspace(linea[i]))
        i++;

    palabra = linea + i;

    ((comando->funcion)(palabra));
 //cambiado
    return 1;
}


COMMAND* encontrar_comando(char* nombre) {
    for (int i = 0; comandos[i].nombre; i++) {
        if (strcmp(nombre, comandos[i].nombre) == 0)
            return &comandos[i];
    }

    return NULL;
}


void iniciar_readline() {
    rl_readline_name = "C-Comenta";
    rl_attempted_completion_function = completar_CComenta;
}


char** completar_CComenta(const char* texto, int inicio, int fin) {
    char** matches = NULL;

    if (inicio == 0)
        matches = rl_completion_matches(texto, generador_de_comandos);

    return matches;
}


char* generador_de_comandos(const char* texto, int estado) {
    static int lista_index, len; //liberar
    char* nombre;

    if (!estado) {
        lista_index = 0;
        len = strlen(texto);
    }

    while ((nombre = comandos[lista_index].nombre)) {
        lista_index++;

        if (strncmp(nombre, texto, len) == 0)
            return strdup(nombre);
    }
    free(nombre);

    return NULL;
}


void* iniciar_proceso(void* args) {
    char *path = (char*) args; // Path del archivo a ejecutar

    // Copiar el path si es necesario, para evitar usar directamente el puntero args
    char *path_copy = strdup(path);
    if (!path_copy) {
        log_error(logger, "Error al duplicar el path del proceso");
        free(path);  // Liberar path si strdup falla
        return NULL;
    }

    creacion_proceso(path_copy); // Invocamos a la función que crea el proceso

    free(path_copy); // Liberamos el path duplicado después de su uso
    //free(path);  // Liberar el argumento original
    return NULL;
}


void* proceso_estado(void* args) {
    printf("\nListando procesos por estado:\n");

    pthread_mutex_lock(&mutex_estado_new);
    imprimir_procesos_en_cola("NEW", cola_new);
    pthread_mutex_unlock(&mutex_estado_new);

    pthread_mutex_lock(&mutex_estado_ready);
    imprimir_procesos_en_cola("READY", cola_ready);
    pthread_mutex_unlock(&mutex_estado_ready);

    imprimir_proceso_exec();

    pthread_mutex_lock(&mutex_estado_block);
    imprimir_procesos_en_cola("BLOCK", cola_block);
    pthread_mutex_unlock(&mutex_estado_block);

    pthread_mutex_lock(&mutex_exit);
    imprimir_procesos_en_cola("EXIT", cola_exit);
    pthread_mutex_unlock(&mutex_exit);


    return NULL;
}


void imprimir_procesos_en_cola(char* estado, t_list* cola) {
    int size_estados_proceso = list_size(cola);

    for (int i = 0; i < size_estados_proceso; i++) {
        t_pcb* pcb = list_get(cola, i);
        log_info(logger, "Proceso/s en %s: %d\n", estado, pcb->pid);
    }
}


void imprimir_proceso_exec() {
    pthread_mutex_lock(&mutex_estado_exec);
    if(proceso_en_exec && proceso_en_exec->estado == EXEC) {
        if (proceso_en_exec) {
            log_info(logger, "Proceso en EXEC: %d\n", proceso_en_exec->pid);
        } else {
            log_info(logger, "No hay proceso en ejecución.\n");
        }
    }
    pthread_mutex_unlock(&mutex_estado_exec);
}


void* multiprogramacion(void* args) {
    char *multiprogramacion = (char*) args;
    int nuevo_grado_multiprogramacion = atoi(multiprogramacion);
    int diferencia = nuevo_grado_multiprogramacion - config_kernel->GRADO_MULTIP;
    if(diferencia < 0) {// O sea, se baja el grado de multi 
        for(int i = 0; i < diferencia + 1; i++) 
            sem_wait(&sem_multiprogramacion);
    }
    else {
        for(int i = 0; i < diferencia + 1; i++)
            sem_post(&sem_multiprogramacion);
    }
    config_kernel->GRADO_MULTIP = nuevo_grado_multiprogramacion;
    return NULL;
    free(multiprogramacion);
}


void* ejecutar_script(void* args) {
    char *script_path = (char*) args;
    char *path_inicial = "/home/utnso/c-comenta-pruebas";

    // Calcular el tamaño necesario para path_nuevo
    size_t len_path_nuevo = strlen(path_inicial) + strlen(script_path) + 1;

    // Asignar memoria para path_nuevo
    char *path_nuevo = malloc(len_path_nuevo);
    if (path_nuevo == NULL) {
        log_error(logger, "Error al asignar memoria para path_nuevo");
        return NULL;
    }
    // Copiar path_inicial a path_nuevo
    strcpy(path_nuevo, path_inicial);

    // Concatenar script_path a path_nuevo
    strcat(path_nuevo, script_path);

    FILE* archivo_script = fopen(path_nuevo, "r");
    if (!archivo_script) {
        log_error(logger, "¡Archivo de script erroneo!");
        free(path_nuevo); // Liberar path_nuevo en caso de error
        return NULL;
    }

    char comando[MAX_COMMAND_LETTERS];
    while (fgets(comando, sizeof(comando), archivo_script)) {
        comando[strcspn(comando, "\n")] = '\0'; // Eliminar el salto de línea si existe
        log_nico(logger2, "Ejecutando comando: %s", comando);
        ejecutar_comando(comando);
    }
    
    fclose(archivo_script);
    free(path_nuevo);

    return NULL;
}


void* finalizar_proceso(void* pid) {
    int pid_buscado = atoi((char*)pid);

    pthread_mutex_lock(&mutex_estado_exec);
    if (proceso_en_exec != NULL && pid_buscado == proceso_en_exec->pid) {
        proceso_finalizado_por_consola = 1;
        puede_ejecutar_otro_proceso();
        finalizar_por_invalidacion(proceso_en_exec, "INTERRUPTED_BY_USER");
        log_info(logger, "¡Proceso a Finalizar encontrado en EXEC!");
        pthread_mutex_unlock(&mutex_estado_exec);
    } else {
        pthread_mutex_unlock(&mutex_estado_exec);

        t_pcb* pcb = pcb_encontrado(cola_block, pid_buscado);
        if (pcb && list_remove_element(cola_block, pcb)) {
            eliminar_proceso_de_cola_recursos(pcb->pid);
            esta_finalizado = 1;
            puede_ejecutar_otro_proceso();
            finalizar_por_invalidacion(pcb, "INTERRUPTED_BY_USER");
            log_info(logger, "Proceso a Finalizar encontrado en BLOCK");
        } else {
            pthread_mutex_lock(&mutex_estado_ready);
            pcb = pcb_encontrado(cola_ready, pid_buscado);
            if (pcb && list_remove_element(cola_ready, pcb)) {
                puede_ejecutar_otro_proceso();
                finalizar_por_invalidacion(pcb, "INTERRUPTED_BY_USER");
                log_info(logger, "Proceso a Finalizar encontrado en READY");
                pthread_mutex_unlock(&mutex_estado_ready);
            } else {
                pthread_mutex_unlock(&mutex_estado_ready);

                pthread_mutex_lock(&mutex_cola_priori_vrr);
                pcb = pcb_encontrado(cola_prima_VRR, pid_buscado);
                if (pcb && list_remove_element(cola_prima_VRR, pcb)) {
                    puede_ejecutar_otro_proceso();
                    finalizar_por_invalidacion(pcb, "INTERRUPTED_BY_USER");
                    log_info(logger, "Proceso a Finalizar encontrado en READY_VRR");
                    pthread_mutex_unlock(&mutex_cola_priori_vrr);
                } else {
                    pthread_mutex_unlock(&mutex_cola_priori_vrr);
                    log_error(logger, "¡No existe el PID: %i ! Ingrese nuevamente ...", pid_buscado);
                }
            }
        }
    }

    return NULL;
}


void eliminar_proceso_de_cola_recursos(int pid_buscado) {
    for (int i = 0; i < tam_cola_resource; i++) {
        t_pcb* pcb = pcb_encontrado(colas_resource_block[i], pid_buscado);
        if (pcb && list_remove_element(colas_resource_block[i], pcb)) {
            break;
        }
    }
}


t_pcb* pcb_encontrado(t_list* cola_a_buscar_pid, int pid_buscado) {
    pid_buscado_global = pid_buscado;
    t_pcb* resultado = list_find(cola_a_buscar_pid, es_el_proceso_buscado);
    return resultado;
}


bool es_el_proceso_buscado(void* elemento) {
    t_pcb* pcb = (t_pcb*)elemento;
    return pid_buscado_global == pcb->pid;
}


// /*Detener planificacion: Este mensaje se encargara de pausar la planificacion de corto y largo plazo.
// El proceso que se encuentra en ejecucion NO es desalojado, pero una vez que salga de EXEC se va a 
// pausar el manejo de su motivo de desalojo. De la misma forma, los procesos bloqueados van a pausar
// su transicion a la cola de Ready.
// Nomenclatura: DETENER_PLANIFICACION*/

void *detener_planificacion(void* args) {
    if(esta_pausada == 1) {
        log_warning(logger, "La consola ya se encuentra detenida...");
    }
    else {
        pthread_mutex_lock(&reanudar_plani);
        pthread_mutex_lock(&reanudar_largo);
        pthread_mutex_lock(&reanudar_ds);
        pthread_mutex_lock(&reanudar_block);
        esta_pausada = 1;
        log_info(logger, "Planificacion pausada!");
    }
    return NULL;
}
// Iniciar planificacion: Este mensaje se encargara de retomar (en caso que se encuentre pausada) 
// la planificacion de corto y largo plazo. En caso que la planificacion no se encuentre pausada, 
// se debe ignorar el mensaje.
// Nomenclatura: INICIAR_PLANIFICACION


void *iniciar_planif(void* args){
    if(esta_pausada == 1){
        pthread_mutex_unlock(&reanudar_plani);
        pthread_mutex_unlock(&reanudar_largo);
        pthread_mutex_unlock(&reanudar_ds);
        pthread_mutex_unlock(&reanudar_block);
        esta_pausada = 0;
        log_info(logger, "Planificacion reanudada!");
    } else {
        log_info(logger, "La planificación ya está en marcha, ignorando comando ...");
    }
    return NULL;
}


void inicializar_mutex_consola() {
    pthread_mutex_init(&reanudar_plani, NULL);
    pthread_mutex_init(&reanudar_largo, NULL);
    pthread_mutex_init(&reanudar_ds, NULL);
    pthread_mutex_init(&reanudar_block, NULL);
}

