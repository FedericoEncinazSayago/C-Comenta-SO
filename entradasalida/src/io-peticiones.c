#include "io-peticiones.h"

void interfaz_recibir_peticiones(t_interfaz * interfaz) {
    if(interfaz->tipo == GENERICA)
        ejecutar_operacion_generica(interfaz);
    else if(interfaz->tipo == STDIN)
        ejecutar_operacion_stdin(interfaz);
    else if(interfaz->tipo == STDOUT)
        ejecutar_operacion_stdout(interfaz);
    else if(interfaz->tipo == DIALFS)
        ejecutar_operaciones_dialFS(interfaz);
    else
        log_error(logger, "Tipo de interfaz no soportado");
}

void ejecutar_operacion_generica(t_interfaz * interfaz) {
    while(1) {
        // Obtenemos los argumentos:
        t_list *argumentos = recibir_argumentos(interfaz, get_socket_kernel(interfaz));

        // Obtenemos los argumentos necesarios:
        int *pid_proceso = list_remove(argumentos, 0);
        int *tipo_operacion = list_remove(argumentos, 0);
        int *tiempo_espera = list_remove(argumentos, 0);
        int tiempo_unidad = get_tiempo_unidad(interfaz);
        char *operacion = string_new();
        string_append(&operacion, get_nombre_operacion(*tipo_operacion));

        // Logeamos la operación:
        log_info(logger, "PID: %d - Operacion: %s", *pid_proceso, operacion);
        log_info(logger, "Tiempo de espera de milisegundos: %d", *tiempo_espera * tiempo_unidad);

        // Esperamos el tiempo de espera:
        usleep(*tiempo_espera * tiempo_unidad * 1000);

        // Enviamos la respuesta al kernel:
        send_respuesta_a_kernel(1, interfaz);

        // Liberamos la memoria:
        free(pid_proceso);
        free(tipo_operacion);
        free(tiempo_espera);
        free(operacion);

        // Liberamos la lista de argumentos:
        list_destroy(argumentos);

        // log_fede(logger2, "Se ejecuto una operacion generica");
    }
}

void ejecutar_operacion_stdin(t_interfaz *interfaz) {

    // Inicializamos las variables:
    char *input = NULL;

    while(1) {
        // Obtenemos los argumentos:
        t_list *argumentos = recibir_argumentos(interfaz, get_socket_kernel(interfaz));

        // Obtenemos los argumentos necesarios:
        int *pid_proceso = list_remove(argumentos, 0);
        int *tipo_operacion = list_remove(argumentos, 0);
        t_list *direcciones = list_remove(argumentos, 0);
        char *operacion = string_new();
        string_append(&operacion, get_nombre_operacion(*tipo_operacion));

        // Obtenemos la cantidad de bytes a escribir:
        int bytes_a_escribir = get_total_de_bytes(direcciones);

        // Logeamos la operación:
        log_info(logger, "PID: %d - Operacion: %s", *pid_proceso, operacion);
        log_info(logger, "Bytes a escribir: %i", bytes_a_escribir);

        if(bytes_a_escribir != 0) {
        
            // Leemos el input:
            input = readline("Ingrese un valor: ");

            if(input == NULL) {
                log_error(logger, "Error al leer el input");
                exit(EXIT_FAILURE);
            }
            
            while(strlen(input) > bytes_a_escribir || strlen(input) < bytes_a_escribir) {
                log_error(logger, "El input ingresado es mayor al tamaño de bytes a leer");
                input = readline("Ingrese un valor: ");
            }

            int bytes_leidos = strlen(input);

            // Enviamos los bytes a escribir a memoria:
            send_bytes_a_leer(interfaz, *pid_proceso, direcciones, input, bytes_leidos);
        }

        send_respuesta_a_kernel(1, interfaz);

        // Liberamos la memoria:
        free(pid_proceso);
        free(tipo_operacion);
        free(operacion);
        free(input);

        // Liberamos la lista de argumentos
        list_destroy(argumentos);

        // log_fede(logger2, "Se ejecuto una operacion stdin");
    }
}

