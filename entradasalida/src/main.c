#include "io.h"

int main(int argc, char *argv[]) {
    // name_interfaz = argv[1];
    // config_path = argv[2];

    char* name_interfaz = readline("Nombre de interfaz: ");
    char* config_path = readline("Nombre del path: ");

    string_trim_right(&name_interfaz);
    string_trim_right(&config_path);

    logger = log_create("io.log", "IO", 1, LOG_LEVEL_TRACE);
    logger2 = log_create2("io.log", "IO", 1, LOG_LEVEL_MATI);
    interfaz = inicializar_interfaz(name_interfaz, config_path);

    configurar_senial_cierre();
    interfaz_generar_conexiones_con(interfaz);
    send_interfaz_a_kernel(interfaz);
    interfaz_recibir_peticiones(interfaz);

    return 0;
}

