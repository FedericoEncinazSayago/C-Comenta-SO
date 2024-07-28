#include "cpu.h"

// Variables globales
t_config_cpu *config_cpu;
t_list* tlb;

int main(void) {
    atomic_store(&interrupt_flag, 0);
    tlb = list_create();
    int md_cpu_ds = 0;
    int md_cpu_it = 0;
    logger = log_create("CPUlog.log", "CPU", 1, LOG_LEVEL_INFO);
    logger2 = log_create2("CPUlog.log", "CPU", 1, LOG_LEVEL_MATI);
    
    if (!logger || !logger2) {
        perror("No se pudo encontrar el archivo");
        return EXIT_FAILURE;
    }

    config_cpu = inicializar_config();
    cargar_configuraciones(config_cpu);
    configurar_senial_cierre_cpu();

    pthread_t hilo_memoria;
    pthread_create(&hilo_memoria, NULL, generar_conexion_a_memoria, NULL); 
    pthread_join(hilo_memoria, NULL);

    crear_servidores_cpu(&md_cpu_ds, &md_cpu_it);
    

    return EXIT_SUCCESS;
}

void limpiar_recursos(int signal) {
     if(signal == SIGINT) {

    log_nico(logger2, "Cerrando programa ...");

    if (config_cpu) {
        free(config_cpu->IP_MEMORIA);
        free(config_cpu->PUERTO_MEMORIA);
        free(config_cpu->PUERTO_ESCUCHA_DISPATCH);
        free(config_cpu->PUERTO_ESCUCHA_INTERRUPT);
        free(config_cpu->ALGORITMO_TLB);
        free(config_cpu);
    }

    liberar_pcb();
    liberar_mmu();
    
    if (tlb) {list_destroy_and_destroy_elements(tlb, (void (*)(void*)) free);}
    if (logger) {log_destroy(logger);}
    if (logger2) {log_destroy(logger2);}
    
    exit(0);
    }
}