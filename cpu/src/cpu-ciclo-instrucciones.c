#include "cpu-ciclo-instrucciones.h"

t_pcb_cpu* pcb;
t_mmu_cpu* mmu;

void iniciar_ciclo_de_ejecucion(int socket_server ,int socket_cliente) {

    config_cpu->SOCKET_KERNEL = socket_cliente;
    while(1) {
        int codigo_operacion = recibir_operacion(socket_cliente); //Acordarme que lo cambie
        switch(codigo_operacion) {  
            case RECIBIR_PROCESO:
            ejecutar_ciclo_instrucciones();
            break;
            case HANDSHAKE:
            recibir_handshake(socket_cliente);
            break;
            case -1:
            log_warning(logger,"Se desconecto el cliente Kernel (Ds)");
            close(socket_cliente);
            return; 
        default:
        log_error(logger, "Operacion desconocida dispatch");
       }
    }   
}

void ejecutar_ciclo_instrucciones() {
    pcb = rcv_contexto_ejecucion_cpu(config_cpu->SOCKET_KERNEL);
    seguir_ciclo();
}   

void seguir_ciclo(){
    fecth(config_cpu->SOCKET_MEMORIA);
    ejecutar_instruccion(config_cpu->SOCKET_MEMORIA);
}

void fecth(int socket_server) {
    int program_counter = 0;
    int PID = pcb->pid;
    program_counter = pcb->registros->PC++;
    solicitar_instruccion(socket_server,PID, program_counter);
    log_nico(logger2,"Fetch Instruccion: PID: %d - FETCH -Programn Counter: %d",PID,program_counter);  
}

void* obtener_registro (char *registro) {
    if(strncmp(registro, "AX", 2) == 0) {
        return &(pcb->registros->AX);
    } else if(strncmp(registro, "BX", 2) == 0) {
        return &(pcb->registros->BX);
    } else if(strncmp(registro, "CX", 2) == 0) {
        return &(pcb->registros->CX);
    } else if(strncmp(registro, "DX", 2) == 0) {
        return &(pcb->registros->DX);
    } else if(strncmp(registro, "PC", 2) == 0) {
        return &(pcb->registros->PC);
    } else if(strncmp(registro, "EAX", 3) == 0) {
        return &(pcb->registros->EAX);
    } else if(strncmp(registro, "EBX", 3) == 0) {
        return &(pcb->registros->EBX);
    }else if(strncmp(registro, "ECX", 3) == 0) {
        return &(pcb->registros->ECX);
    }else if(strncmp(registro, "EDX", 3) == 0) {
        return &(pcb->registros->EDX);
    }else if(strncmp(registro, "SI", 2) == 0) {
        return &(pcb->registros->SI);
    }else if(strncmp(registro, "DI", 2) == 0) {
        return &(pcb->registros->DI);
    }else {
        return NULL;
    }
}

int encontrar_int(void* registro, int tamanio){
     //log_info(logger, "CONVIRTIENDO DE VOID* A INT");
    
    if (registro == NULL) {
        log_nico(logger2, "REGISTRO EN NULL");
        return -1;
    }
    
    int registro_final = -1;

    if (tamanio == sizeof(uint8_t)) {
        uint8_t reg;
        memcpy(&reg, registro, sizeof(uint8_t));
        registro_final = (int) reg;

    } else if (tamanio == sizeof(uint32_t)) {
        uint32_t reg;
        memcpy(&reg, registro, sizeof(uint32_t));
        registro_final = (int) reg;
    } else {
        return -1;
    }

    return registro_final;
}

int espacio_de_registro(char* registro){
    if (strncmp(registro, "AX", 2) == 0 || strncmp(registro, "BX", 2) == 0|| strncmp(registro, "CX", 2) == 0|| strncmp(registro, "DX", 2) == 0){
        return sizeof(uint8_t); //en bits
    }else if (strncmp(registro, "PC", 2) == 0 || strncmp(registro, "EAX",3) == 0|| strncmp(registro, "EBX",3) == 0 || strncmp(registro, "ECX",3) == 0 || strncmp(registro, "EDX",3) == 0 || strncmp(registro, "SI",2) == 0|| strncmp(registro, "DI", 2) == 0){
        return sizeof(uint32_t); //en bits
    }else{
        log_error(logger,"Error al calulcar el tamanio del registro");
        return 0;
    }
}

