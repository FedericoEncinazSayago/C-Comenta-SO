#include "io-operaciones-dialfs.h"

void operacion_create_file(t_interfaz *interfaz, t_bitarray *bitmap, t_list *argumentos, t_list *archivos_ya_abiertos) {

    // Obtenemos los argumentos:
    int *pid_proceso = list_remove(argumentos, 0);
    char *nombre_archivo = list_remove(argumentos, 0);

    // Logeamos la operación:
    log_info(logger, "PID: %i - Crear Archivo: %s", *pid_proceso, nombre_archivo);

    // Primero verificamos si el archivo ya se encuentra abierto:
    if(ya_esta_abierto(archivos_ya_abiertos, nombre_archivo)) {
        log_error(logger, "El archivo ya se encuentra abierto");

        // Liberamos la memoria utilizada:
        free(nombre_archivo);

        return;
    }

    // Creamos el archivo:
    t_config *archivo_metadata = abrir_archivo_metadata_config(interfaz, nombre_archivo, "w");
    int bloque_inicial = obtener_bloque_libre(bitmap, interfaz);

    if(bloque_inicial == -1) {
        log_error(logger, "No hay bloques libres para el archivo");

        return;
    }

    // Seteamos el bloque como ocupado:
    set_bloque_ocupado(bitmap, bloque_inicial);

    // Seteamos los datos del archivo:
    set_bloque_inicial_en_archivo_metadata(archivo_metadata, bloque_inicial);
    set_tamanio_archivo_en_archivo_metadata(archivo_metadata, 0);
    set_nuevo_archivo_abierto(archivos_ya_abiertos, nombre_archivo, archivo_metadata);
    set_archivo_metada_en_fs(archivo_metadata);

    // Logeamos la operación:
    log_info(logger, "Se creo el archivo %s", nombre_archivo);
    log_info(logger, "Bloque inicial: %d", bloque_inicial);

    // Liberamos la memoria utilizada:
    free(pid_proceso);
}

void operacion_write_file(t_interfaz *interfaz, void *bloques, t_list *argumentos, t_list *archivos_ya_abiertos) {

    // Obtenemos los argumentos:
    int *pid_proceso = list_remove(argumentos, 0);
    char *nombre_archivo = list_remove(argumentos, 0);
    int *offset  = list_remove(argumentos, 0);
    t_list *direcciones_fisicas = list_remove(argumentos, 0);
    int cantidad_de_bytes = get_total_de_bytes(direcciones_fisicas);

    // Logeamos la operación:
    log_info(logger, "PID: %i - Escribir Archivo: %s - Tamaño a Escribir: %i - Puntero Archivo: %i", *pid_proceso, nombre_archivo, cantidad_de_bytes, *offset);

    // Verificamos si el archivo se encuentra abierto:
    t_archivo_abierto *archivo_abierto = obtener_archivo_abierto(archivos_ya_abiertos, nombre_archivo);

    if(!archivo_abierto) {
        log_error(logger, "El archivo no se encuentra abierto");

        // Liberamos la memoria utilizada:
        free(pid_proceso);
        free(offset);
        free(nombre_archivo);

        // Liberamos las direcciones fisicas:
        list_destroy_and_destroy_elements(direcciones_fisicas, (void *) liberar_direccion_fisica);

        return;
    }

    // Verificamos si el tamaño supera el tamaño del archivo:
    if(cantidad_de_bytes + *offset > get_tamanio_archivo(get_archivo_metadata(archivo_abierto))) {
        log_error(logger, "No se puede escribir en el archivo, el tamanio a escribir supera el tamanio del archivo");

        // Liberamos la memoria utilizada:
        free(pid_proceso);
        free(offset);
        free(nombre_archivo);

        // Liberamos las direcciones fisicas:
        list_destroy_and_destroy_elements(direcciones_fisicas, (void *) liberar_direccion_fisica);

        return;
    }

    // Obtenemos los datos necesarios:
    t_config *archivo_metadata = get_archivo_metadata(archivo_abierto);
    int bloque_inicial = get_bloque_inicial(archivo_metadata);
    char *contenido = rcv_contenido_a_mostrar(interfaz, direcciones_fisicas, *pid_proceso);

    // Escribimos el contenido en el archivo:
    memcpy(bloques + bloque_inicial * get_block_size(interfaz) + *offset, contenido, cantidad_de_bytes);
    msync(bloques, get_block_size(interfaz) * get_block_count(interfaz), MS_SYNC);
    contenido[cantidad_de_bytes] = '\0';

    // Logeamos la operación:
    log_info(logger, "Se escribio en el archivo %s", nombre_archivo);
    log_info(logger, "Contenido: %s", contenido);

    // Liberamos la memoria utilizada:
    free(contenido);
    free(pid_proceso);
    free(offset);
}

