#include "memoria_instrucciones.h"

pthread_mutex_t mutex_instrucciones;

void inicializacion_semaforos() {
    pthread_mutex_init(&mutex_instrucciones, NULL);
}

char* crear_path_instrucciones(char* archivo_path) {
    char *path = string_new();
    string_append(&path, config_memoria->path_instrucciones);
    string_append(&path, archivo_path);
    
    if (string_contains(path, "./")) {
        char *buffer = malloc(100 * sizeof(char));
        getcwd(buffer, 100);
        string_append(&buffer, "/");
        char *old_path = path;
        path = string_replace(path, "./", buffer);
        free(old_path);
        free(buffer);
    } else if (string_contains(path, "~/")) {
        char *old_path = path;
        path = string_replace(path, "~/", "/home/utnso/");
        free(old_path);
    }

    return path;
}
// Obtenemos las instrucciones de los archivos de pseudocódigo

void leer_archivoPseudo(int socket_kernel) {
    int size;
    int tam = 0;
    int pid;
    int desplazamiento = 0;
    void *buffer = recibir_buffer(&size, socket_kernel);

    // Recibimos el PID:
    memcpy(&pid, buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);

    // Recibimos el tamaño del archivo:
    memcpy(&tam, buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);

    // Recibimos el nombre del archivo:
    char* archivo_path = malloc(tam + 1);
    memcpy(archivo_path, buffer + desplazamiento, tam);
    archivo_path[tam] = '\0'; // Añadir el terminador nulo
    log_info(logger, "Nombre del archivo recibido: %s", archivo_path);
    free(buffer);
    //creamos el proceso asignado al PID
    crear_proceso(pid);

    char* path = crear_path_instrucciones(archivo_path);
    free(archivo_path);

    //Abrimos archivo_path para leer broOOOOoooOoder
    FILE *archivo = fopen(path, "r");

    log_info(logger, "Abriendo archivo: %s", path);
    //Comprobar si el archivo existe
    if(archivo == NULL){
        log_error(logger, "Error en la apertura del archivo: Error");
        free(path);
        return;
    }

    char *cadena = NULL;
    char **parametros = NULL;
    size_t len = 0;      // Tamaño del buffer
    ssize_t read;        
    t_list *lista_de_instrucciones = list_create();
    t_list *parametros_instruccion;

    // leemos el archivo y lo pasamos a una lista
    while ((read = getline(&cadena, &len, archivo)) != -1) { // && program_counter != -1
        parametros = string_split(cadena, " "); //separamos los parametros
        parametros_instruccion = list_create();

        for (int i = 0; parametros[i] != NULL; i++) {
            list_add(parametros_instruccion, strdup(parametros[i])); // Duplicar la cadena
        }
        
        list_add(lista_de_instrucciones, parametros_instruccion);

        // Liberar la memoria de `parametros`
        for (int i = 0; parametros[i] != NULL; i++) {
            free(parametros[i]);
        }

        free(parametros);
    }

    agregar_a_diccionario_instrucciones(pid, lista_de_instrucciones);


    free(cadena);
    free(path);
    fclose(archivo);

    //liberar_lista_instrucciones(lista_de_instrucciones);
}


void enviar_instruccion_a_cpu(int socket_cpu) {

    int pid;
    int program_counter;
    int desplazamiento = 0;
    int size; 

    void *buffer = recibir_buffer(&size, socket_cpu);

    memcpy(&pid, buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);

    memcpy(&program_counter, buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);

    //recibir_program_counter(socket_cpu, &program_counter, &pid);

    //log_info(logger, "Recibiendo program counter %i para el PID %i", program_counter, pid);

    t_list* lista_de_instrucciones = obtener_lista_instrucciones(pid);

    if(lista_de_instrucciones == NULL){
        log_error(logger, "No se encontró la lista de instrucciones para el PID %i", pid);
        t_paquete *paquete = crear_paquete(INSTRUCCION);
        int error = -1;
        agregar_a_paquete(paquete, &error, sizeof(int));
        enviar_paquete(paquete, socket_cpu);
        eliminar_paquete(paquete);
        free(buffer);
        return;
    }

    t_paquete *paquete = crear_paquete(INSTRUCCION);
    t_list *parametros = list_get(lista_de_instrucciones, program_counter);

    for(int i = 0; i < list_size(parametros); i++){
        char *parametro = list_get(parametros, i);
        agregar_a_paquete_string(paquete, parametro, strlen(parametro) + 1);
    }

    enviar_paquete(paquete, socket_cpu);
    eliminar_paquete(paquete);

    free(buffer);
}

void agregar_a_diccionario_instrucciones(int pid, t_list *lista_de_instrucciones) {
    char *pid_en_string = string_itoa(pid);

    //log_warning(logger, "Agregando lista de instrucciones para el PID %s", pid_en_string);
    dictionary_put(diccionario_de_instrucciones_porPID, pid_en_string, lista_de_instrucciones);
    free(pid_en_string);
}

t_list* obtener_lista_instrucciones(int pid) {
    char *pid_en_string = string_itoa(pid);

    if(!dictionary_has_key(diccionario_de_instrucciones_porPID, pid_en_string)){
        log_error(logger, "Esta key no esta en el diccionario pelotudos %i", pid);
        free(pid_en_string);
        return NULL;        
    }
    t_list* lista_de_instrucciones = dictionary_get(diccionario_de_instrucciones_porPID, pid_en_string);
    free(pid_en_string);
    //log_warning(logger, "Obteniendo lista de instrucciones para el PID %i", pid);
    return lista_de_instrucciones;
}       