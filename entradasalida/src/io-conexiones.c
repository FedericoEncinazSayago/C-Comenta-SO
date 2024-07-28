#include "io-conexiones.h"

void interfaz_generar_conexiones_con(t_interfaz* interfaz) {
    generar_conexiones_con_kernel(interfaz);

    if(interfaz->tipo != GENERICA)
        generar_conexiones_con_memoria(interfaz);   
}

void generar_conexiones_con_kernel(t_interfaz* interfaz) {
    int socket_kernel;

    char *ip_kernel = interfaz->config->IP_KERNEL;
    char *puerto_kernel = interfaz->config->PUERTO_KERNEL;

    generar_conexion(&socket_kernel, "KERNEL", ip_kernel, puerto_kernel);

    interfaz->socket_with_kernel = socket_kernel;
}

void generar_conexiones_con_memoria(t_interfaz* interfaz) {
    int socket_memoria;

    char *ip_memoria = interfaz->config->IP_MEMORIA;
    char *puerto_memoria = interfaz->config->PUERTO_MEMORIA;

    generar_conexion(&socket_memoria, "MEMORIA", ip_memoria, puerto_memoria);

    interfaz->socket_with_memoria = socket_memoria;
}

void generar_conexion(int *socket, char *nombre_interfaz, char *ip, char *puerto) {
    *socket = crear_conexion(nombre_interfaz, ip, puerto);

    if(*socket == 0 || *socket == -1)
        exit(EXIT_FAILURE);

    generar_handshake(*socket, nombre_interfaz, ip, puerto);
}