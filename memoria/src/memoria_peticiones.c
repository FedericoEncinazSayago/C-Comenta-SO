#include "memoria_peticiones.h"

sem_t enviar_instruccion;
sem_t sem_lectura_archivo;

void inicializar_semaforo() {
    sem_init(&enviar_instruccion, 0, 0);
    sem_init(&sem_lectura_archivo, 0, 1);
}

void* escuchar_peticiones(void* args){
    int socket_cliente = *(int*) args;
    inicializar_semaforo(); 

    while (1){   
        int cod_op = recibir_operacion(socket_cliente);
        //log_warning(logger,"cod op = %i/FIN_DE_OP_CODE",cod_op);
        switch (cod_op){
        case HANDSHAKE:
            recibir_handshake(socket_cliente);
            break;
        case MENSAJE:
            recibir_mensaje(socket_cliente);
            //enviar_mensaje("MEMORIA -> CPU", socket_cliente);
            break;
        case HANDSHAKE_PAGINA:
            //log_warning(logger,"HANDSHAKE_PAGINA");
            recibir_handshake(socket_cliente);
            handshake_desde_memoria(socket_cliente);
            break;
        case INICIAR_PROCESO:
            // sem_wait(&sem_lectura_archivo);
            log_info(logger, "Iniciando proceso");
            retardo_pedido(config_memoria -> retardo_respuesta);
            leer_archivoPseudo(socket_cliente);
            enviar_respuesta_a_kernel(socket_cliente);
            // sem_wait(&sem_lectura_archivo);
            break;
        case FINALIZAR_PROCESO:
            retardo_pedido(config_memoria -> retardo_respuesta);
            terminar_proceso(socket_cliente);
            break;
        case INSTRUCCION: 
            retardo_pedido(config_memoria -> retardo_respuesta);
            log_facu(logger2,"Enviando instruccion a CPU");
            enviar_instruccion_a_cpu(socket_cliente);
            break;
        case ACCEDER_TABLA_PAGINAS: 
            retardo_pedido(config_memoria -> retardo_respuesta);
            obtener_marco(socket_cliente);
            break;
        case MODIFICAR_TAMAÑO_MEMORIA:
            retardo_pedido(config_memoria -> retardo_respuesta);
            resize_proceso(socket_cliente);
            break;
        case ACCESO_A_LECTURA:
            retardo_pedido(config_memoria -> retardo_respuesta);
            acceso_lectura(socket_cliente);
            break;
        case ACCESO_A_ESCRITURA:
            retardo_pedido(config_memoria -> retardo_respuesta);
            acceso_escritura(socket_cliente);
            break;
        case -1:
            log_info(logger,"Se desconecto el cliente");
            free(args);
            return;
        default:
            log_error(logger, "Operacion desconocida");
        }
    }
}

void handshake_desde_memoria(int socket_cliente) {
    t_paquete* paquete = crear_paquete(HANDSHAKE_PAGINA);
    int tam_pagina = config_memoria ->tam_pagina;
    // send(socket_cliente, &tam_pagina, sizeof(int), 0);    
    agregar_a_paquete(paquete, &tam_pagina, sizeof(int));
    enviar_paquete(paquete, socket_cliente);
    eliminar_paquete(paquete);
}



void enviar_respuesta_a_kernel(int socket_cliente) {
    int respuesta = 1;

    send(socket_cliente, &respuesta, sizeof(int), MSG_WAITALL);
}



//Creacion / destruccion de Tabla de Paginas: "PID:" <PID> "- Tamanio:" <CANTIDAD_PAGINAS>
void crear_proceso(int pid){
    //Creamos una nueva tabla de páginas
    t_list* tabla_de_paginas = list_create();

    // Agregar la tabla de páginas al diccionario
    char* pid_string=string_itoa(pid);

    dictionary_put(diccionario_tabla_de_paginas_porPID, pid_string, tabla_de_paginas);
    free(pid_string);

    log_mati(logger2, "Creacion de Tabla de Paginas: PID: %i - Tamanio: %i", pid, list_size(tabla_de_paginas));
}



