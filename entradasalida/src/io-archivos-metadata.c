#include "io-archivos-metadata.h"

t_config *abrir_archivo_metadata_config(t_interfaz *interfaz, char *name_file, char *modo_de_apertura) {
    char *path = get_path_archivo(interfaz, name_file);

    // Ya esta creado el archivo, solo lo abro:
    if(strcmp(modo_de_apertura, "r") == 0)
        return config_create(path);

    // Creo el archivo y lo abro:
    FILE *archivo = abrir_archivo(path, modo_de_apertura);
    t_config *config = config_create(path);

    // Liberamos memoria:
    fclose(archivo);
    free(path);

    return config;
}

// Funciones para validar los archivos:
int es_un_archivo_valido(t_config *archivo_metadata) {

    // Verificamos que el archivo tenga las propiedades necesarias:
    char *propiedades[] = {"BLOQUE_INICIAL", "TAMANIO_ARCHIVO", NULL};

    // Verificamos que el archivo tenga todas las propiedades necesarias:
    return tiene_todas_las_configuraciones(archivo_metadata, propiedades);
}

// Funciones para setear los datos del archivo metadata:
void set_bloque_inicial_en_archivo_metadata(t_config *archivo_metadata, int bloque_inicial) {
    char *bloque_inicial_string = string_itoa(bloque_inicial);

    config_set_value(archivo_metadata, "BLOQUE_INICIAL", bloque_inicial_string);
    free(bloque_inicial_string);
}

void set_tamanio_archivo_en_archivo_metadata(t_config *archivo_metadata, int tamanio_archivo) {
    char *tamanio_archivo_string = string_itoa(tamanio_archivo);

    config_set_value(archivo_metadata, "TAMANIO_ARCHIVO", tamanio_archivo_string);
    free(tamanio_archivo_string);
}

void set_archivo_metada_en_fs(t_config *archivo_metadata) {
    config_save(archivo_metadata);
}

// Funciones para gettear los datos del archivo metadata:
int get_bloque_inicial(t_config *archivo_metadata) {
    return config_get_int_value(archivo_metadata, "BLOQUE_INICIAL");
}

int get_tamanio_archivo(t_config *archivo_metadata) {
    return config_get_int_value(archivo_metadata, "TAMANIO_ARCHIVO");
}

// Funciones para cerrar el archivo metadata:
void cerrar_archivo_metadata(t_config *archivo_metadata) {
    config_destroy(archivo_metadata);
}