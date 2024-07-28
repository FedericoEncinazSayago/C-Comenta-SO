#include "protocolo.h"

t_paquete* crear_paquete(op_code operacion) {
	t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = operacion;
    
	crear_buffer(paquete);
	return paquete;
}

void crear_buffer(t_paquete* paquete) {
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio) {
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio);
	memcpy(paquete->buffer->stream + paquete->buffer->size, valor, tamanio);
	paquete->buffer->size += tamanio;
}


void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2 *sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete) {
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void* recibir_buffer(int* size, int socket_cliente) {
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if (recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void* serializar_paquete(t_paquete* paquete, int bytes) {
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(op_code));
	desplazamiento += sizeof(op_code);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);

	return magic;
}

void enviar_mensaje(char* mensaje, int socket_cliente) {
    t_paquete *paquete = crear_paquete(MENSAJE);

    agregar_a_paquete_string(paquete, mensaje, strlen(mensaje) + 1);

    enviar_paquete(paquete, socket_cliente);
    eliminar_paquete(paquete);
}

void recibir_mensaje(int socket_cliente) {
    int size;
    int tam;
    int desplazamiento = 0;
    char *buffer = recibir_buffer(&size, socket_cliente);

    // Leemos el tamaño del mensaje:
    memcpy(&tam, buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);

    // Reservamos memoria para el mensaje y copiamos el mensaje recibido:
    char* mensaje = malloc(tam);

    if (mensaje == NULL) {
        
        log_error(logger, "Error al asignar memoria para el mensaje");
        free(buffer);
        return;
    }

    memcpy(mensaje, buffer + desplazamiento, tam );

    log_info(logger, "Mensaje recibido: %s", mensaje);
    free(buffer);
    free(mensaje); // No olvides liberar la memoria del mensaje después de usarlo
}


void send_contexto_ejecucion(op_code operacion, int socket_cliente, t_pcb* proceso) {
    t_paquete* paquete = crear_paquete(operacion);
    agregar_a_paquete_PCB(paquete, proceso);
    agregar_a_paquete_registros(paquete, proceso->registros);

    enviar_paquete(paquete, socket_cliente);
    eliminar_paquete(paquete);
}

void agregar_a_paquete_PCB(t_paquete* paquete, t_pcb* proceso) {
    agregar_a_paquete(paquete, &proceso->pid, sizeof(int)); 
}

void agregar_a_paquete_registros(t_paquete* paquete, t_registro_cpu* registros) {
	agregar_a_paquete(paquete, &registros->PC, sizeof(uint32_t)); 
	agregar_a_paquete(paquete, &registros->AX, sizeof(uint32_t));
    agregar_a_paquete(paquete, &registros->BX, sizeof(uint8_t));
    agregar_a_paquete(paquete, &registros->CX, sizeof(uint8_t));
    agregar_a_paquete(paquete, &registros->DX, sizeof(uint8_t));
	agregar_a_paquete(paquete, &registros->EAX, sizeof(uint32_t));
	agregar_a_paquete(paquete, &registros->EBX, sizeof(uint32_t));
	agregar_a_paquete(paquete, &registros->ECX, sizeof(uint32_t));
	agregar_a_paquete(paquete, &registros->EDX, sizeof(uint32_t));
	agregar_a_paquete(paquete, &registros->SI, sizeof(uint32_t)); 
	agregar_a_paquete(paquete, &registros->DI, sizeof(uint32_t));
}

void agregar_a_paquete_lista_string(t_paquete* paquete, t_list* archivos_abiertos) {
    int cantidad_archivos = list_size(archivos_abiertos);

    for(int i = 0; i < cantidad_archivos; i++) {
        char* archivo = list_get(archivos_abiertos, i);
        agregar_a_paquete_string(paquete, archivo, strlen(archivo) + 1);
    }
}

void agregar_a_paquete_string(t_paquete* paquete, char* cadena, int tamanio) {
    int cadena_length = string_length(cadena);
    size_t size = sizeof(int);  // Tamaño en bytes de un entero
    
    // Expandir el tamaño del buffer del paquete para acomodar la longitud de la cadena
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + size);
    memcpy(paquete->buffer->stream + paquete->buffer->size, &cadena_length, size);
    paquete->buffer->size += size;

    // Expandir el tamaño del buffer para acomodar la cadena
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio);
    memcpy(paquete->buffer->stream + paquete->buffer->size, cadena, tamanio);
    paquete->buffer->size += tamanio;
}


