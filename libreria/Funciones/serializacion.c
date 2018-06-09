#include "serializacion.h"


bool EnviarPaquete(int socketCliente, Paquete* paquete) {
	int cantAEnviar = sizeof(Header) + paquete->header.tamanioMensaje;
	void* datos = malloc(cantAEnviar);
	memcpy(datos, &(paquete->header), sizeof(Header));
	if (paquete->header.tamanioMensaje > 0){ //No sea t_HANDSHAKE
		memcpy(datos + sizeof(Header), (paquete->mensaje), paquete->header.tamanioMensaje);
	}
	int enviado = 0; //bytes enviados
	int totalEnviado = 0;
	bool valor_retorno=true;
	do {
		enviado = send(socketCliente, datos + totalEnviado, cantAEnviar - totalEnviado, 0);
		totalEnviado += enviado;
		if(enviado==-1){
			valor_retorno=false;
			break;
		}
	} while (totalEnviado != cantAEnviar);
	free(datos);
	return valor_retorno;
}

bool EnviarDatosTipo(int socketFD, proceso quienEnvia, void* datos, int tamDatos, t_protocolo tipoMensaje){
	Paquete* paquete = malloc(sizeof(Paquete));
	paquete->header.tipoMensaje = tipoMensaje;
	paquete->header.quienEnvia = quienEnvia;
	uint32_t r = 0;
	bool valor_retorno;
	if(tamDatos<=0 || datos==NULL){
		paquete->header.tamanioMensaje = sizeof(uint32_t);
		paquete->mensaje = &r;
	} else {
		paquete->header.tamanioMensaje = tamDatos;
		paquete->mensaje=datos;
	}
	valor_retorno=EnviarPaquete(socketFD, paquete);
	free(paquete);
	return valor_retorno;
}

bool EnviarMensaje(int socketFD, char* msg, proceso quienEnvia) {
	Paquete paquete;
	paquete.header.quienEnvia = quienEnvia;
	paquete.header.tipoMensaje = t_HANDSHAKE;
	paquete.header.tamanioMensaje = string_length(msg) + 1;
	paquete.mensaje = msg;
	return EnviarPaquete(socketFD, &paquete);
}

bool EnviarHandshake(int socketFD, proceso quienEnvia) {
	Paquete* paquete = malloc(sizeof(Header));
	Header header;
	header.tipoMensaje = t_HANDSHAKE;
	header.tamanioMensaje = 0;
	header.quienEnvia = quienEnvia;
	paquete->header = header;
	return EnviarPaquete(socketFD, paquete);

	free(paquete);
}

bool EnviarDatos(int socketFD, proceso quienEnvia, void* datos, int tamDatos) {
	return EnviarDatosTipo(socketFD, quienEnvia, datos, tamDatos, t_HANDSHAKE);
}

bool RecibirHandshake(int socketFD, proceso quienEnvia) {
	Header header;
	int resul = RecibirDatos(&header, socketFD, sizeof(Header));
	if (resul > 0) { // si no hubo error en la recepcion
		if (header.quienEnvia == quienEnvia) {
			if (header.tipoMensaje == t_HANDSHAKE){
//				nombreProceso aux = getNombreDelProceso(quienEnvia);
//				char paraImprimir[aux.tamanio];
//				strcpy(paraImprimir,aux.nombre);
				printf("\nConectado con el servidor %d\n",header.quienEnvia);
				return true;
/*				printf("%d\n",header.quienEnvia);
				printf("%d\n",header.tipoMensaje);
				printf("%d\n",header.tamanioMensaje);
*/
			}else{
				perror("Error de Conexion, no se recibio un t_HANDSHAKE\n");
			}
		} else
			perror("Error, no se recibio un t_HANDSHAKE del servidor esperado\n");
	}
	return false;
}

