#include "mmu.h"
int posicion_fifo = 0;


t_mmu_cpu* traducirDireccion(int direccionLogica, int tamanio) {
    t_mmu_cpu* mmu = (t_mmu_cpu*) malloc(sizeof(t_mmu_cpu)); //linea 6
    if (!mmu) {
        log_error(logger, "Error al asignar memoria para MMU");
        return NULL;
    }

    mmu->tamanio = list_create();
    mmu->direccionFIsica = list_create();
    mmu->num_pagina = list_create();
    mmu->ofset = list_create();

    int pagina = (int)floor((double)direccionLogica / config_cpu->TAMANIO_PAGINA);
    int offset = direccionLogica - pagina * config_cpu->TAMANIO_PAGINA;

    int *pagina_ptr = malloc(sizeof(int));
    int *offset_ptr = malloc(sizeof(int));
    if (!pagina_ptr || !offset_ptr) {
        log_error(logger, "Error al asignar memoria para pagina u offset");
        liberar_mmu();
        return NULL;
    }

    *pagina_ptr = pagina;
    *offset_ptr = offset;
    list_add(mmu->num_pagina, pagina_ptr);
    list_add(mmu->ofset, offset_ptr);

    int tamanio_Actualizado = config_cpu->TAMANIO_PAGINA - offset;

    int* tamanio_ptr = malloc(sizeof(int)); //LINEA 34
    if (!tamanio_ptr) {
        log_error(logger, "Error al asignar memoria para tamanio");
        liberar_mmu();
        return NULL;
    }
    *tamanio_ptr = tamanio_Actualizado < tamanio ? tamanio_Actualizado : tamanio;
    list_add(mmu->tamanio, tamanio_ptr);

    int pagina_actualizada = pagina + 1;
    int direccionLogica_actualizada = pagina_actualizada * config_cpu->TAMANIO_PAGINA;
    tamanio_Actualizado = tamanio - tamanio_Actualizado;

    while (config_cpu->TAMANIO_PAGINA <= tamanio_Actualizado) {
        //log_warning(logger, "*tamanio_Actualizad: %i ", tamanio_Actualizado);
        int* pagina_ptrr = malloc(sizeof(int));
        int* offset_ptrr = malloc(sizeof(int)); //linea 52
        int* tamanio_ptrr = malloc(sizeof(int));

        if (!pagina_ptrr || !offset_ptrr || !tamanio_ptrr) {
            log_error(logger, "Error al asignar memoria en el bucle");
            liberar_mmu();
            return NULL;
        }

        *pagina_ptrr = pagina_actualizada;
        *offset_ptrr = 0;
        *tamanio_ptrr = config_cpu->TAMANIO_PAGINA;
       // log_warning(logger, "*tamanio_ptr: %i ", *tamanio_ptrr);

        list_add(mmu->num_pagina, pagina_ptrr);
        list_add(mmu->ofset, offset_ptrr);
        list_add(mmu->tamanio, tamanio_ptrr);

        direccionLogica_actualizada += config_cpu->TAMANIO_PAGINA;
        tamanio_Actualizado -= config_cpu->TAMANIO_PAGINA;
        pagina_actualizada++;
    }

    if (tamanio_Actualizado > 0) {
        //log_warning(logger, "*tamanio_Actualizad: %i ", tamanio_Actualizado);
        int* pagina_ptrrr = malloc(sizeof(int));
        int* offset_ptrrr = malloc(sizeof(int)); // linea 76
        int* tamanio_ptrrr = malloc(sizeof(int));

        if (!pagina_ptrrr || !offset_ptrrr || !tamanio_ptrrr) {
            log_error(logger, "Error al asignar memoria para la última porción");
            liberar_mmu();
            return NULL;
        }

        *pagina_ptrrr = pagina_actualizada;
        *offset_ptrrr = 0;
        *tamanio_ptrrr = tamanio_Actualizado;
        //log_warning(logger, "*tamanio_ptrrr: %i ", *tamanio_ptrrr);

        list_add(mmu->num_pagina, pagina_ptrrr);
        list_add(mmu->ofset, offset_ptrrr);
        list_add(mmu->tamanio, tamanio_ptrrr);
    }

    for (int i = 0; i < list_size(mmu->num_pagina); i++) {
        int* pagina = (int*)list_get(mmu->num_pagina, i);
        int* offset_ptr = (int*)list_get(mmu->ofset, i);
        int offset = *offset_ptr;
        int pagina_a_buscar = *pagina;

        t_tabla_de_paginas_cpu* tabla = (t_tabla_de_paginas_cpu*)buscarEnTLB(pagina_a_buscar);

        if (tabla == NULL) { // NO TLB
            if (list_size(tlb) < config_cpu->CANTIDAD_ENTRADAS_TLB || config_cpu->CANTIDAD_ENTRADAS_TLB == 0) {
                solicitar_tablas_a_memoria(pagina_a_buscar);
                t_tabla_de_paginas_cpu* tab = recv_tablas();

                tab->pid = pcb->pid;
                tab->contador = 0;
                tab->nropagina = pagina_a_buscar;

                if (list_size(tlb) < config_cpu->CANTIDAD_ENTRADAS_TLB) {
                    list_add(tlb, tab);
                }

                int dirc_fisica = tab->marco * config_cpu->TAMANIO_PAGINA + offset;
                int* ptr_dirc_fisica = malloc(sizeof(int)); // linea 114
                if (!ptr_dirc_fisica) {
                    log_error(logger, "Error al asignar memoria para ptr_dirc_fisica");
                    liberar_mmu();
                    return NULL;
                }

                *ptr_dirc_fisica = dirc_fisica;
                list_add(mmu->direccionFIsica, ptr_dirc_fisica);
                log_info(logger, "PID %i -TLB MISS -Pagina %i", pcb->pid, tab->nropagina);
                if (config_cpu->CANTIDAD_ENTRADAS_TLB == 0){
                    free(tab);
                }
                
            } else {
                if (strcmp(config_cpu->ALGORITMO_TLB, "FIFO") == 0) {
                    t_tabla_de_paginas_cpu* tab = (t_tabla_de_paginas_cpu*)actualizar_TLB_por_fifo(pagina_a_buscar);
                    int dirc_fisica = tab->marco * config_cpu->TAMANIO_PAGINA + offset;
                    int* ptr_dirc_fisica = malloc(sizeof(int));
                    if (!ptr_dirc_fisica) {
                        log_error(logger, "Error al asignar memoria para ptr_dirc_fisica");
                        liberar_mmu();
                        free(tab);
                        return NULL;
                    }

                    *ptr_dirc_fisica = dirc_fisica;
                    list_add(mmu->direccionFIsica, ptr_dirc_fisica);
                } else if (strcmp(config_cpu->ALGORITMO_TLB, "LRU") == 0) {
                    t_tabla_de_paginas_cpu* tab = (t_tabla_de_paginas_cpu*)actualizar_Tlb_por_lru(pagina_a_buscar);
                    int dirc_fisica = tab->marco * config_cpu->TAMANIO_PAGINA + offset;
                    int* ptr_dirc_fisica = malloc(sizeof(int));
                    if (!ptr_dirc_fisica) {
                        log_error(logger, "Error al asignar memoria para ptr_dirc_fisica");
                        liberar_mmu();
                        free(tab);
                        return NULL;
                    }

                    *ptr_dirc_fisica = dirc_fisica;
                    list_add(mmu->direccionFIsica, ptr_dirc_fisica);
                    // free(tab);
                } else {
                    log_error(logger, "Algoritmo no es valido en mmu");
                    
                    liberar_mmu();
                    return NULL;
                }
            }
        } else {
            int dirc_fisica = tabla->marco * config_cpu->TAMANIO_PAGINA + offset;
            int* ptr_dirc_fisica = malloc(sizeof(int)); //LINEA 161
            if (!ptr_dirc_fisica) {
                log_error(logger, "Error al asignar memoria para ptr_dirc_fisica");
                liberar_mmu();
                return NULL;
            }

            *ptr_dirc_fisica = dirc_fisica;
            log_info(logger, "PID %i - TLB HIT - Pagina: %i", pcb->pid, tabla->nropagina);
            log_info(logger, "PID %i - OBTENER MARCO - Pagina: %i - Marco: %i", pcb->pid, tabla->nropagina, tabla->marco);
            // free(tabla);
            list_add(mmu->direccionFIsica, ptr_dirc_fisica);
        }
    }

    return mmu;
}