void operacion_read_file(t_interfaz *interfaz, void *bloques, t_list *argumentos, t_list *archivos_ya_abiertos) {

    // Obtenemos los argumentos:
    int *pid_proceso = list_remove(argumentos, 0);
    char *nombre_archivo = list_remove(argumentos, 0);
    int *offset = list_remove(argumentos, 0);
    t_list *direcciones_fisicas = list_remove(argumentos, 0);
    int cantidad_de_bytes = get_total_de_bytes(direcciones_fisicas);

    // Logeamos la operación:
    log_info(logger, "PID: %i - Leer Archivo: %s - Tamaño a Leer: %i - Puntero Archivo: %i", *pid_proceso, nombre_archivo, cantidad_de_bytes, *offset);

    // Verificamos si el archivo se encuentra abierto:
    t_archivo_abierto *archivo_abierto = obtener_archivo_abierto(archivos_ya_abiertos, nombre_archivo);

    if(!archivo_abierto) {
        log_error(logger, "El archivo no se encuentra abierto");

        // Liberamos la memoria utilizada:
        free(pid_proceso);
        free(offset);
        free(nombre_archivo);

        // Liberamos las direcciones fisicas:
        list_destroy_and_destroy_elements(direcciones_fisicas, (void *) liberar_direccion_fisica);

        return;
    }

    // Verificamos si el tamaño supera el tamaño del archivo:
    if(cantidad_de_bytes + *offset > get_tamanio_archivo(get_archivo_metadata(archivo_abierto))) {
        log_error(logger, "No se puede leer el archivo, el tamanio a leer supera el tamanio del archivo");

        // Liberamos la memoria utilizada:
        free(pid_proceso);
        free(offset);
        free(nombre_archivo);

        // Liberamos las direcciones fisicas:
        list_destroy_and_destroy_elements(direcciones_fisicas, (void *) liberar_direccion_fisica);

        return;
    }

    t_config *archivo_metadata = get_archivo_metadata(archivo_abierto);

    // Obtenemos los datos necesarios:
    int bloque_inicial = get_bloque_inicial(archivo_metadata);
    int bytes_a_escribir = get_total_de_bytes(direcciones_fisicas);
    char *contenido = malloc(bytes_a_escribir + 1);

    // Leemos el contenido del archivo:
    memcpy(contenido, bloques + bloque_inicial * get_block_size(interfaz) + *offset, bytes_a_escribir);
    contenido[bytes_a_escribir] = '\0';

    // Enviamos los bytes leidos a memoria:
    send_bytes_a_leer(interfaz, *pid_proceso, direcciones_fisicas, contenido, bytes_a_escribir);

    // Logeamos la operación:
    log_info(logger, "Se leyo el archivo %s", nombre_archivo);
    log_info(logger, "Contenido: %s", contenido);

    // Liberamos la memoria utilizada:
    free(contenido);
    free(pid_proceso);
    free(offset);
}

void operacion_delete_file(t_interfaz *interfaz, t_bitarray *bitmap, t_list *argumentos, t_list *archivos_ya_abiertos) {

    // Obtenemos los argumentos:
    int *pid_proceso = list_remove(argumentos, 0);
    char *nombre_archivo = list_remove(argumentos, 0);

    // Logeamos la operación:
    log_info(logger, "PID: %i - Eliminar Archivo: %s", *pid_proceso, nombre_archivo);

    // Verificamos si el archivo se encuentra abierto:
    t_archivo_abierto *archivo_abierto = obtener_archivo_abierto(archivos_ya_abiertos, nombre_archivo);

    if(!archivo_abierto) {
        log_error(logger, "El archivo no se encuentra abierto");

        // Liberamos la memoria utilizada:
        free(nombre_archivo);

        return;
    }

    t_config *archivo_metadata = get_archivo_metadata(archivo_abierto);

    // Obtenemos los datos necesarios:
    int bloque_inicial = get_bloque_inicial(archivo_metadata);
    int tamanio_archivo = get_tamanio_archivo(archivo_metadata);
    int bloques_necesarios = calcular_cantidad_bloques_asignados(interfaz, tamanio_archivo);

    // Liberamos los datos asignados:
    liberar_bloques_asignados(bitmap, bloque_inicial, bloques_necesarios);
    cerrar_archivo_abierto(archivos_ya_abiertos, nombre_archivo);
    cerrar_archivo_metadata(archivo_metadata);
    eliminar_archivo_en_fs(interfaz, nombre_archivo);

    // Logeamos la operación:
    log_info(logger, "Se elimino el archivo %s", nombre_archivo);
    
    // Liberamos la memoria utilizada:
    free(pid_proceso);
}