void terminar_proceso(int socket_cliente){
    int size;
    void* buffer = recibir_buffer(&size, socket_cliente);
    int pid;
    memcpy(&pid, buffer, sizeof(int));

    char* pid_string = string_itoa(pid);

    // Obtener la tabla de páginas del diccionario
    t_list* tabla_de_paginas = dictionary_remove(diccionario_tabla_de_paginas_porPID, pid_string);
    if (!tabla_de_paginas) {
        log_error(logger, "Error: No se encontró la tabla de páginas para el PID %d\n", pid);
        free(buffer);
        exit(-1);
    }

    int cantidad_de_paginas = list_size(tabla_de_paginas);//averiguamos cuantas paginas tiene
    for(int i = 0; i < cantidad_de_paginas; i++){ 
        t_tabla_de_paginas* pag_a_eliminar = list_remove(tabla_de_paginas, 0); //obtengo la pagina [i]
        if(pag_a_eliminar == NULL){
            log_error(logger, "No se encontró la pagina %i", i);
            exit(-1);
        }
        bitarray_clean_bit(bitmap, pag_a_eliminar->marco); //marco en el bitmap la pagina [i] como "libre"
        free(pag_a_eliminar); //libero la pagina [i]
    }
    list_destroy(tabla_de_paginas); //libero la tabla de paginas
    dictionary_remove_and_destroy(diccionario_de_instrucciones_porPID, pid_string, liberar_lista_instrucciones);    
    log_mati(logger2, "Destruccion de Tabla de Paginas: PID: %i - Tamanio: %i", pid, cantidad_de_paginas); // Log minimo y obligatorio
    free(pid_string);
    free(buffer);
}



//Ampliacion de Proceso: "PID:" <PID> "- Tamanio Actual:" <TAMANIO_ACTUAL> "- Tamanio a Ampliar:" <TAMANIO_A_AMPLIAR>
//Reduccion de Proceso: "PID:" <PID> "- Tamanio Actual:" <TAMANIO_ACTUAL> "- Tamanio a Reducir:" <TAMANIO_A_REDUCIR>
void resize_proceso(int socket_cliente) {

  	int size;
	void* buffer = recibir_buffer(&size, socket_cliente);
	int tamanio_bytes;
	int pid;
    memcpy(&pid, buffer, sizeof(int));
	memcpy(&tamanio_bytes, buffer + sizeof(int), sizeof(int));

    char* pid_string = string_itoa(pid);

    // Obtener la tabla de páginas del diccionario
    t_list* tabla_de_paginas = dictionary_get(diccionario_tabla_de_paginas_porPID, pid_string); //obtenemos la tabla de paginas del proceso que vamos a modificar
    free(pid_string);
    if (!tabla_de_paginas) {
        log_error(logger, "Error: No se encontró la tabla de páginas para el PID %d\n", pid);
        t_paquete* paquete = crear_paquete(OUT_OF_MEMORY);
        int tabla_de_pag_no_encontrado = -1;
        agregar_a_paquete(paquete, &tabla_de_pag_no_encontrado , sizeof(int));
        enviar_paquete(paquete, socket_cliente);
        eliminar_paquete(paquete);
    } // CHEQUEAR ESTO

    // Calcular el número de páginas necesarias
    int num_paginas = ceil((double)tamanio_bytes / config_memoria->tam_pagina); // La funcion ceil redondea hacia arriba
    int paginas_actuales = list_size(tabla_de_paginas); // Calculamos la cantidad de paginas que tiene la tabla
    log_mati(logger2, "El numero de pagians a ampliar es : %i", num_paginas);               
    if (num_paginas > paginas_actuales) { // Si la cantidad de paginas de la tabla actual son menos al calculo del resize -> se amplia 
    // Ajustar el tamaño de la tabla de páginas
    // log_fede(logger2, "PID: %d - Tamaño Actual: %d - Tamaño a Ampliar: %d ", pid, num_paginas*4 ,tamanio_bytes);

        log_mati(logger2, "PID: %d - Tamaño Actual: %d - Tamaño a Ampliar: %d ", pid, paginas_actuales*4 ,tamanio_bytes); // Log minimo y obligatorio
        for (int i = paginas_actuales; i < num_paginas; i++) { //buscamos si hay suficientes marco libre
            int marco_libre = obtener_marco_libre(bitmap);
        // log_mati(logger2, "Agregando pagina... : %i", i);
        
            if (marco_libre == -1) {    // si no se encuentra un marco libre, tira OUT_OF_MEMORY y muere
                log_error(logger, "Out of memory (faltan marcos).\n");
            
                t_paquete* paquete = crear_paquete(OUT_OF_MEMORY);
                int fuera_de_memoria = -1;
                agregar_a_paquete(paquete, &fuera_de_memoria , sizeof(int));
                enviar_paquete(paquete, socket_cliente);
                eliminar_paquete(paquete);
                free(buffer);
                return;
            }
            //guardamos espacio de memoria para la nueva pagina
            t_tabla_de_paginas* nueva_pagina = malloc(sizeof(t_tabla_de_paginas));
            nueva_pagina->nro_pagina = i;
            nueva_pagina->marco = marco_libre;
            nueva_pagina->bit_validez = 1;

            list_add(tabla_de_paginas, nueva_pagina);
            log_mati(logger2, "Se agrego la pagina %i", nueva_pagina->nro_pagina);
        
        }

    } else if (num_paginas < paginas_actuales) {
        // Reducir el tamaño del proceso
        log_mati(logger2, "PID: %d - Tamaño Actual: %d - Tamaño a Reducir: %d ", pid, paginas_actuales*4 ,tamanio_bytes); // Log minimo y obligatorio
        for (int i = paginas_actuales - 1; i >= num_paginas; i--) {
            t_tabla_de_paginas* pagina_a_eliminar = list_remove(tabla_de_paginas, i);
            liberar_marco(pagina_a_eliminar->marco); // Liberar el marco
            free(pagina_a_eliminar); // Liberar la memoria de la página
        }
    }
    t_paquete* paquete = crear_paquete(EXITO);
    int operacion_exitosa = 1;
    agregar_a_paquete(paquete, &operacion_exitosa, sizeof(int));
    enviar_paquete(paquete, socket_cliente);
    eliminar_paquete(paquete);
    free(buffer);
}