t_tabla_de_paginas_cpu* buscarEnTLB(int num_pagina){
    
    if (tlb == NULL) {
        log_warning(logger, "La lista TLB no está inicializada.");
        return NULL;
    }

  if (strcmp(config_cpu->ALGORITMO_TLB, "LRU") == 0) {
        for (int i = 0; i < list_size(tlb); i++) {
            t_tabla_de_paginas_cpu* tlb_item = (t_tabla_de_paginas_cpu*) list_get(tlb, i);
            if (tlb_item == NULL) {
                continue;
            }
            tlb_item->contador++;
        }
    }

    for (int i = 0; i < list_size(tlb); i++){
        t_tabla_de_paginas_cpu* tabla_a_buscar = (t_tabla_de_paginas_cpu*)list_get(tlb,i); // recorremos la TLB
        //log_warning(logger, "Nro de pagina: %i", tabla_a_buscar->nropagina);
        if (tabla_a_buscar->nropagina == num_pagina && tabla_a_buscar->pid == pcb->pid ){ //caundo hallamos la pagina requerida, returneamos
            if (strcmp(config_cpu->ALGORITMO_TLB, "LRU") == 0){
                t_tabla_de_paginas_cpu* tabla = (t_tabla_de_paginas_cpu*)list_remove(tlb,i);
                tabla->contador = 0;
                list_add_in_index(tlb,i,tabla);
            } 
            //log_warning(logger, "Nro de pagina: %i", tabla_a_buscar->nropagina);
            return tabla_a_buscar;

        }
    }
    return NULL;
}


