#include "io-archivos.h"

FILE *abrir_archivo_metadata(t_interfaz *interfaz, char *name_file, char *modo_de_apertura) {
    char *path = get_path_archivo(interfaz, name_file);
    FILE *archivo = abrir_archivo(path, modo_de_apertura);

    // Liberar la memoria del path:
    free(path); 
    
    return archivo;
}

FILE *abrir_archivo_bitmap(t_interfaz *interfaz, char *modo_de_apertura) {
    int tamanio_archivo = interfaz->config->BLOCK_COUNT / 8;
    FILE *archivo = persistir_archivo(interfaz, "bitmap.bin", modo_de_apertura, tamanio_archivo);

    return archivo;
}

void *mapear_archivo_bloques(t_interfaz *interfaz, char *modo_de_apertura) {

    // Obtenemos el archivo de bloques:
    int tamanio_archivo = get_block_size(interfaz) * get_block_count(interfaz);
    archivo_bloque = persistir_archivo(interfaz, "bloques.bin", modo_de_apertura, tamanio_archivo);

    // Mapeamos el archivo:
    void *region_bloques_en_memoria = mmap(NULL, tamanio_archivo, PROT_READ | PROT_WRITE, MAP_SHARED, fileno(archivo_bloque), 0);

    // Verifico que se haya mapeado correctamente:
    if(region_bloques == MAP_FAILED) {
        log_info(logger, "No se pudo mapear el archivo bloques");
        exit(1);
    }

    return region_bloques_en_memoria;
}

FILE *persistir_archivo(t_interfaz *interfaz, char *name_file, char *modo_de_apertura, int tamanio_archivo) {
    char *path = get_path_archivo(interfaz, name_file);
    FILE *archivo = abrir_archivo(path, modo_de_apertura);

    if (archivo && strcmp(modo_de_apertura, "wb+") == 0) {
        ftruncate(fileno(archivo), tamanio_archivo);
    }

    // Liberar la memoria del path:
    free(path); 

    return archivo;
}

FILE *abrir_archivo(char *path, char *modo_de_apertura) {
    FILE *archivo = fopen(path, modo_de_apertura);

    if (!archivo) {
        log_error(logger, "No se pudo abrir el archivo %s", path);
        return NULL;
    }

    return archivo;
}

// Funcion para escribir en un archivo:
void escribir_contenido_en_bloques(void *bloques, t_queue *buffers) {

    // Inicializamos el desplazamiento:
    int desplazamiento = 0;
    
    // Iteramos sobre los buffers:
    while(!queue_is_empty(buffers)) {
        
        // Obtenemos el buffer y la cantidad de bytes a escribir:
        char *buffer = queue_pop(buffers);
        int *bytes_a_escribir = queue_pop(buffers);

        // Escribimos el contenido en los bloques:
        memcpy(bloques + desplazamiento, buffer, *bytes_a_escribir);
        desplazamiento += *bytes_a_escribir;

        // Liberamos la memoria utilizada:
        free(buffer);
        free(bytes_a_escribir);
    }

    // Liberamos la memoria utilizada:
    queue_destroy(buffers);
}

// Funcion para liberar region de memoria mapeada:
void liberar_region_memoria_bloques(void *region, t_interfaz *interfaz) {
    int block_size = get_block_size(interfaz);
    int block_count = get_block_count(interfaz);
    int tamanio = block_size * block_count;

    munmap(region, tamanio);
}