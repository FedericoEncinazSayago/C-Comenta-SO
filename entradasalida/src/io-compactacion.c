#include "io-compactacion.h"

void compactar_fs(t_interfaz  *interfaz, void *bloques, t_bitarray *bitmap, t_list *archivos_ya_abiertos, t_config *archivo_metadata, int cantidad_bloques_asignados_a_archivo_compactar, int bloque_inicial_archivo_a_compactar, int tam_nuevo_archivo_a_compactar) {

    // Ordenamos los archivos abiertos por bloque inicial:
    int cantidad_archivos_abiertos = list_size(archivos_ya_abiertos);
    list_sort(archivos_ya_abiertos, (void *) comparar_bloque_inicial);

    // Creamos una cola para los buffers:
    t_queue *buffers = queue_create();

    // Variables auxiliares:
    int bloque_final_anterior;
    int nuevo_bloque_libre;
    int bloque_inicial_archivo_actual;
    int tamanio_archivo_archivo_actual;
    int cantidad_bloques_asignados;
    int *tam_buffer_archivo_a_compactar;
    char *buffer_archivo_a_compactar;

    // Recorremos los archivos abiertos:
    for(int i =  0; i < cantidad_archivos_abiertos; i++) {

        // Obtenemos el archivo actual:
        t_archivo_abierto *archivo_abierto_actual = list_get(archivos_ya_abiertos, i);
        t_config *archivo_metadata_archivo_actual = get_archivo_metadata(archivo_abierto_actual);

        // Obtenemos los datos necesarios:
        bloque_inicial_archivo_actual = get_bloque_inicial(archivo_metadata_archivo_actual);
        tamanio_archivo_archivo_actual = get_tamanio_archivo(archivo_metadata_archivo_actual);
        nuevo_bloque_libre = obtener_bloque_libre(bitmap, interfaz);

        // Obtenemos los datos del archivo en el archivo bloques:
        char *buffer = malloc(tamanio_archivo_archivo_actual);
        memcpy(buffer, bloques + bloque_inicial_archivo_actual * get_block_size(interfaz), tamanio_archivo_archivo_actual);

        // Obtenemos los datos del archivo dentro de bloques:
        if(bloque_inicial_archivo_actual != bloque_inicial_archivo_a_compactar) {
            int *tam_bufer = malloc(sizeof(int));
            *tam_bufer = tamanio_archivo_archivo_actual;

            queue_push(buffers, buffer);
            queue_push(buffers, tam_bufer);
        } else {
            buffer_archivo_a_compactar = buffer;
            tam_buffer_archivo_a_compactar = malloc(sizeof(int));
            *tam_buffer_archivo_a_compactar = tamanio_archivo_archivo_actual;
        }

        // Verificamos si el archivo actual es el archivo a compactar o si es un archivo que se puede mover:
        if(nuevo_bloque_libre < bloque_inicial_archivo_actual || bloque_inicial_archivo_actual == bloque_inicial_archivo_a_compactar) {

            // Liberamos los bloques asignados al archivo actual:
            cantidad_bloques_asignados = calcular_cantidad_bloques_asignados(interfaz, tamanio_archivo_archivo_actual);
            liberar_bloques_asignados(bitmap, bloque_inicial_archivo_actual, cantidad_bloques_asignados);
        
            if(bloque_inicial_archivo_actual != bloque_inicial_archivo_a_compactar) {
                bloque_final_anterior = calcular_bloque_final(interfaz, nuevo_bloque_libre, tamanio_archivo_archivo_actual);

                // Asignamos los bloques al archivo actual:
                set_bloques_como_ocupados_desde(bitmap, nuevo_bloque_libre, cantidad_bloques_asignados);
                set_bloque_inicial_en_archivo_metadata(archivo_metadata_archivo_actual, nuevo_bloque_libre);
                set_archivo_metada_en_fs(archivo_metadata_archivo_actual);
            }
        }
    }

    queue_push(buffers, buffer_archivo_a_compactar);
    queue_push(buffers, tam_buffer_archivo_a_compactar);

    // Calculamos los bloques necesarios para el archivo a compactar:
    nuevo_bloque_libre = bloque_final_anterior + 1;
    cantidad_bloques_asignados = calcular_cantidad_bloques_asignados(interfaz, tam_nuevo_archivo_a_compactar);

    // Asignamos los bloques al archivo a compactar:
    set_bloques_como_ocupados_desde(bitmap, nuevo_bloque_libre, cantidad_bloques_asignados);
    set_bloque_inicial_en_archivo_metadata(archivo_metadata, nuevo_bloque_libre);

    // Cambiamos el contenido de los bloques:
    escribir_contenido_en_bloques(bloques, buffers);
}