void operar_con_registros(void* registro_destino, void* registro_origen, char* registro, char* operacion, int valor){
    if (strncmp(registro, "AX", 2) == 0 || strncmp(registro, "BX", 2) == 0|| strncmp(registro, "CX", 2) == 0|| strncmp(registro, "DX", 2) == 0){
        if (strcmp(operacion, "+") == 0){
            *(uint8_t*)registro_destino += *(uint8_t*) registro_origen;
        }else if (strcmp(operacion, "-") == 0){
            *(uint8_t*)registro_destino = *(uint8_t*)registro_destino - *(uint8_t*) registro_origen;
        }else if(strcmp(operacion, "set") == 0){
            *(uint8_t*)registro_destino = (uint8_t)valor;
        }else{  
            log_error(logger, "los registros no son de 8 bits");
        }
    }else if(strncmp(registro, "PC", 2) == 0 || strncmp(registro, "EAX", 3) == 0|| strncmp(registro, "EBX", 3) == 0 || strncmp(registro, "ECX", 3) == 0 || strncmp(registro, "EDX", 3) == 0 || strncmp(registro, "SI", 2) == 0|| strncmp(registro, "DI", 2) == 0){
        if (strcmp(operacion, "+") == 0){
            *(uint32_t*)registro_destino += *(uint32_t*) registro_origen;
        }else if (strcmp(operacion, "-") == 0){
            *(uint32_t*)registro_destino = *(uint32_t*)registro_destino - *(uint32_t*) registro_origen;
        }else if(strcmp(operacion, "set") == 0){
            log_info(logger, "Entre a set!");
            *(uint32_t*)registro_destino = (uint32_t)valor;
        }else{
            log_error(logger, "los registros no son de 32 bits");
        }
    }else{
        log_error(logger, "Registros desconocidos");
    }
    
}

void tengoAlgunaInterrupcion(){
    if (atomic_load(&interrupt_flag) == 1){
    t_paquete* paquete_a_kernel = crear_paquete(FIN_QUANTUM);
        log_info(logger, "DESALAJANDO");
        atomic_store(&interrupt_flag, 0);  // Reset the flag here
        enviar_pcb_a_kernel(paquete_a_kernel);
        enviar_paquete(paquete_a_kernel, config_cpu->SOCKET_KERNEL);
        eliminar_paquete(paquete_a_kernel);
        liberar_pcb();
    return;
    }else{;
        seguir_ciclo();
    }  
}