//Acceso a Tabla de Paginas: "PID:" <PID> "- Pagina:" <PAGINA> "- Marco:" <MARCO>
void obtener_marco(int socket_cliente){

    int size;
	void* buffer = recibir_buffer(&size, socket_cliente);
    int pid;
	int pagina;
    
	memcpy(&pid, buffer, sizeof(int));
	memcpy(&pagina, buffer + sizeof(int), sizeof(int));
    log_mati(logger2, "La pagina que recibo para obtener el marco es: %i", pagina);
    // Pasamos el pid a string xq dictionary_get recibe si o si un string
    char* pid_string = string_itoa(pid);

    t_list* tabla_de_paginas = dictionary_get(diccionario_tabla_de_paginas_porPID , pid_string); //Buscamos la tabla de paginas del PID requerido
    free(pid_string);
    if (!tabla_de_paginas) {
        log_error(logger, "Error: No se encontró la tabla de páginas para el PID %d\n", pid);
        exit(-1);
    }

    if(list_size(tabla_de_paginas) > pagina){
        t_tabla_de_paginas* entrada = list_get(tabla_de_paginas, pagina);  //Obtenemos la pagina requerida
        log_facu(logger2, "La pagina que recibo para obtener el marco es: %i", entrada->nro_pagina);
        if(entrada->bit_validez == 1){// si la pagina existe, se la enviamos a cpu
    
            t_paquete* marco_paquete = crear_paquete(ACCEDER_TABLA_PAGINAS);                                                        
            agregar_a_paquete(marco_paquete, &(entrada->marco), sizeof(int));
            enviar_paquete(marco_paquete, socket_cliente);
            eliminar_paquete(marco_paquete);
            log_mati(logger2, "PID: %d - Pagina: %d - Marco: %d" , pid, entrada->nro_pagina, entrada->marco);
        } 
    }
    else{ //caso contrario enviamos el mensaje de error
        t_paquete* marco_paquete = crear_paquete(OUT_OF_MEMORY);
        int marco_no_encontrado = -1;
        agregar_a_paquete(marco_paquete, &marco_no_encontrado , sizeof(int));
        enviar_paquete(marco_paquete, socket_cliente);
        eliminar_paquete(marco_paquete);
        log_error(logger,"La pagina pedida no existe");
    }
    free(buffer);
}



int obtener_marco_libre(t_bitarray *bitmap){
    for (int i = 0; i < bitarray_get_max_bit(bitmap); i++) {
        if (!bitarray_test_bit(bitmap, i)) {
        //if (bitarray_test_bit(bitmap, i) == 0) {  // Verifica si el bit está libre (es 0)
            bitarray_set_bit(bitmap, i); // Marca el bit como ocupado (pone 1)
            return i;
        }
    }
    return -1;  // No hay bits libres, OUT OF MEMORY?
}

void liberar_marco(int marco) {
    bitarray_clean_bit(bitmap, marco); // Marca el bit como libre (pone 0)
}

void inicializar_bitmap(){
    for(int i = 0; i < bitarray_get_max_bit(bitmap); i++){
        bitarray_clean_bit(bitmap, i);
    }
}

