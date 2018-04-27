#include "Instancia.h"

int crearCliente(void) {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(client_ip);
	direccionServidor.sin_port = htons(client_puerto);


	int cliente = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(cliente,(void*) &direccionServidor,sizeof(direccionServidor))!=0){
		perror("No se pudo conectar");
		return 1;
	}

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
     	buffer[bytesRecibidos] = "\0";
     	printf("me llegaron %d bytes con %s\n", bytesRecibidos, buffer);
     	free(buffer);
	}

	return 0;
}
void leerConfig(char * configPath) {
 	leerArchivoDeConfiguracion(configPath);
 //free(configPath);
 	log_info(logger, "Archivo de configuracion leido correctamente");
 }
void leerArchivoDeConfiguracion(char * configPath) {
 	t_config * archivoConfig;

 	archivoConfig = config_create(configPath);

 	if (archivoConfig == NULL){
 		perror("[ERROR] Archivo de configurarchcion no encontrado");
 		log_error(logger,"Archivo de configurarchcion no encontrado");
 	}

 	setearValores(archivoConfig);
 	config_destroy(archivoConfig);
 }

void setearValores(t_config * archivoConfig) {

 	client_puerto = config_get_int_value(archivoConfig, "CLIENT_PUERTO");
 	client_ip = strdup(config_get_string_value(archivoConfig, "CLIENT_IP"));
 }