void ejecutar_instruccion(int socket_cliente) {
    t_instruccion *instruccion = recv_instruccion(socket_cliente);
    if (strncmp(instruccion->opcode, "Desalojo de usuario.", 20) == 0){
        log_nico(logger2, "Desalojo de usuario");
        liberar_pcb();
        return; //En caso de que kenrel pida desalojar y para que no explote.
    }

    t_tipo_instruccion tipo_instruccion = obtener_tipo_instruccion(instruccion->opcode); //decode

        switch (tipo_instruccion){
        case EXIT:
            log_nico(logger2,"Instruccion Ejecutada: PID: %i Ejecutando: %s", pcb->pid,instruccion->opcode);
            t_paquete* paquete_a_kernel = crear_paquete(EXIT);
            enviar_pcb_a_kernel(paquete_a_kernel);
            enviar_paquete(paquete_a_kernel, config_cpu->SOCKET_KERNEL);
            eliminar_paquete(paquete_a_kernel);
            liberar_pcb();
            liberar_instrucciones(instruccion);
            atomic_store(&interrupt_flag, 0); //Para no acumular un desalojo que no este acorde al proceso
            return; 
        case SET:
            log_nico(logger2,"Instruccion Ejecutada: PID: %d- Ejecutando: %s - %s %s", pcb->pid,instruccion->opcode,instruccion->parametro1,instruccion->parametro2);
            ejecutar_set(instruccion->parametro1,instruccion->parametro2);
            break;
        case SUM:
            log_nico(logger2,"Instruccion Ejecutada: PID: %d- Ejecutando: %s - %s %s", pcb->pid,instruccion->opcode,instruccion->parametro1,instruccion->parametro2);
            ejecutar_sum(instruccion->parametro1,instruccion->parametro2);
            break;
        case SUB:
            log_nico(logger2,"Instruccion Ejecutada: PID: %d- Ejecutando: %s - %s %s", pcb->pid,instruccion->opcode,instruccion->parametro1,instruccion->parametro2);
            ejecutar_sub(instruccion->parametro1,instruccion->parametro2);
            break;           
        case JNZ:
            log_nico(logger2,"Instruccion Ejecutada: PID: %d- Ejecutando: %s - %s %s", pcb->pid,instruccion->opcode,instruccion->parametro1,instruccion->parametro2);
            ejecutar_JNZ(instruccion->parametro1,instruccion->parametro2);
            break;
        case IO_GEN_SLEEP:
            log_nico(logger2,"Instruccion Ejecutada: PID: %d- Ejecutando: %s - %s %s", pcb->pid,instruccion->opcode,instruccion->parametro1,instruccion->parametro2);
            ejecutar_IO_GEN_SLEEP(instruccion->parametro1,instruccion->parametro2);
            liberar_pcb();
            break;
        case MOV_IN:
            log_nico(logger2,"Instruccion Ejecutada: PID: %d- Ejecutando: %s - %s %s", pcb->pid,instruccion->opcode,instruccion->parametro1,instruccion->parametro2);
            ejecutar_MOV_IN(instruccion->parametro1, instruccion->parametro2);
            break;
        case MOV_OUT:
            log_nico(logger2,"Instruccion Ejecutada: PID: %d- Ejecutando: %s - %s %s", pcb->pid,instruccion->opcode,instruccion->parametro1,instruccion->parametro2);
            ejecutar_MOV_OUT(instruccion->parametro1, instruccion->parametro2);
            break;
        case RESIZE:
            log_nico(logger2,"Instruccion Ejecutada: PID: %d- Ejecutando: %s - %s ", pcb->pid,instruccion->opcode,instruccion->parametro1);
            ejecutar_RESIZE(instruccion->parametro1);
            break;
        case COPY_STRING:
            log_nico(logger2,"Instruccion Ejecutada: PID: %d- Ejecutando: %s - %s ", pcb->pid,instruccion->opcode,instruccion->parametro1);
            ejecutar_COPY_STRING(instruccion->parametro1);
            break;
        case WAIT:
            log_nico(logger2,"Instruccion Ejecutada: PID: %d- Ejecutando: %s - %s", pcb->pid,instruccion->opcode,instruccion->parametro1);
            ejecutar_WAIT(instruccion->parametro1);
            break;
        case SIGNAL:
            log_nico(logger2,"Instruccion Ejecutada: PID: %d- Ejecutando: %s - %s", pcb->pid,instruccion->opcode,instruccion->parametro1);
            ejecutar_SINGAL(instruccion->parametro1);
            break;
        case IO_STDIN_READ:
            log_nico(logger2,"Instruccion Ejecutada: PID: %d- Ejecutando: %s - %s %s %s", pcb->pid,instruccion->opcode,instruccion->parametro1,instruccion->parametro2, instruccion->parametro3);
            ejecutar_IO_STDIN_READ(instruccion->parametro1, instruccion->parametro2,instruccion->parametro3);
            liberar_pcb();
            break;
        case IO_STDOUT_WRITE:
            log_nico(logger2,"Instruccion Ejecutada: PID: %d- Ejecutando: %s - %s %s %s", pcb->pid,instruccion->opcode,instruccion->parametro1,instruccion->parametro2, instruccion->parametro3);
            ejecutar_IO_STDOUT_WRITE(instruccion->parametro1, instruccion->parametro2,instruccion->parametro3);
            liberar_pcb();
            break;
        case IO_FS_CREATE:
            log_nico(logger2,"Instruccion Ejecutada: PID: %d- Ejecutando: %s - %s %s", pcb->pid,instruccion->opcode,instruccion->parametro1,instruccion->parametro2);
            ejecutar_IO_FS_CREATE(instruccion->parametro1,instruccion->parametro2);
            liberar_pcb();
            break;
        case IO_FS_DELETE:
            log_nico(logger2,"Instruccion Ejecutada: PID: %d- Ejecutando: %s - %s %s", pcb->pid,instruccion->opcode,instruccion->parametro1,instruccion->parametro2);
            ejecutar_IO_FS_DELETE(instruccion->parametro1,instruccion->parametro2);
            liberar_pcb();
            break;
        case IO_FS_TRUNCATE:
            log_nico(logger2,"Instruccion Ejecutada: PID: %d- Ejecutando: %s - %s %s %s", pcb->pid,instruccion->opcode,instruccion->parametro1,instruccion->parametro2, instruccion->parametro3);
            ejecutar_IO_FS_TRUNCATE(instruccion->parametro1, instruccion->parametro2,instruccion->parametro3);
            liberar_pcb();
            break;
        case IO_FS_WRITE:
            log_nico(logger2,"Instruccion Ejecutada: PID: %d- Ejecutando: %s - %s %s %s %s %s", pcb->pid,instruccion->opcode,instruccion->parametro1,instruccion->parametro2,instruccion->parametro3,instruccion->parametro4,instruccion->parametro5);
            ejecutar_IO_FD_WRITE(instruccion->parametro1,instruccion->parametro2,instruccion->parametro3,instruccion->parametro4,instruccion->parametro5);
            liberar_pcb();
            break;       
         case IO_FS_READ:
            log_nico(logger2,"Instruccion Ejecutada: PID: %d- Ejecutando: %s - %s %s %s %s %s", pcb->pid,instruccion->opcode,instruccion->parametro1,instruccion->parametro2,instruccion->parametro3,instruccion->parametro4,instruccion->parametro5);
            ejecutar_IO_FS_READ(instruccion->parametro1,instruccion->parametro2,instruccion->parametro3,instruccion->parametro4,instruccion->parametro5);
            liberar_pcb();
            break;
        default:
            log_error(logger, "Operacion desconocida instruccion");          
        }
        liberar_instrucciones(instruccion);
}