//Acceso a espacio de usuario: "PID:" <PID> "- Accion:" <LEER / ESCRIBIR> "- Direccion fisica:" <DIRECCION_FISICA> "- Tamanio" <TAMANIO A LEER / ESCRIBIR>
void acceso_lectura(int socket_cliente){
    int size;
    void* buffer = recibir_buffer(&size, socket_cliente);
    
    int desplazamiento = 0;
    int pid;
    int direc_fisica;
    int tamanio_lectura;
    
    //log_warning(logger, "acceso_lectura");
    
    memcpy(&pid, buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);
    log_warning(logger, "Pid : %d", pid);
    memcpy(&direc_fisica, buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);
    log_warning(logger, "Direc_fisica : %i", direc_fisica);
    memcpy(&tamanio_lectura, buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);
    log_warning(logger, "Tamanio a leer : %i", tamanio_lectura);
    // Asignar memoria para el contenido a leer
    void* contenido_leer = malloc(tamanio_lectura);

    if((direc_fisica + tamanio_lectura) > config_memoria->tam_memoria){
        log_error(logger, "Quisieron leer fuera del espacio de memoria");
        t_paquete* paquete = crear_paquete(OUT_OF_MEMORY);
        int fuera_de_memoria = -1;
        agregar_a_paquete(paquete, &fuera_de_memoria , sizeof(int));
        enviar_paquete(paquete, socket_cliente);
        eliminar_paquete(paquete);
        free(buffer);
        return;
    }

    // Verificar que no se lea fuera del espacio de usuario
    if ((direc_fisica + tamanio_lectura) > config_memoria->tam_memoria) {
        log_error(logger, "Error: Se intentó leer fuera del espacio de usuario\n");
        t_paquete* paquete = crear_paquete(OUT_OF_MEMORY);
        int verificacion = -1;
        agregar_a_paquete(paquete, &verificacion,sizeof(int));
        enviar_paquete(paquete, socket_cliente);
        eliminar_paquete(paquete);
        return;
    }

    //mem_hexdump(espacio_de_usuario, 200);

    // Realizar la lectura del espacio de usuario
    //mem_hexdump(espacio_de_usuario + direc_fisica, tamanio_lectura);


    memcpy(contenido_leer, espacio_de_usuario + direc_fisica, tamanio_lectura);
    //log_fede(logger2, "Lo que leemos es: %s ", mem_hexdump(espacio_de_usuario + direc_fisica, tamanio_lectura));
    log_info(logger, "Acceso a espacio de usuario: PID: %d - Accion: LEER - Direccion fisica: %d - Tamaño: %d", pid, direc_fisica, tamanio_lectura);
    //log_info(logger, "El espacio de memoria es: %s", mem_hexstring(espacio_de_usuario, 300));
    //mem_hexdump(espacio_de_usuario + direc_fisica, tamanio_lectura);


    t_paquete* paquete = crear_paquete(EXITO);
    agregar_a_paquete(paquete, contenido_leer, tamanio_lectura);
    enviar_paquete(paquete, socket_cliente);

    eliminar_paquete(paquete);
    free(contenido_leer);
    free(buffer);
}

void acceso_escritura(int socket_cliente){
    int size;
    void* buffer = recibir_buffer(&size, socket_cliente);
    
    int desplazamiento = 0;
    int pid;
    int direc_fisica;
    int tamanio_escritura;
    int tamanio;
    void* contenido_a_escribir;

    memcpy(&pid, buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);
    log_warning(logger, "Pid : %i", pid);
    memcpy(&direc_fisica, buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);
    log_warning(logger, "Direc_fisica : %i", direc_fisica);
    memcpy(&tamanio_escritura, buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);
    log_warning(logger, "Tamanio a escribir : %i", tamanio_escritura);
    contenido_a_escribir = malloc(tamanio_escritura); // Asignar memoria para el contenido a escribir
    memcpy(contenido_a_escribir, buffer + desplazamiento, tamanio_escritura);
    //log_fede(logger2, "Lo que leemos es: %s ", memcpy(contenido_a_escribir, buffer + desplazamiento, tamanio_escritura));

    
    int verificacion = 1;
    
    // Verificar que no se escriba fuera del espacio de usuario
    if ((direc_fisica + tamanio_escritura) > config_memoria->tam_memoria) {
        log_error(logger, "Error: Se intentó escribir fuera del espacio de usuario\n");
        t_paquete* paquete = crear_paquete(OUT_OF_MEMORY);
        verificacion = -1;
        agregar_a_paquete(paquete, &verificacion,sizeof(int));
        enviar_paquete(paquete, socket_cliente);
        eliminar_paquete(paquete);
        return;
    }
    // se escribe en el espacio usuario el contenido a escribir

    memcpy(espacio_de_usuario + direc_fisica, contenido_a_escribir, tamanio_escritura);
    //mem_hexdump(espacio_de_usuario + direc_fisica, tamanio_escritura);

    //mem_hexdump(espacio_de_usuario, 200);

    log_info(logger2, "Acceso a espacio de usuario: PID: %d - Accion: ESCRIBIR - Direccion fisica: %d - Tamaño: %d", pid, direc_fisica, tamanio_escritura);
    //log_info(logger, "El espacio de memoria es: %s", mem_hexstring(espacio_de_usuario, 300));
    t_paquete* paquete = crear_paquete(EXITO);
    agregar_a_paquete(paquete, &verificacion,sizeof(int));
    enviar_paquete(paquete, socket_cliente);
    eliminar_paquete(paquete);

    free(contenido_a_escribir);
    free(buffer);

}