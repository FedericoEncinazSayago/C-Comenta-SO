#include "estructuras_compartidas.h"

t_procesar_conexion* crear_procesar_conexion(char *server_name, int socket_cliente)
{
    t_procesar_conexion *procesar_cliente = malloc(sizeof(t_procesar_conexion));
    procesar_cliente -> server_name = server_name;
    procesar_cliente -> socket_cliente = socket_cliente;

    return procesar_cliente;
}

