#include "instrucciones.h"



t_instruccion* recv_instruccion(int socket_cliente) {
    op_code operacion = recibir_operacion(socket_cliente);
    t_list *parametros = list_create();

    int size;
    int desplazamiento = 0;
    void* buffer = recibir_buffer(&size, socket_cliente);

    if(buffer == NULL) {
        log_error(logger, "Error al recibir el buffer del socket");
        return NULL;
    }

    while (desplazamiento < size) {
        int tamanio;
        memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
        desplazamiento += sizeof(int);
        if (tamanio == -1){
            t_instruccion* instruccion;
            instruccion->opcode = "Desalojo de usuario.";
            free(buffer);
            list_destroy(parametros);
            return instruccion;
        }
        
        char* parametro = malloc(tamanio + 1);
        if (parametro == NULL) {
            log_error(logger,"Error al asignar memoria");
            exit(EXIT_FAILURE);
        }

        memcpy(parametro, buffer + desplazamiento, tamanio + 1);
        desplazamiento += tamanio + 1;

        parametro[tamanio] = '\0';

        list_add(parametros, parametro);
    }

    free(buffer);

    t_instruccion *instruccion = crear_instrucciones(parametros);
    list_destroy(parametros);

    return instruccion;
}

t_instruccion *crear_instrucciones(t_list *parametros) {
    t_instruccion *instruccion = malloc(sizeof(t_instruccion));
    
    instruccion->opcode = NULL;
    instruccion->parametro1 = NULL;
    instruccion->parametro2 = NULL;
    instruccion->parametro3 = NULL;
    instruccion->parametro4 = NULL;
    instruccion->parametro5 = NULL;


    for(int i = 0; i < list_size(parametros); i++) {
        char *parametro = list_get(parametros, i);

        if(i == 0) {
            instruccion->opcode = parametro;
        } else if(i == 1) {
            instruccion->parametro1 = parametro;
        } else if(i == 2) {
            instruccion->parametro2 = parametro;
        } else if(i == 3) {
            instruccion->parametro3 = parametro;
        } else if(i == 4) {
            instruccion->parametro4 = parametro;
        } else if (i == 5){
            instruccion->parametro5 = parametro;
        }
    }

    return instruccion;
}

void solicitar_instruccion(int socket_server, int PID, int program_counter) {
    t_paquete *paquete = crear_paquete(INSTRUCCION);
    agregar_a_paquete(paquete, &PID, sizeof(int));
    agregar_a_paquete(paquete, &program_counter, sizeof(int));
    
    enviar_paquete(paquete, socket_server);
    eliminar_paquete(paquete);
}

t_tipo_instruccion obtener_tipo_instruccion(char* instruccion) {
    if (instruccion == NULL){
        log_error(logger, "Instruccion llego incorrectamente");
        return -1;
    }
    

    if(strcmp(instruccion, "SET") == 0) {
        return SET;
    } else if(strcmp(instruccion, "MOV_IN") == 0) {
        return MOV_IN;
    } else if(strcmp(instruccion, "MOV_OUT") == 0) {
        return MOV_OUT;
    } else if(strcmp(instruccion, "SUM") == 0) {
        return SUM;
    } else if(strcmp(instruccion, "SUB") == 0) {
        return SUB;
    } else if(strcmp(instruccion, "JNZ") == 0) {
        return JNZ;
    } else if(strcmp(instruccion, "RESIZE") == 0) {
        return RESIZE;
    }else if(strcmp(instruccion, "COPY_STRING") == 0) {
        return COPY_STRING;  
    }else if(strcmp(instruccion, "WAIT") == 0) {
        return WAIT; 
    }else if(strcmp(instruccion, "SIGNAL") == 0) {
        return SIGNAL; 
    }else if(strcmp(instruccion, "IO_GEN_SLEEP") == 0) {
        return IO_GEN_SLEEP;
    }else if(strcmp(instruccion, "IO_STDIN_READ") == 0) {
        return IO_STDIN_READ;
    }else if(strcmp(instruccion, "IO_STDOUT_WRITE") == 0) {
        return IO_STDOUT_WRITE;
    }else if(strcmp(instruccion, "IO_FS_CREATE") == 0) {
        return IO_FS_CREATE;
    }else if(strcmp(instruccion, "IO_FS_DELETE") == 0) {
        return IO_FS_DELETE;
    }else if(strcmp(instruccion, "IO_FS_TRUNCATE") == 0) {
        return IO_FS_TRUNCATE;
    }else if(strcmp(instruccion, "IO_FS_WRITE") == 0) {
        return IO_FS_WRITE;
    }else if(strcmp(instruccion, "IO_GEN_SLEEP") == 0) {
        return IO_GEN_SLEEP;
    }else if(strcmp(instruccion, "IO_FS_READ") == 0) {
        return IO_FS_READ;
    }else if(strncmp(instruccion, "EXIT", 4) == 0) {
        return EXIT;
    }else{
        log_error(logger, "Error en buscar la instruccion");
        return -1;
    }
}