void ejecutar_set (char* registro, char* valor){

    void* reg = obtener_registro(registro);
   
    if(reg != NULL){
        operar_con_registros(reg,NULL,registro,"set",atoi(valor));
    }else{
        log_error(logger,"Error al obtener el SET");
    }

    tengoAlgunaInterrupcion();

}

void ejecutar_sum (char* registro_desitino_char ,char* registro_origen_char) {

    void* registro_origen = obtener_registro(registro_origen_char);
    void* registro_destino = obtener_registro(registro_desitino_char);

    if (registro_origen != NULL && registro_destino != NULL)
    {
        operar_con_registros(registro_destino,registro_origen,registro_origen_char, "+" ,0);
    }else{
        log_error(logger,"Error al obtener el SUM");
    } 

    tengoAlgunaInterrupcion();  
}


void ejecutar_sub (char* registro_desitino_char, char* registro_origen_char){

    void* registro_origen = obtener_registro(registro_origen_char);
    void* registro_destino = obtener_registro(registro_desitino_char);

       if (registro_origen != NULL && registro_destino != NULL)
    {
        operar_con_registros(registro_destino,registro_origen,registro_desitino_char, "-" , 0);
    }else{
        log_error(logger,"Error al obtener el SUB");
    }

    tengoAlgunaInterrupcion();
}

void ejecutar_JNZ(char* registro, char* valor){

    void* reg = obtener_registro(registro);
    uint32_t regg = *(uint32_t*)reg;
    if (regg == 0){
        pcb->registros->PC += atoi(valor);
        tengoAlgunaInterrupcion();
    }

    tengoAlgunaInterrupcion();
}

void ejecutar_IO_GEN_SLEEP(char* interfazAUsar, char* tiempoDeTrabajo){

    int tiempo = atoi(tiempoDeTrabajo);
    t_paquete* paquete_a_kernel = crear_paquete(OPERACION_IO);
    enviar_pcb_a_kernel(paquete_a_kernel);
    enviar_paquete(paquete_a_kernel,config_cpu->SOCKET_KERNEL);
    eliminar_paquete(paquete_a_kernel);

    int respuesta;

    recv(config_cpu->SOCKET_KERNEL,&respuesta , sizeof(int), MSG_WAITALL);

    if(respuesta == 1){
        t_paquete* paquete = crear_paquete(IO_GEN_SLEEP_INT);
        //log_warning(logger, "OP: %i", IO_GEN_SLEEP_INT);
        agregar_a_paquete_string(paquete, interfazAUsar, strlen(interfazAUsar) + 1);
        agregar_a_paquete (paquete ,&tiempo, sizeof(int));
        //log_warning(logger, "Tiempo: %i", tiempo);
        enviar_paquete(paquete,config_cpu->SOCKET_KERNEL);
        eliminar_paquete(paquete);
    }else{log_error(logger , "Error en la respuesta de desalojo de I/O");}
}