void ejecutar_operacion_stdout(t_interfaz *interfaz) {
    while(1) {
        // Obtenemos los argumentos:
        t_list *argumentos = recibir_argumentos(interfaz, get_socket_kernel(interfaz));

        // Obtenemos los argumentos necesarios:
        int *pid_proceso = list_remove(argumentos, 0);
        int *tipo_operacion = list_remove(argumentos, 0);
        t_list *direcciones = list_remove(argumentos, 0);
        int cantidad_de_bytes = get_total_de_bytes(direcciones);
        char *operacion = get_nombre_operacion(*tipo_operacion);
        
        // Logeamos la operación:
        log_info(logger, "PID: %d - Operacion: %s", *pid_proceso, operacion);

        // Solicitamos los bytes a memoria:
        char *contenido_a_mostrar = rcv_contenido_a_mostrar(interfaz, direcciones, *pid_proceso);
        contenido_a_mostrar[cantidad_de_bytes] = '\0';

        // Mostramos el contenido:
        log_info(logger, "PID: %i - Contenido leido: %s", *pid_proceso, contenido_a_mostrar);

        // Enviamos la respuesta:
        send_respuesta_a_kernel(1, interfaz);

        // Liberamos la memoria:
        free(contenido_a_mostrar);
        free(pid_proceso);
        free(tipo_operacion);

        // Liberamos la lista de argumentos:
        list_destroy(argumentos);
    }
}

void ejecutar_operaciones_dialFS(t_interfaz *interfaz) {

    // Inicializamos las variables:
    int socket_kernel = get_socket_kernel(interfaz);

    // Inicializamos las estructuras necesarias:
    archivos_ya_abiertos = obtener_archivos_ya_abiertos(interfaz);
    int size = list_size(archivos_ya_abiertos);

    // Obtenemos el modo de apertura:
    char *modo_de_apertura = string_new();
    string_append(&modo_de_apertura, get_modo_de_apertura(size));

    // Abrimos los archivos necesarios:
    region_bloques = mapear_archivo_bloques(interfaz, modo_de_apertura);
    bitmap = crear_bitmap(interfaz, modo_de_apertura);

    // Liberamos la memoria:
    free(modo_de_apertura);

    while(1) {
        tipo_operacion operacion = recibir_operacion(socket_kernel);
        t_list *argumentos = recibir_argumentos_para_dial(interfaz, operacion);

        // Obtenemos los pid y el tipo de operacion:
        int *pid_proceso = list_get(argumentos, 0);
        char *operacion_string = string_new();
        string_append(&operacion_string, get_nombre_operacion(operacion));

        // Logeamos la operación:
        log_info(logger, "PID: %d - Operacion: %s", *pid_proceso, operacion_string);

        switch(operacion) {
            case IO_FS_CREATE_INT:
                operacion_create_file(interfaz, bitmap, argumentos, archivos_ya_abiertos);
                aplicar_unidad_trabajo(interfaz);
                send_respuesta_a_kernel(1, interfaz);
                break;
            case IO_FS_READ_INT:
                operacion_read_file(interfaz, region_bloques, argumentos, archivos_ya_abiertos);
                aplicar_unidad_trabajo(interfaz);
                send_respuesta_a_kernel(1, interfaz);
                break;
            case IO_FS_DELETE_INT:
                operacion_delete_file(interfaz, bitmap, argumentos, archivos_ya_abiertos);
                aplicar_unidad_trabajo(interfaz);
                send_respuesta_a_kernel(1, interfaz);
                break;
            case IO_FS_WRITE_INT:
                operacion_write_file(interfaz, region_bloques, argumentos, archivos_ya_abiertos);
                aplicar_unidad_trabajo(interfaz);
                send_respuesta_a_kernel(1, interfaz);
                break;
            case IO_FS_TRUNCATE_INT:
                operacion_truncate_file(interfaz, region_bloques, bitmap, argumentos, archivos_ya_abiertos);
                aplicar_unidad_trabajo(interfaz);
                send_respuesta_a_kernel(1, interfaz);
                break;
            default:
                log_error(logger, "Operacion desconocida");
        }

        // Liberamos la memoria:
        free(operacion_string);

        // Liberamos la lista de argumentos:
        list_destroy(argumentos);
    }
}