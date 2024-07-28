#ifndef MMU_H
#define MMU_H

#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <utils/socket.h>
#include <utils/logger.h>
#include <utils/instrucciones.h>
#include "cpu.h"
#include "cpu-estructuras.h"
#include "cpu-ciclo-instrucciones.h"

t_mmu_cpu* traducirDireccion(int direccionLogica , int tamanio);
t_tabla_de_paginas_cpu* buscarEnTLB(int num_pagina);
t_tabla_de_paginas_cpu* actualizar_TLB_por_fifo(int numero_pagina);
t_tabla_de_paginas_cpu* actualizar_Tlb_por_lru(int numero_pagina);


#endif