#include "ESI.h"

int crearClienteCoor() {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(coordinador_ip);
	direccionServidor.sin_port = htons(coordinador_puerto);

	int cliente = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(cliente,(void*) &direccionServidor,sizeof(direccionServidor))!=0){
		perror("No se pudo conectar");
//		return 1;
	}

	while(1){
		char mensaje [1000];
		scanf("%s", mensaje);
		send(cliente, mensaje, strlen(mensaje), 0);


     	char* buffer = malloc(30);
     	int bytesRecibidos = recv(cliente,buffer, 25, 0);
     	if (bytesRecibidos <= 0) {
     		perror("El chabon se desconecto o bla bla bla");
//     		return 1;
     	}
     	buffer[bytesRecibidos] = '\0';
     	printf("me llegaron %d bytes con %s\n", bytesRecibidos, buffer);
     	free(buffer);
	}

//	return 0;
}

int crearClientePlanif() {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(planificador_ip);
	direccionServidor.sin_port = htons(planificador_puerto);

	int cliente = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(cliente,(void*) &direccionServidor,sizeof(direccionServidor))!=0){
		perror("No se pudo conectar");
		return 1;
	}

	EnviarHandshake(cliente,ESI);

	while(1){
		char mensaje [1000];
		scanf("%s", mensaje);
		send(cliente, mensaje, strlen(mensaje), 0);


     	char* buffer = malloc(30);
     	int bytesRecibidos = recv(cliente,buffer, 25, 0);
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

void atenderPlanificador(){
	pthread_t hilo;
	pthread_create(&hilo, NULL, (void *) crearClientePlanif, NULL);
	pthread_detach(hilo);
}

void atenderCoordinador(){
	pthread_t unHilo;
	pthread_create(&unHilo, NULL, (void *) crearClienteCoor,NULL);
	pthread_detach(unHilo);
}


void setearValores(t_config * archivoConfig) {
 	planificador_puerto = config_get_int_value(archivoConfig, "PLANIFICADOR_PUERTO");
 	planificador_ip = strdup(config_get_string_value(archivoConfig, "PLANIFICADOR_IP"));
 	coordinador_puerto = config_get_int_value(archivoConfig, "COORDINADOR_PUERTO");
 	coordinador_ip = strdup(config_get_string_value(archivoConfig, "COORDINADOR_IP"));
 }

int cantidadDeApariciones(char * cadena, char separador){
	int i;
	int cont = 0;
	for(i=0;i < strlen(cadena);i++){
		if (cadena[i] == separador){
			cont++;
		}
	}
	return cont;
}