int RecibirDatos(void* paquete, int socketFD, uint32_t cantARecibir) {
	void* datos = malloc(cantARecibir);
	int recibido = 0;
	int totalRecibido = 0;

	do {
		recibido = recv(socketFD, datos + totalRecibido, cantARecibir - totalRecibido, 0);
		totalRecibido += recibido;
	} while (totalRecibido != cantARecibir && recibido > 0);
	memcpy(paquete, datos, cantARecibir);
	free(datos);
	if (recibido < 0) {
		printf("Cliente Desconectado\n");
		close(socketFD); // ¡Hasta luego!
	} else if (recibido == 0) {
		printf("Fin de Conexion en socket %d\n", socketFD);
		close(socketFD); // ¡Hasta luego!
	}
	return recibido;
}

int RecibirPaqueteServidor(int socketFD, proceso quienManda, Paquete* paquete) {
	paquete->mensaje = NULL;
	int resul = RecibirDatos(&(paquete->header), socketFD, sizeof(Header));
	if (resul > 0) { //si no hubo error
		if (paquete->header.tipoMensaje == t_HANDSHAKE) { //vemos si es un t_HANDSHAKE
			nombreProceso aux = getNombreDelProceso(paquete->header.quienEnvia);
			char paraImprimir[aux.tamanio];
			strcpy(paraImprimir,aux.nombre);
			printf("Se establecio conexion con %s\n", paraImprimir);
			EnviarHandshake(socketFD, quienManda); // paquete->header.emisor
		} else if (paquete->header.tamanioMensaje > 0){ //recibimos un payload y lo procesamos (por ej, puede mostrarlo)
			paquete->mensaje = malloc(paquete->header.tamanioMensaje);
			resul = RecibirDatos(paquete->mensaje, socketFD, paquete->header.tamanioMensaje);
		}
	}
	return resul;
}

/*int RecibirPaqueteESI(int socketFD, proceso quienManda, Paquete* paquete) {
	void* aux = paquete->mensaje;
	(t_ESIplanificador) aux= NULL;
	void* resul = RecibirDatos(&(paquete->header), socketFD, sizeof(Header));
	if (resul > 0) { //si no hubo error
		if (paquete->header.tipoMensaje == t_HANDSHAKE) { //vemos si es un t_HANDSHAKE
			nombreProceso aux = getNombreDelProceso(paquete->header.quienEnvia);
			char paraImprimir[aux.tamanio];
			strcpy(paraImprimir,aux.nombre);
			printf("Se establecio conexion con %s\n", paraImprimir);
			Enviart_HANDSHAKE(socketFD, quienManda); // paquete->header.emisor
		} else if (paquete->header.tamanioMensaje > 0){ //recibimos un payload y lo procesamos (por ej, puede mostrarlo)
			paquete->mensaje = malloc(paquete->header.tamanioMensaje);
			resul = RecibirDatos(paquete->mensaje, socketFD, paquete->header.tamanioMensaje);
		}
	}
	return resul;
}*/

int RecibirPaqueteCliente(int socketFD, proceso quienEnvia, Paquete* paquete) {
	paquete->mensaje = NULL;
	int resul = RecibirDatos(&(paquete->header), socketFD, sizeof(Header));
	if (resul > 0 && paquete->header.tipoMensaje != t_HANDSHAKE && paquete->header.tamanioMensaje > 0) { //si no hubo error ni es un t_HANDSHAKE
		paquete->mensaje = malloc(paquete->header.tamanioMensaje);
		resul = RecibirDatos(paquete->mensaje, socketFD, paquete->header.tamanioMensaje);
	}
	return resul;
}

nombreProceso getNombreDelProceso(proceso proceso){
	nombreProceso aux;
	switch (proceso) {
		case ESI:
			aux.tamanio=strlen("ESI");
			strcpy(aux.nombre, "ESI");
			break;
		case COORDINADOR:
			aux.tamanio=strlen("COORDINADOR");
			strcpy(aux.nombre, "COORDINADOR");
			break;
		case PLANIFICADOR:
			aux.tamanio=strlen("PLANIFICADOR");
			strcpy(aux.nombre, "PLANIFICADOR");
			break;
		case INSTANCIA:
			aux.tamanio=strlen("INSTANCIA");
			strcpy(aux.nombre, "INSTANCIA");
			break;
		default:
			aux.tamanio=strlen("ERROR");
			strcpy(aux.nombre, "ERROR");
			break;
	}
	return aux;
}
