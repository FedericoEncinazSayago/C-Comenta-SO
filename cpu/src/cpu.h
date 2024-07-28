#ifndef CPU_H
#define CPU_H

// Bibliotecas generales:
#include <utils/logger.h>

// Bibliotecas internas:
#include "cpu-config.h"
#include "cpu-estructuras.h"
#include "cpu-conexiones.h"
#include "cpu-ciclo-instrucciones.h"

void limpiar_recursos(int signal);

//extern t_pcb* pcb;
extern t_config_cpu* config_cpu;
#endif