void ejecutar_MOV_IN(char* registro_Datos ,char* registro_Direccion){

    void* reg_Direccion = obtener_registro(registro_Direccion);
    void* reg_Datos   = obtener_registro(registro_Datos);

    int tamanio_registro = espacio_de_registro(registro_Datos);

    int tamanio_registro_direcion = espacio_de_registro(registro_Direccion);

    int direccionLogica = encontrar_int(reg_Direccion, tamanio_registro_direcion);

    mmu = traducirDireccion(direccionLogica, tamanio_registro);

    int valor = comunicaciones_con_memoria_lectura();
    log_info(logger,"valor enviada por memoria: %i", valor);

    operar_con_registros(reg_Datos,NULL,registro_Datos,"set",valor);
    liberar_mmu();

    tengoAlgunaInterrupcion();
}

void ejecutar_MOV_OUT(char* Registro_Direccion, char* Registro_Datos) {

    void* reg_Direc = obtener_registro(Registro_Direccion);
    void* reg_Datos = obtener_registro(Registro_Datos);

    int tamanio_registro = espacio_de_registro(Registro_Datos); 
    int tamanio_logica = espacio_de_registro(Registro_Direccion);

    int direccionLogica = encontrar_int(reg_Direc, tamanio_logica);
    int regDAtos = encontrar_int(reg_Datos, tamanio_registro);   

    mmu = traducirDireccion(direccionLogica, tamanio_registro);

    if (comunicaciones_con_memoria_escritura(regDAtos) == 1) {
        log_info(logger, "Se pudo escribir correctamente");
    } else {
        log_error(logger, "No se pudo escribir en memoria");
    }

    liberar_mmu();

    tengoAlgunaInterrupcion();
}

void ejecutar_RESIZE(char* tam){
    int tamanio  = atoi(tam);
    
    send_agrandar_memoria(pcb->pid,tamanio);
    int valor = recv_agrandar_memoria();
    if(valor != -1){
        log_info(logger,"Se pudo agrandar correctamente");


        tengoAlgunaInterrupcion();
    }else{
        log_error(logger,"NO se pudo agrandar correctamente !!");
        t_paquete* paquete_a_kernel = crear_paquete(OUT_OF_MEMORY);
        enviar_pcb_a_kernel(paquete_a_kernel);
        enviar_paquete(paquete_a_kernel, config_cpu->SOCKET_KERNEL);
        eliminar_paquete(paquete_a_kernel);
    }

}


void ejecutar_COPY_STRING(char* tam) {

    int tamanio = atoi(tam);
    void* registroSI = obtener_registro("SI");
    void* registroDI = obtener_registro("DI");

    int registerSI = encontrar_int(registroSI, 4);
    int registreDI = encontrar_int(registroDI, 4);

    mmu = traducirDireccion(registerSI, tamanio);
    char* valor = comunicaciones_con_memoria_lectura_copy_string(); // Acordarse que lo cambie!!
    liberar_mmu(mmu);
    int tamm = strlen(valor);

    mmu = traducirDireccion(registreDI, tamm);

    if(comunicaciones_con_memoria_escritura_copy_string(valor) == 1) {
        log_info(logger, "Se pudo escribir correctamente");
    }else {
        log_error(logger, "No se pudo escribir en memoria");
    }
    liberar_mmu();
    free(valor);

    tengoAlgunaInterrupcion();
}