void operacion_truncate_file(t_interfaz *interfaz, void *bloques, t_bitarray *bitmap, t_list *argumentos, t_list *archivos_ya_abiertos) {

    // Obtenemos los argumentos:
    int *pid_proceso = list_remove(argumentos, 0);
    char *nombre_archivo = list_remove(argumentos, 0);
    int *nuevo_tamanio = list_remove(argumentos, 0);

    // Logeamos la operación:
    log_info(logger, "PID: %i - Truncar Archivo: %s - Tamanio: %i", *pid_proceso, nombre_archivo, *nuevo_tamanio);

    // Verificamos si el archivo se encuentra abierto:
    t_archivo_abierto *archivo_abierto = obtener_archivo_abierto(archivos_ya_abiertos, nombre_archivo);

    if(!archivo_abierto) {
        log_error(logger, "El archivo no se encuentra abierto");

        // Liberamos la memoria utilizada:
        free(nombre_archivo);
        free(pid_proceso);
        free(nuevo_tamanio);

        return;
    }

    // Obtenemos los datos necesarios:
    t_config *archivo_metadata = get_archivo_metadata(archivo_abierto);
    int tamanio_actual = get_tamanio_archivo(archivo_metadata);
    int bloque_inicial = get_bloque_inicial(archivo_metadata);

    // Truncamos el archivo:
    if(*nuevo_tamanio  < tamanio_actual){
        log_info(logger, "Se no se puede truncar el archivo a un tamanio menor al actual");

        return;
    }

    int bloques_necesarios = get_bloques_necesarios(interfaz, *nuevo_tamanio) - 1;
    int tam_resultante = *nuevo_tamanio  + tamanio_actual;

    log_info(logger, "Cantidad de bloques necesarios: %d", bloques_necesarios);

    if(bloques_necesarios > 1) {
        if(!hay_suficientes_bloques_libres(bitmap, interfaz, bloques_necesarios)) {
            log_error(logger, "No hay bloques libres suficientes para truncar el archivo");
            
            return;
        }

        int bloque_final = calcular_bloque_final(interfaz, bloque_inicial, tamanio_actual);

        if(no_hay_bloques_contiguos_libres(bitmap, bloque_final, bloques_necesarios)) {
            log_info(logger, "PID: %i - Inicio Compactación.", *pid_proceso);
            compactar_fs(interfaz, bloques, bitmap, archivos_ya_abiertos, archivo_metadata, bloques_necesarios, bloque_inicial, tam_resultante);
            retardo_compactacion(interfaz);
            log_info(logger, "PID: %i - Fin Compactación.", *pid_proceso);
        } else {
            log_info(logger, "Hay bloques libres suficientes para truncar el archivo");
            set_bloques_como_ocupados(bitmap, bloque_final, bloques_necesarios);
        }
    } 

    int cantidad_de_bloques_anterior = calcular_cantidad_bloques_asignados(interfaz, tamanio_actual);

    // Logeamos la operación:
    log_info(logger, "Se truncó el archivo %s", nombre_archivo);
    log_info(logger, "Nuevo tamanio: %d", tam_resultante);
    log_info(logger, "Cantidad de nuevos bloques asignados: %d", bloques_necesarios);
    log_info(logger, "Cantidad de bloques actuales: %d", cantidad_de_bloques_anterior + bloques_necesarios);

    // Seteamos los datos del archivo:
    set_tamanio_archivo_en_archivo_metadata(archivo_metadata, tam_resultante);
    set_archivo_metada_en_fs(archivo_metadata);

    // Liberamos la memoria utilizada:
    free(pid_proceso);
    free(nuevo_tamanio);
}