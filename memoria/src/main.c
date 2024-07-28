#include "memoria.h"

int main()
{
    //Creando logger
    logger =  log_create("memoria.log","Memoria", 1, LOG_LEVEL_INFO);
    logger2 = log_create2("memoria.log", "Memoria", 1, LOG_LEVEL_MATI);

    if (logger == NULL)
	{
		perror("No se puedo encontrar el archivo");
		return EXIT_FAILURE;
	}

    crear_config_memoria();
    configurar_senial_cierre();
    
    //Inicializamos conexiones
    if (cargar_configuraciones_memoria(config_memoria) != 1)
    { // Generar conexiones, no va a mantener la conexion, sino que va a crear la conexion y la va a cerrar!
        log_error(logger, "Cargar las configuraciones");

        return EXIT_FAILURE;
    }

    //Creamos el diccionario de instrucciones por PID     
    inicializacion_diccionario();

    //Creamos el diccionario de paginas por PID  
    crear_diccionario_tabla_de_paginas_porPID();
    
    // memoria de usuario y bitmap
    crear_bitmap();
    crear_espacio_usuario();
    inicializar_bitmap();

    //abrimos el servidor
    
    iniciar_modulo(config_memoria);
 

    return 0;
}