t_tabla_de_paginas_cpu* actualizar_TLB_por_fifo(int numero_pagina){
        if (posicion_fifo < config_cpu->CANTIDAD_ENTRADAS_TLB){
            t_tabla_de_paginas_cpu* tabla   = list_remove(tlb, posicion_fifo); //Puede ser tlb 
            free(tabla);

            solicitar_tablas_a_memoria(numero_pagina); 
            t_tabla_de_paginas_cpu* nueva_tabla = recv_tablas();
            if (nueva_tabla != NULL){
                nueva_tabla->pid = pcb->pid;
                nueva_tabla->nropagina = numero_pagina;
                log_info(logger,"PID %i -TLB MISS -Pagina %i",pcb->pid,nueva_tabla->nropagina);
                list_add_in_index(tlb,posicion_fifo,nueva_tabla);//Inserto en el lugar sacado
                ++posicion_fifo;
                return nueva_tabla;
            }else{
                log_error(logger, "No llego nada la tabla de memoria FIFO");
                return NULL;
                free(nueva_tabla);
            }
        }else{
            posicion_fifo = 0;
            solicitar_tablas_a_memoria(numero_pagina); 
            t_tabla_de_paginas_cpu* nueva_tabla = recv_tablas();
            if (nueva_tabla != NULL){
                nueva_tabla->pid = pcb->pid;
                nueva_tabla->nropagina = numero_pagina;
                log_info(logger,"PID %i -TLB MISS -Pagina %i",pcb->pid,nueva_tabla->nropagina);
                list_add_in_index(tlb,posicion_fifo,nueva_tabla);//Puede ser tlb
                return nueva_tabla;
                free(nueva_tabla);
            }else{
                log_error(logger, "No llego nada la tabla de memoria FIFO");
                free(nueva_tabla);
                return NULL;
            }
            ++posicion_fifo;
        }        
        
}


t_tabla_de_paginas_cpu* actualizar_Tlb_por_lru(int numero_pagina){
    int contador = -1;
    int indice = -1;


    for (int i = 0; i < list_size(tlb); i++){
        t_tabla_de_paginas_cpu* tabla = list_get(tlb,i);
        //log_warning(logger, "Posicion del lru: %i    || contador: %i", tabla->nropagina, tabla->contador);

        if (tabla->contador > contador){
            contador = tabla->contador;
            indice = i;
        }
    }
//log_warning(logger, "indice: %i", indice);
if (indice != -1){
   t_tabla_de_paginas_cpu* tabla_a_eliminar = (t_tabla_de_paginas_cpu*)list_remove(tlb,indice);
   log_nico(logger2, "Pagina extraida: %i", tabla_a_eliminar->nropagina);
    free(tabla_a_eliminar);

    solicitar_tablas_a_memoria(numero_pagina);
    t_tabla_de_paginas_cpu* nueva_tabla = recv_tablas();
    if (nueva_tabla != NULL){
        nueva_tabla->contador = 0;
        nueva_tabla->nropagina = numero_pagina;
        nueva_tabla->pid = pcb->pid;  
        list_add(tlb, nueva_tabla); //Puede ser tlb
        log_info(logger,"PID %i -TLB MISS -Pagina %i",pcb->pid,nueva_tabla->nropagina);
        return nueva_tabla;
    }else{
        log_error(logger, "No llego nada la tabla de memoria LRU");
        return NULL;
        free(nueva_tabla);
    }
    
}else{
    log_error(logger, "Error en reccorer la tlb en LRU");
    return NULL;
}
}