void ejecutar_WAIT(char* recurso){

    t_paquete* paquete_a_kernel = crear_paquete(WAIT);
    enviar_pcb_a_kernel(paquete_a_kernel);
    enviar_paquete(paquete_a_kernel, config_cpu->SOCKET_KERNEL);
    eliminar_paquete(paquete_a_kernel);

    t_paquete* paquete = crear_paquete(WAIT);
    agregar_a_paquete_string(paquete, recurso, strlen(recurso) + 1);
    enviar_paquete(paquete, config_cpu->SOCKET_KERNEL);
    eliminar_paquete(paquete);

    int respuesta = -1;
    recv(config_cpu->SOCKET_KERNEL, &respuesta , sizeof(int), MSG_WAITALL);
    if (respuesta == 1){
        tengoAlgunaInterrupcion();
    }else{
        liberar_pcb();
        return;
    } 

}

void ejecutar_SINGAL(char* recurso) {

    t_paquete* paquete_a_kernel = crear_paquete(SIGNAL);
    enviar_pcb_a_kernel(paquete_a_kernel);
    enviar_paquete(paquete_a_kernel, config_cpu->SOCKET_KERNEL);
    eliminar_paquete(paquete_a_kernel);

    t_paquete* paquete_signal = crear_paquete(SIGNAL);
    agregar_a_paquete_string(paquete_signal, recurso, strlen(recurso) + 1);
    enviar_paquete(paquete_signal, config_cpu->SOCKET_KERNEL);
    eliminar_paquete(paquete_signal);

    int respuesta = -1;
    recv(config_cpu->SOCKET_KERNEL, &respuesta, sizeof(int), MSG_WAITALL);
    if (respuesta == 1) {


        tengoAlgunaInterrupcion();
    } else {
        liberar_pcb();
        return;
    }
}


void ejecutar_IO_STDIN_READ(char* interfaz, char* registro_direccion, char* registro_tamanio) {


    void* registroDireccion = obtener_registro(registro_direccion);
    void* registroTamanio = obtener_registro(registro_tamanio);

    int tamanio_registro = espacio_de_registro(registro_tamanio);
    int tamanio_logica = espacio_de_registro(registro_direccion);

    int reg_Direc = encontrar_int(registroDireccion, tamanio_logica);
    int reg_Tamanio = encontrar_int(registroTamanio, tamanio_registro);

    mmu = traducirDireccion(reg_Direc, reg_Tamanio);
    t_paquete* paquete_std = crear_paquete(OPERACION_IO);
    enviar_pcb_a_kernel(paquete_std);
    enviar_paquete(paquete_std, config_cpu->SOCKET_KERNEL);
    eliminar_paquete(paquete_std);

    t_paquete* paquete_stdin = crear_paquete(IO_STDIN_READ_INT);
    solicitar_a_kernel_std(interfaz, paquete_stdin);
    liberar_mmu();

}

void ejecutar_IO_STDOUT_WRITE(char* interfaz, char* registro_direccion, char* registro_tamanio) {


    void* registroDireccion = obtener_registro(registro_direccion);
    void* registroTamanio = obtener_registro(registro_tamanio);

    int tamanio_registro = espacio_de_registro(registro_tamanio);
    int tamanio_logica = espacio_de_registro(registro_direccion);

    int reg_Direc = encontrar_int(registroDireccion, tamanio_logica);
    int reg_Tamanio = encontrar_int(registroTamanio, tamanio_registro);

    mmu = traducirDireccion(reg_Direc, reg_Tamanio);
    t_paquete* paquete_std = crear_paquete(OPERACION_IO);
    enviar_pcb_a_kernel(paquete_std);
    enviar_paquete(paquete_std, config_cpu->SOCKET_KERNEL);
    eliminar_paquete(paquete_std);

    t_paquete* paquete_stdout = crear_paquete(IO_STDOUT_WRITE_INT);
    solicitar_a_kernel_std(interfaz, paquete_stdout);
    liberar_mmu();


}


void ejecutar_IO_FS_CREATE(char* interfaz, char* nombre_archivo){
    t_paquete* paquete_IO = crear_paquete(OPERACION_IO);
    enviar_pcb_a_kernel(paquete_IO);
    enviar_paquete(paquete_IO, config_cpu->SOCKET_KERNEL);
    eliminar_paquete(paquete_IO);

    int respuesta;
    recv(config_cpu->SOCKET_KERNEL, &respuesta , sizeof(int), MSG_WAITALL);
    if (respuesta == 1){
    t_paquete* paquet_fs_create = crear_paquete(IO_FS_CREATE_INT);
    agregar_a_paquete_string(paquet_fs_create, interfaz, strlen(interfaz) + 1);
    agregar_a_paquete_string(paquet_fs_create, nombre_archivo, strlen(nombre_archivo) + 1);
    enviar_paquete(paquet_fs_create, config_cpu->SOCKET_KERNEL);
    eliminar_paquete(paquet_fs_create); 
    }
    
    
}

