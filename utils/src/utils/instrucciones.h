#ifndef INSTRUCCIONES_H
#define INSTRUCCIONES_H

#include <commons/collections/list.h>
#include <utils/protocolo.h>
#include <stdlib.h>
#include <string.h>


typedef enum {
// Generales Compartidas con kernel
    FIN_QUANTUM,    
    WAIT,
    SIGNAL,
    EXIT,
    OUT_OF_MEMORY,
    INSTRUCCION_IO,
    IO_INVALIDA,
    INVALID_RESOURCE,
    INTERRUPTED_BY_USER,
// CPU
    SET,
    MOV_IN,
    MOV_OUT,
    SUM,
    SUB,
    JNZ,
    RESIZE,
    COPY_STRING,
    IO_GEN_SLEEP,
    IO_STDIN_READ,
    IO_STDOUT_WRITE,
    IO_FS_CREATE,
    IO_FS_DELETE,
    IO_FS_TRUNCATE,
    IO_FS_WRITE,
    IO_FS_READ
} t_tipo_instruccion;

typedef struct {  
    char* opcode;
    char* parametro1;
    char* parametro2;
    char* parametro3;
    char* parametro4;
    char* parametro5;
} t_instruccion;


t_instruccion* recv_instruccion(int socket_cliente);
void solicitar_instruccion(int socket_server, int PID, int program_counter);
t_tipo_instruccion obtener_tipo_instruccion(char *instruccion);
t_instruccion* crear_instrucciones(t_list *parametros);

#endif // INSTRUCCIONES_H