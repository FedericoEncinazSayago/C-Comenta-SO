#include "shared.h"

void validar_configuraciones(t_config *config, char *configuraciones[]) {
    if (!tiene_todas_las_configuraciones(config, configuraciones)) {
        log_error(logger, "Faltan configuraciones en el archivo de configuración");
        exit(EXIT_FAILURE);
    }
}

void info_config(t_config *config) {
    log_info(logger, "Configuraciones cargadas correctamente");
    //config_destroy(config);
}

int tiene_todas_las_configuraciones(t_config* config, char* configs[]) {  
    for(size_t i = 0; configs[i] !=  NULL; i++) {
        if(!config_has_property(config, configs[i]))
            return 0;
    }

    return 1;
}

int es_esta_palabra(char* palabra, char* palabraNecesaria) // Funcion: compara dos palabras y devuelve true si son iguales
{ 
    return strcmp(palabra, palabraNecesaria) == 0;
}

int  tiene_algun_algoritmo_de_reemplazo(char* palabra) // Funcion: devuelve true si la palabra es CLOCK o LRU
{ 
    if (es_esta_palabra(palabra, "FIFO"))
        return 1;
    else 
    {
        if (es_esta_palabra(palabra, "LRU")) 
            return 1;
    }

    return 0;
}

int tiene_algun_algoritmo_de_planificacion(char* palabra) // Funcion: devuelve true si la palabra es FIFO, RR o PRIORIDADES
{ 
    if (es_esta_palabra(palabra, "FIFO"))
        return 1;
    else 
    {
        if (es_esta_palabra(palabra, "RR")) 
            return 1;
        else
        {

            if(es_esta_palabra(palabra, "PRIORIDADES"));

                return 1;
        }
    }

    return 0;
}

void liberar_espacios_de_memoria(void** vector) // Funcion: libera todos los espacios de memoria de un array de punteros asignados dinamicamente
{ 
    for(size_t i = 0; vector[i] != NULL; i++) {
        free(vector[i]);
    }

    free(vector);
} // Comentario: tipo de dato (cualquiera)** nombre de la variable = array de punteros

void copiar_valor(char **destino, char *valor) {
    if (*destino) {
        free(*destino);
        *destino = NULL; // Asegurar que el puntero no apunte a una dirección inválida
    }
    if (valor) {
        *destino = strdup(valor);
    } else {
        *destino = NULL;
    }
}

void crear_vector_dinamico_char(char*** vector, char* informacion[]) {
    int cantidad_de_elementos = string_array_size(informacion);
    *vector = (char **)malloc(sizeof(char *) * (cantidad_de_elementos + 1));

    if (*vector == NULL) {
        perror("Error al asignar memoria");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; informacion[i] != NULL; i++) {
        (*vector)[i] = strdup(informacion[i]);
        if ((*vector)[i] == NULL) {
            perror("Error al duplicar cadena");
            // Liberar memoria previamente asignada en caso de error
            for (size_t j = 0; j < i; j++) {
                free((*vector)[j]);
            }
            free(*vector);
            exit(EXIT_FAILURE);
        }
    }
    (*vector)[cantidad_de_elementos] = NULL;
}

void crear_vector_dinamico_int(int** vector, char* informacion[]) {
    int cantidad_de_elementos = string_array_size(informacion);
    *vector = (int *)malloc(sizeof(int) * cantidad_de_elementos);

    if (*vector == NULL) {
        perror("Error al asignar memoria");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < cantidad_de_elementos; i++) {
        (*vector)[i] = atoi(informacion[i]);
    }
}

void recorrer_vector_char(char* vector[]) 
{
    for(int i = 0; vector[i] != NULL; i++)
        printf("%s\n", vector[i]);
}

char* eliminar_espacios(char* cadena) 
{
    char* aux = cadena;
    char* aux2;

    // Elimina los espacios en blanco al principio de la cadena
    while (*aux && isspace(*aux))
        aux++;

    if (*aux == '\0') {
        // Si la cadena queda vacía después de eliminar espacios en blanco,
        // se retorna una cadena vacía
        return aux;
    }

    aux2 = aux + strlen(aux) - 1;

    // Elimina los espacios en blanco al final de la cadena
    while (aux2 > aux && isspace(*aux2))
        aux2--;

    // Coloca el carácter nulo '\0' después del último carácter no espacio en blanco
    *(++aux2) = '\0';

    // Retorna la dirección del primer carácter no espacio en blanco
    return aux;
}