void ejecutar_IO_FS_DELETE(char* interfaz, char* nombre_archivo){
    t_paquete* paquete_IO = crear_paquete(OPERACION_IO);
    enviar_pcb_a_kernel(paquete_IO);
    enviar_paquete(paquete_IO, config_cpu->SOCKET_KERNEL);
    eliminar_paquete(paquete_IO);

    int respuesta;
    recv(config_cpu->SOCKET_KERNEL, &respuesta , sizeof(int), MSG_WAITALL);

    if(respuesta == 1) {
        t_paquete* paquete_delete = crear_paquete(IO_FS_DELETE_INT);
        agregar_a_paquete_string(paquete_delete, interfaz, strlen(interfaz ) + 1);
        agregar_a_paquete_string(paquete_delete, nombre_archivo, strlen(nombre_archivo ) + 1);  
        enviar_paquete(paquete_delete, config_cpu->SOCKET_KERNEL);
        eliminar_paquete(paquete_delete); 
    }
}

void ejecutar_IO_FS_TRUNCATE(char* interfaz, char* nombre_archivo, char* registro_tamanio){
    void* registroTamanio  = obtener_registro(registro_tamanio);

    int tamanio = espacio_de_registro(registro_tamanio);

    int reg_Tamanio = encontrar_int(registroTamanio, tamanio);

    t_paquete* paquete_IO = crear_paquete(OPERACION_IO);
    enviar_pcb_a_kernel(paquete_IO);
    enviar_paquete(paquete_IO, config_cpu->SOCKET_KERNEL);
    eliminar_paquete(paquete_IO);

    int respuesta;
    recv(config_cpu->SOCKET_KERNEL, &respuesta , sizeof(int), MSG_WAITALL);
    if (respuesta == 1){
    t_paquete* paquete = crear_paquete(IO_FS_TRUNCATE_INT);
    agregar_a_paquete_string(paquete, interfaz, strlen(interfaz) + 1);
    agregar_a_paquete_string(paquete, nombre_archivo, strlen(nombre_archivo) + 1);
    agregar_a_paquete(paquete, &reg_Tamanio, sizeof(int));
    enviar_paquete(paquete, config_cpu->SOCKET_KERNEL);
    eliminar_paquete(paquete);
    }
}

void ejecutar_IO_FD_WRITE(char* interfaz, char* nombre_archivo, char* registro_direccion, char* registro_tamanio, char* registro_puntero_archivo) {
    void* registroDireccion = obtener_registro(registro_direccion);
    int tamanio1 = espacio_de_registro(registro_direccion);
    int reg_Direc = encontrar_int(registroDireccion, tamanio1);

    void* registroTamanio = obtener_registro(registro_tamanio);
    int tamanio2 = espacio_de_registro(registro_tamanio);
    int reg_Tamanio = encontrar_int(registroTamanio, tamanio2);

    void* registroArchivo = obtener_registro(registro_puntero_archivo);
    int tamanio3 = espacio_de_registro(registro_puntero_archivo);
    int reg_Archi = encontrar_int(registroArchivo, tamanio3);

    mmu = traducirDireccion(reg_Direc, reg_Tamanio);

    t_paquete* paquete_IO = crear_paquete(OPERACION_IO);
    enviar_pcb_a_kernel(paquete_IO);
    enviar_paquete(paquete_IO, config_cpu->SOCKET_KERNEL);
    eliminar_paquete(paquete_IO);

    int respuesta;
    recv(config_cpu->SOCKET_KERNEL, &respuesta, sizeof(int), MSG_WAITALL);
    if (respuesta == 1) { 
        t_paquete* paquete = crear_paquete(IO_FS_WRITE_INT);
        agregar_a_paquete_string(paquete, interfaz, strlen(interfaz) + 1);
        agregar_a_paquete_string(paquete, nombre_archivo, strlen(nombre_archivo) + 1);
        agregar_a_paquete(paquete, &reg_Archi, sizeof(int));

        while (!list_is_empty(mmu->direccionFIsica)) {
            int* direccion_fisica = list_remove(mmu->direccionFIsica, 0);
            int* ptr_tamanio = list_remove(mmu->tamanio, 0);

            agregar_a_paquete(paquete, direccion_fisica, sizeof(int));
            agregar_a_paquete(paquete, ptr_tamanio, sizeof(int));

            free(direccion_fisica);
            free(ptr_tamanio);
        }

        enviar_paquete(paquete, config_cpu->SOCKET_KERNEL);
        eliminar_paquete(paquete);
    }
    liberar_mmu();
}

