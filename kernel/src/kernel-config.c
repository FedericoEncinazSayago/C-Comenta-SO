#include "kernel-config.h"

t_config* config;
// Funciones para inicializar:

t_config_kernel* inicializar_config_kernel() {
	t_config_kernel* config_kernel = malloc(sizeof(t_config_kernel));

	config_kernel->PUERTO_ESCUCHA = 0;
	config_kernel->IP_MEMORIA = NULL;
	config_kernel->PUERTO_MEMORIA = 0;
	config_kernel->IP_CPU = NULL;
	config_kernel->SOCKET_DISPATCH = 0;
	config_kernel->SOCKET_INTERRUPT = 0;
	config_kernel->ALGORITMO_PLANIFICACION = NULL;
	config_kernel->QUANTUM = 0;
	config_kernel->RECURSOS = NULL;
	config_kernel->INST_RECURSOS = NULL;
	config_kernel->GRADO_MULTIP = 0;
	config_kernel->PUERTO_CPU_DS = NULL;
	config_kernel->PUERTO_CPU_IT = NULL;

	return config_kernel;
}

t_config_kernel *cargar_config_kernel(char *path_config) {
	t_config_kernel *config_kernel = inicializar_config_kernel();
	cargar_configuraciones(config_kernel, path_config);

	return config_kernel;
}

// Funciones para cargar configuraciones:

void cargar_configuraciones(t_config_kernel* config_kernel, char *path_config) {
	config = config_create("kernel.config"); //path_config

	if (!config) {
		log_info(logger, "No se pudo abrir el archivo de configuraciones!");

		exit(-1);
	}

	char *configuraciones[] = {
		"PUERTO_ESCUCHA",
		"IP_MEMORIA",
		"PUERTO_MEMORIA",
		"IP_CPU",
		"PUERTO_CPU_DISPATCH",
		"PUERTO_CPU_INTERRUPT",
		"ALGORITMO_PLANIFICACION",
		"QUANTUM",
		"RECURSOS",
		"INSTANCIAS_RECURSOS",
		"GRADO_MULTIPROGRAMACION",
		NULL
	};

	if(!tiene_todas_las_configuraciones(config, configuraciones)){
		log_error(logger,"NO se pudieron cargar todas las configuraciones");
	}

	cargar_valores_de_memoria(config, config_kernel);
	cargar_valores_de_cpu(config, config_kernel);
	cargar_valores_de_planificacion(config, config_kernel);
	cargar_valores_de_recursos(config, config_kernel);
	cargar_valores_de_grado_multiprogramacion(config, config_kernel);

	info_config(config);
}
// Funciones para cargar valores:

void cargar_valores_de_memoria(t_config *config, t_config_kernel *config_kernel) {
	char *ip_memoria = config_get_string_value(config, "IP_MEMORIA");
	char *puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
	char *puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");

    copiar_valor(&config_kernel->IP_MEMORIA, ip_memoria);
    copiar_valor(&config_kernel->PUERTO_MEMORIA, puerto_memoria);
    copiar_valor(&config_kernel->PUERTO_ESCUCHA, puerto_escucha);

	free(ip_memoria);
	free(puerto_memoria);
	free(puerto_escucha);
}

void cargar_valores_de_cpu(t_config *config, t_config_kernel *config_kernel) {
	char *ip_cpu = config_get_string_value(config, "IP_CPU");
	char *puerto_cpu_ds = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
	char *puerto_cpu_it = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");

	copiar_valor(&config_kernel->IP_CPU, ip_cpu);
	copiar_valor(&config_kernel->PUERTO_CPU_DS, puerto_cpu_ds);
	copiar_valor(&config_kernel->PUERTO_CPU_IT, puerto_cpu_it);

	free(ip_cpu);
	free(puerto_cpu_ds);
	free(puerto_cpu_it);
}

void cargar_valores_de_planificacion(t_config *config, t_config_kernel *config_kernel) {
	char *algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");

	copiar_valor(&config_kernel->ALGORITMO_PLANIFICACION, algoritmo_planificacion);
	config_kernel->QUANTUM = config_get_int_value(config, "QUANTUM");

	free(algoritmo_planificacion);
}

void cargar_valores_de_recursos(t_config *config, t_config_kernel *config_kernel) {
    char** recursos = config_get_array_value(config, "RECURSOS");
    crear_vector_dinamico_char(&config_kernel->RECURSOS, recursos);
    string_array_destroy(recursos); // Liberar el array de recursos TODO

    char** instancias_recursos = config_get_array_value(config, "INSTANCIAS_RECURSOS");
    crear_vector_dinamico_int(&config_kernel->INST_RECURSOS, instancias_recursos);
    string_array_destroy(instancias_recursos); // Liberar el array de instancias de recursos
}

void cargar_valores_de_grado_multiprogramacion(t_config *config, t_config_kernel *config_kernel) {
	config_kernel->GRADO_MULTIP = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
}

// Funciones para extraer valores:

int get_socket_memoria() {
	return config_kernel->SOCKET_MEMORIA;
}

int get_socket_dispatch() {
	return config_kernel->SOCKET_DISPATCH;
}

int get_socket_interrupt() {
	return config_kernel->SOCKET_INTERRUPT;
}

// Funciones para setear valores:

void set_socket_memoria(int socket) {
	config_kernel->SOCKET_MEMORIA = socket;
}

void set_socket_dispatch(int socket) {
	config_kernel->SOCKET_DISPATCH = socket;
}

void set_socket_interrupt(int socket) {
	config_kernel->SOCKET_INTERRUPT = socket;
}