t_pcb* rcv_contexto_ejecucion(int socket_cliente) {
    int size;
    int desplazamiento = 0;

    void *buffer = recibir_buffer(&size, socket_cliente);
    if (buffer == NULL) {
        return NULL;
    }

    t_pcb *pcb = malloc(sizeof(t_pcb));
    if (pcb == NULL) {
        perror("Error al asignar memoria para t_pcb");
        free(buffer);
        return NULL;
    }

    pcb->registros = malloc(sizeof(t_registro_cpu));
    if (pcb->registros == NULL) {
        perror("Error al asignar memoria para t_registro_cpu");
        free(pcb);
        free(buffer);
        return NULL;
    }

    memcpy(&(pcb->pid), buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);

    memcpy(&(pcb->registros->PC), buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(&(pcb->registros->AX), buffer + desplazamiento, sizeof(uint8_t));
    desplazamiento += sizeof(uint8_t);

    memcpy(&(pcb->registros->BX), buffer + desplazamiento, sizeof(uint8_t));
    desplazamiento += sizeof(uint8_t);

    memcpy(&(pcb->registros->CX), buffer + desplazamiento, sizeof(uint8_t));
    desplazamiento += sizeof(uint8_t);

    memcpy(&(pcb->registros->DX), buffer + desplazamiento, sizeof(uint8_t));
    desplazamiento += sizeof(uint8_t);

    memcpy(&(pcb->registros->EAX), buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(&(pcb->registros->EBX), buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(&(pcb->registros->ECX), buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(&(pcb->registros->EDX), buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(&(pcb->registros->SI), buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(&(pcb->registros->DI), buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    free(buffer);
    return pcb;
}

t_list *recv_list(int socket_cliente) {
	int tamanio;
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* lista = list_create();
	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);

		
		char* valor = malloc(tamanio);
		memcpy(valor, buffer + desplazamiento, tamanio);

		desplazamiento += tamanio;

		list_add(lista, valor);
	}

	free(buffer);

	return lista;
}

t_list* recibir_paquete(int socket_cliente)
{
	int tamanio;
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* lista = list_create();

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);

		
		char* valor = malloc(tamanio);
		memcpy(valor, buffer + desplazamiento, tamanio);

		desplazamiento += tamanio;

		list_add(lista, valor);
	}

	free(buffer);

	return lista;
}

void generar_handshake(int socket, char *server_name, char *ip, char *puerto) {
    int32_t handshake = 1;
    int32_t result;

	op_code cod_op = HANDSHAKE; 
	send(socket, &cod_op, sizeof(op_code), 0);
    
    send(socket, &handshake, sizeof(int32_t), 0);
	recv(socket, &result, sizeof(int32_t), MSG_WAITALL);

    if(result == 0) 
        log_info(logger, "Handshake exitoso con %s", server_name);
    else {
        log_error(logger, "Error en el handshake con %s", server_name);
        exit(EXIT_FAILURE);
    }
}

void recibir_handshake(int socket) {
	int32_t handshake;
	int32_t resultOk = 0;
	int32_t resultError = -1;

	recv(socket, &handshake, sizeof(int32_t), MSG_WAITALL);
	
	if (handshake == 1) 
		send(socket, &resultOk, sizeof(int32_t), 0);
	else 
		send(socket, &resultError, sizeof(int32_t), 0);
}

//Creamos una funcion que envie el archivo pseudo y el pid del proceso desde kernel a memoria para que pueda ser utilizado en la lectura de pseudocodigo

void send_archi_pid(int socket_cliente, char *path, int pid) {
    int tamanio = strlen(path) + 1; //Calculamos el largo de archivo_path
    int size = sizeof(int) + tamanio + sizeof(int);  //Arbitrariamente elegimos el largo del paquete a enviar ya que sabemos lo que mandamos
    void *buffer = malloc(size); //Reservamos memoria para el buffer
    int desplazamiento = 0; //Init del desplazamiento que será utilizado para leer dato a dato

    memcpy(buffer + desplazamiento, &tamanio, sizeof(int)); //Copiamos el dato, ente caso del tamaño de un int
    desplazamiento += sizeof(int); //Desplzamos el puntero para que avance al siguiente bloque de datos, sin pisar el dato anterior

    memcpy(buffer + desplazamiento, path, tamanio); 
    desplazamiento += tamanio; 

    memcpy(buffer + desplazamiento, &pid, sizeof(int));

    enviar_buffer(buffer, size, socket_cliente); //Enviamos el buffer con el tamaño del paquete a enviar que será recibido luego
    free(buffer); // Liberamos el buffer xq no somos petes
}

//Creamos una funcion que reciba el archivo pseudo y el pid del proceso que será utilizado para la lectura de pseudocodigo de instrucciones en memoria 
void enviar_buffer(void* buffer, size_t tamanio, int socket){
	size_t bytes_enviados;

	bytes_enviados = send(socket, buffer,tamanio,0);
	if(bytes_enviados){
		log_error(logger, "Error al enviar los datos");
		return;
	}
	
}

void rcv_string(char **string, int socket_cliente) {
    int size;

    recv(socket_cliente, &size, sizeof(int), MSG_WAITALL);
    *string = malloc(size);
    recv(socket_cliente, *string, size, MSG_WAITALL);
}

// void recv_archi_pid(int socket_cliente, char **path, int* pid){ // Se usan punteros xq necesitamos modificar las variables que se pasan por parametro
// 	int size;
// 	int desplazamiento = 0;
// 	void *buffer;
// 	int tamaño;

// 	buffer = recibir_buffer(&size, socket_cliente); //Recibimos el buffer que antes enviamos
// 	while (desplazamiento < size) // Leemos el buffer hasta que lleguemos al final
// 	{
// 		memcpy(&tamaño, buffer + desplazamiento, sizeof(int)); //Averiguamos que tan largo es el string (path)
// 		desplazamiento += sizeof(int);
// 		path = malloc(tamaño); // Una vez que sabemos el largo, podemos reservar el espacio de memoraia del mismo 
								
// 		memcpy(*path, buffer + desplazamiento, tamaño); // Copiamos todo el path de una en la variable "*path"
// 		desplazamiento += tamaño;

// 		memcpy(pid, buffer + desplazamiento, sizeof(int));
// 		desplazamiento += sizeof(int);

// 	}
// 	free(buffer);
// }

// void recv_archi_pid(int socket_cliente, char **path, int* pid) {
//     int size;
//     int desplazamiento = 0;
//     void *buffer;
//     int tamanio;

//     buffer = recibir_buffer(&size, socket_cliente); // Recibimos el buffer que antes enviamos
//     if (size > 0) { // Ensure there's data to process
//         memcpy(&tamanio, buffer + desplazamiento, sizeof(int)); // Averiguamos que tan largo es el string (path)
//         desplazamiento += sizeof(int);

//         *path = malloc(tamanio + 1); // Allocate memory for the path, +1 for null terminator
//         if (*path == NULL) {
//             free(buffer);
//             return; // Handle memory allocation failure
//         }

//         memcpy(*path, buffer + desplazamiento, tamanio); // Copiamos todo el path en la variable "*path"
//         (*path)[tamanio] = '\0'; // Null-terminate the string
//         desplazamiento += tamanio;

//         memcpy(pid, buffer + desplazamiento, sizeof(int)); // Copy the PID
//         desplazamiento += sizeof(int);
//     }

//     free(buffer);
// }

//Funcion para recibir el PC y el PID de un proceso
void recibir_program_counter(int socket_cpu, int *pid, int *program_counter){

    int tamaño;
    int desplazamiento = 0;
    void *buffer;

    buffer = recibir_buffer(&tamaño, socket_cpu); //Si llega a fallar funcion recibir_paquete
    while(desplazamiento < tamaño){

        memcpy(program_counter, buffer + desplazamiento, sizeof(int));//memcpy copia los datos del buffer al program counter
        desplazamiento += sizeof(int);//el desplazamiento se incrementa en el tamaño de un entero para moverse al siguiente conjunto de datos en el buffer.
                                      															
        memcpy(pid, buffer+desplazamiento, sizeof(int));//copiar los datos del bufer las pid
        desplazamiento += sizeof(int);
    }
    free(buffer);
}

//Retardo pedido
void retardo_pedido(int tiempo_de_espera){
    usleep(tiempo_de_espera * 1000); //*1000 es para pasarlo a milisegundos ya que usleep usa microsegundos
    //cambiar por el cofig
}