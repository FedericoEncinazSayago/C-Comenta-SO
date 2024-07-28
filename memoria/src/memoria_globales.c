#include "memoria_globales.h"

t_dictionary * diccionario_tabla_de_paginas_porPID;
t_dictionary* diccionario_de_instrucciones_porPID;
void* espacio_de_usuario;
t_config_memoria* config_memoria;
void* memoria_usuario_bitmap;
t_bitarray* bitmap;


void crear_espacio_usuario() {
    espacio_de_usuario = malloc (config_memoria->tam_memoria); // reservamos memoria para el espacio de usuario
}

void crear_bitmap(){
    int cantidad_marcos = config_memoria->tam_memoria / config_memoria->tam_pagina; //calculamos la cantidad de marcos
    memoria_usuario_bitmap = malloc(cantidad_marcos/8);// reservamos memoria para el bitmap
    bitmap = bitarray_create_with_mode(memoria_usuario_bitmap , cantidad_marcos/8, LSB_FIRST);
}

void crear_config_memoria(){
    config_memoria = inicializar_config_memoria();
}

void crear_diccionario_tabla_de_paginas_porPID(){
    diccionario_tabla_de_paginas_porPID = dictionary_create();
}

void inicializacion_diccionario() {
    diccionario_de_instrucciones_porPID = dictionary_create();
    if (diccionario_de_instrucciones_porPID == NULL) {
        log_error(logger, "Error al crear el diccionario");
        exit(1);
    }
}
// Diccionario -> Lista instruc -> Lista parametros -> strdup's de parametros
void cerrar_programa_memoria(int signal) {
    if(signal == SIGINT) {
        log_fede(logger2, "Cerrando programa memoria");

        //liberamos la memoria que reservamos
        dictionary_destroy_and_destroy_elements(diccionario_de_instrucciones_porPID, liberar_lista_instrucciones);
        dictionary_destroy_and_destroy_elements(diccionario_tabla_de_paginas_porPID, liberar_paginas_porPID);

        free(espacio_de_usuario);
        config_destroy_version_memoria(config_memoria);
        bitarray_destroy(bitmap);
        free(memoria_usuario_bitmap);
        // log_destroy(logger);
        // log_destroy2(logger2);
        exit(0);
    }
}


void liberar_lista_instrucciones(void* lista) {
    t_list *lista_instrucciones = (t_list*) lista;
    //log_info(logger, "Entrando a liberar_lista_instrucciones");
    list_destroy_and_destroy_elements(lista_instrucciones, liberar_lista_parametros);
}

void liberar_lista_parametros(void* lista) {
    t_list *lista_parametros = (t_list*) lista;
    //log_info(logger, "Entrando a liberar_lista_parametros");
    list_destroy_and_destroy_elements(lista_parametros, free);
}

void free_parametro_array(void* array) {
    char **parametros = (char**) array;
    //log_info(logger, "Entrando a free_parametro_array");
    for (int i = 0; parametros[i] != NULL; i++) {
        //log_info(logger, "Liberando parámetro: %s", parametros[i]);
        free(parametros[i]);
    }
    free(parametros);
}


void liberar_paginas_porPID(void *paginas) {
    t_list *lista_paginas = paginas;
    list_destroy_and_destroy_elements(lista_paginas, free);
}

void configurar_senial_cierre() {
    struct sigaction sa;
    sa.sa_handler = cerrar_programa_memoria;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error al configurar la senial de cierre");
        exit(1);
    }
}