#include "kernel-conexiones.h"

void generar_conexiones_con() {
    generar_conexiones_con_cpu();
    generar_conexion_con_memoria();
}

void generar_conexiones_con_cpu(void) {
    int md_dispatch_cpu = 0;
    int md_interrupt_cpu = 0;

    char* puerto_dispatch = config_kernel->PUERTO_CPU_DS;
    char* puerto_interrupt = config_kernel->PUERTO_CPU_IT;
    char* ip_cpu = config_kernel->IP_CPU;

    md_dispatch_cpu  = crear_conexion("DISPATCHER", ip_cpu, puerto_dispatch);
    md_interrupt_cpu = crear_conexion("INTERRUPT", ip_cpu, puerto_interrupt);

    if(md_dispatch_cpu == -1 || md_interrupt_cpu == -1) {
        log_error(logger, "No se pudo conectar con la CPU");
        exit(-1);
    }

    // Handshake:    
    generar_handshake(md_dispatch_cpu, "DISPATCHER", ip_cpu, puerto_dispatch);
    generar_handshake(md_interrupt_cpu, "INTERRUPT", ip_cpu, puerto_interrupt);

    // Seteo de sockets:
    set_socket_dispatch(md_dispatch_cpu);
    set_socket_interrupt(md_interrupt_cpu);
}

void generar_conexion_con_memoria(void) {
    int md_memoria = 0;

    char* puerto_memoria = config_kernel->PUERTO_MEMORIA;
    char* ip_memoria = config_kernel->IP_MEMORIA;

    md_memoria = crear_conexion("MEMORIA", ip_memoria, puerto_memoria);

    if(md_memoria == -1)
        exit(-1);

    // Handshake:
    generar_handshake(md_memoria, "MEMORIA", ip_memoria, puerto_memoria);
    
    // Seteo de socket:
    set_socket_memoria(md_memoria);
}

int crear_servidor_kernel() {
    int socket_servidor = 0;
    char *puerto = config_kernel->PUERTO_ESCUCHA;

    socket_servidor = iniciar_servidor("KERNEL", NULL, puerto);

    if(socket_servidor == -1) {
        log_error(logger, "No se pudo crear el servidor");
        exit(-1);
    }

    return socket_servidor;
}