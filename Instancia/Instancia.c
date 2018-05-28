#include "Instancia.h"

int crearCliente(void) {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(client_ip);
	direccionServidor.sin_port = htons(client_puerto);


	socket_coordinador = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(socket_coordinador,(void*) &direccionServidor,sizeof(direccionServidor))!=0){
		perror("No se pudo conectar");
		return 1;
	}

	while(1){
		char mensaje [1000];
		scanf("%s", mensaje);
		send(socket_coordinador, mensaje, strlen(mensaje), 0);


     	char* buffer = malloc(30);
     	int bytesRecibidos = recv(socket_coordinador,buffer, 25, 0);
     	if (bytesRecibidos <= 0) {
     		perror("El chabon se desconecto o bla bla bla");
     		return 1;
     	}
     	buffer[bytesRecibidos] = '\0';
     	printf("me llegaron %d bytes con %s\n", bytesRecibidos, buffer);
     	free(buffer);
	}

	return 0;
}

void setearValores(t_config * archivoConfig) {

 	client_puerto = config_get_int_value(archivoConfig, "CLIENT_PUERTO");
 	client_ip = strdup(config_get_string_value(archivoConfig, "CLIENT_IP"));
	algoritmo_de_reemplazo = strdup(config_get_string_value(archivoConfig, "ALGORITMO_DE_REEMPLAZO"));
	punto_de_montaje = strdup(config_get_string_value(archivoConfig, "PUNTO_DE_MONTAJE"));
	nombre_de_la_instancia = strdup(config_get_string_value(archivoConfig, "NOMBRE_DE_LA_INSTANCIA"));
	intervalo_de_dump = config_get_int_value(archivoConfig, "INTERVALO_DE_DUMP");
 }
bool handshakeInstanciaCoordinador(){
	EnviarDatosTipo(socket_coordinador, INSTANCIA,(void*)cantidad_de_entradas, sizeof(cantidad_de_entradas),t_HANDSHAKE);
	EnviarDatosTipo(socket_coordinador, INSTANCIA,(void*)tamanio_entrada, sizeof(tamanio_entrada),t_HANDSHAKE);
	return true;
}
void manejarEntradas() {

	Paquete paquete;
	void* datos;
	while (RecibirPaqueteCliente(socket_coordinador, INSTANCIA, &paquete) > 0) {
		datos = paquete.mensaje;
		switch (paquete.header.tipoMensaje) {
		case t_HANDSHAKE: {
			tamanio_entrada = *((int*) datos);
			datos += sizeof(int);
			cantidad_de_entradas = *((int*) datos);
			datos += sizeof(int);
			tabla_entradas = malloc(cantidad_de_entradas * sizeof(char*));
			int i;
			for (i = 0; i < cantidad_de_entradas; i++) {
				tabla_entradas[i] = malloc(tamanio_entrada);
				strcpy(tabla_entradas[i], "null");
			}

			handshakeInstanciaCoordinador();
		}
			break;
		case t_STORE: {

		}
			break;
		case t_SET: {
			char*clave = malloc(strlen(datos) + 1);
			strcpy(clave, datos);
			datos += strlen(datos) + 1;
			char* valor = malloc(strlen(datos) + 1);
			strcpy(valor, datos);
			t_AlmacenamientoEntradaAdministrativa* nueva = malloc(sizeof(t_AlmacenamientoEntradaAdministrativa));
			nueva->clave = malloc(strlen(clave) + 1);
			strcpy(nueva->clave, clave);
			nueva->entradasOcupadas = ceilDivision(strlen(valor));
			nueva->tamanio = strlen(valor);//nueva->tamanio = strlen(valor) + strlen(clave);
			nueva->index = getFirstIndex(nueva->entradasOcupadas);
			list_add(entradas_administrativa, nueva);
			int i;
			char *valueAux = malloc(strlen(valor) + 1);
			strcpy(valueAux, valor);
			for (i = nueva->index; i < (nueva->index + nueva->entradasOcupadas);
					i++) {
				if ((nueva->index + nueva->entradasOcupadas) - 1 == i) {
					strcpy(tabla_entradas[i], valueAux);
					break;
				}
				strncpy(tabla_entradas[i], valueAux, tamanio_entrada);
				valueAux += tamanio_entrada;
			}
			EnviarDatosTipo(socket_coordinador, INSTANCIA, clave, strlen(clave) + 1, t_SET);
			free(clave);
			free(valor);
			free(valueAux);

		}
			break;

		}
		if (paquete.mensaje != NULL) {
			free(paquete.mensaje);
		}
	}

}

int ceilDivision(int lengthValue) {
	double cantidadEntradas;
	cantidadEntradas = lengthValue / tamanio_entrada;
	return cantidadEntradas;
}

int getFirstIndex (int entradasValue){
	int i;
	for (i=0;  i< cantidad_de_entradas; i++) {
		if(!strcmp(tabla_entradas[i],"null") &&  tabla_entradas[entradasValue-1]){
			int aux;
			bool cumple=true;
			//evaluo valores intermedios entre el inicio y el supuesto final (entradasValue-1)
			for(aux=i+1; aux< entradasValue; aux++){
				if(strcmp(tabla_entradas[aux],"null")){
					cumple=false;
					break;
				}
			}
			if(cumple){
				return i;
			}
		}
	}
	return -1;
}