void ejecutar_IO_FS_READ(char* interfaz, char* nombre_archivo, char* registro_direccion, char* registro_tamanio, char* registro_puntero_archivo) {
    void* registroDireccion = obtener_registro(registro_direccion);
    int tamanio1 = espacio_de_registro(registro_direccion);
    int reg_Direc = encontrar_int(registroDireccion, tamanio1);

    void* registroTamanio = obtener_registro(registro_tamanio);
    int tamanio2 = espacio_de_registro(registro_tamanio);
    int reg_Tamanio = encontrar_int(registroTamanio, tamanio2);

    void* registroArchivo = obtener_registro(registro_puntero_archivo);
    int tamanio3 = espacio_de_registro(registro_puntero_archivo);
    int reg_Archi = encontrar_int(registroArchivo, tamanio3);

    mmu = traducirDireccion(reg_Direc, reg_Tamanio);
    t_paquete* paquete_IO = crear_paquete(OPERACION_IO);
    enviar_pcb_a_kernel(paquete_IO);
    enviar_paquete(paquete_IO, config_cpu->SOCKET_KERNEL);
    eliminar_paquete(paquete_IO);

    int respuesta;
    recv(config_cpu->SOCKET_KERNEL, &respuesta, sizeof(int), MSG_WAITALL);
    if (respuesta == 1){
    t_paquete* paquete = crear_paquete(IO_FS_READ_INT);
    agregar_a_paquete_string(paquete, interfaz, strlen(interfaz) + 1);
    agregar_a_paquete_string(paquete, nombre_archivo, strlen(nombre_archivo) + 1);
    agregar_a_paquete(paquete, &reg_Archi, sizeof(int));

    while (!list_is_empty(mmu->direccionFIsica)) {
        int* direccion_fisica = list_remove(mmu->direccionFIsica, 0);
        int* ptr_tamanio = list_remove(mmu->tamanio, 0);

        agregar_a_paquete(paquete, direccion_fisica, sizeof(int));
        agregar_a_paquete(paquete, ptr_tamanio, sizeof(int));

        free(direccion_fisica);
        free(ptr_tamanio);
    }

    enviar_paquete(paquete, config_cpu->SOCKET_KERNEL);
    eliminar_paquete(paquete);
    }
    
    
    liberar_mmu();
}


void liberar_pcb(){
    if (pcb != NULL) {
        if (pcb->registros != NULL) {
            free(pcb->registros);
            pcb->registros = NULL;  // Previene la doble liberación
        }
        free(pcb);
        pcb = NULL;  // Previene la doble liberación
    }
}

void liberar_mmu() {
    if (mmu == NULL) return;

    liberar_lista(mmu->num_pagina);
    liberar_lista(mmu->ofset);
    liberar_lista(mmu->direccionFIsica);
    liberar_lista(mmu->tamanio);

    free(mmu);
    mmu = NULL; // Evitar acceso posterior a memoria liberada
}

void liberar_instrucciones(t_instruccion* instruccion){
    if (instruccion) {
        free(instruccion->opcode);
        free(instruccion->parametro1);
        free(instruccion->parametro2);
        free(instruccion->parametro3);
        free(instruccion->parametro4);
        free(instruccion->parametro5);
        free(instruccion);
    }
}

void liberar_elemento(void* elemento) {
    if (elemento != NULL) {
        free(elemento);
         elemento = NULL; // Evitar acceso posterior a memoria liberada
    }
}

void liberar_lista(t_list* lista) {
    if (lista != NULL) {
        list_destroy_and_destroy_elements(lista, liberar_elemento);
    }
}