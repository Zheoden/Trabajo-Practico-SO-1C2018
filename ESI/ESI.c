#include "ESI.h"

void crearClienteCoor() {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(coordinador_ip);
	direccionServidor.sin_port = htons(coordinador_puerto);

	socket_coordinador = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(socket_coordinador,(void*) &direccionServidor,sizeof(direccionServidor))!=0){
		log_error(logger,"No se pudo conectar: %s",strerror(errno));
	}

	EnviarHandshake(socket_coordinador,ESI);

	while(1){
		char mensaje [1000];
		scanf("%s", mensaje);
		send(socket_coordinador, mensaje, strlen(mensaje), 0);


     	char* buffer = malloc(30);
     	int bytesRecibidos = recv(socket_coordinador,buffer, 25, 0);
     	if (bytesRecibidos <= 0) {
     		log_error(logger,"Se desconecto el socket: %d",socket_coordinador);
     	}
     	buffer[bytesRecibidos] = '\0';
     	printf("me llegaron %d bytes con %s\n", bytesRecibidos, buffer);
     	free(buffer);
	}

}

void crearClientePlanif() {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(planificador_ip);
	direccionServidor.sin_port = htons(planificador_puerto);

	socket_planificador = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(socket_planificador,(void*) &direccionServidor,sizeof(direccionServidor))!=0){
		log_error(logger,"No se pudo conectar: %s",strerror(errno));
	}

	EnviarHandshake(socket_planificador,ESI);

	while(1){
		char mensaje [1000];
		scanf("%s", mensaje);
		send(socket_planificador, mensaje, strlen(mensaje), 0);


     	char* buffer = malloc(30);
     	int bytesRecibidos = recv(socket_planificador,buffer, 25, 0);
     	if (bytesRecibidos <= 0) {
     		log_error(logger,"Se desconecto el socket: %d",socket_planificador);
     	}
     	buffer[bytesRecibidos] = '\0';
     	log_error(logger,"me llegaron %d bytes con %s",bytesRecibidos, buffer);

     	free(buffer);
	}

}

void atenderPlanificador(){
	pthread_t hilo;
	log_info(logger,"Se inicio un hilo para manejar la comunicacion con el Planificador.");
	pthread_create(&hilo, NULL, (void *) crearClientePlanif, NULL);
	pthread_detach(hilo);
}

void atenderCoordinador(){
	pthread_t unHilo;
	log_info(logger,"Se inicio un hilo para manejar la comunicacion con el Coordinador.");
	pthread_create(&unHilo, NULL, (void *) crearClienteCoor,NULL);
	pthread_detach(unHilo);
}

void setearValores(t_config * archivoConfig) {
 	planificador_puerto = config_get_int_value(archivoConfig, "PLANIFICADOR_PUERTO");
 	planificador_ip = strdup(config_get_string_value(archivoConfig, "PLANIFICADOR_IP"));
 	coordinador_puerto = config_get_int_value(archivoConfig, "COORDINADOR_PUERTO");
 	coordinador_ip = strdup(config_get_string_value(archivoConfig, "COORDINADOR_IP"));

 	log_info(logger,"Se inicio cargo correctamente el archivo de configuración.");
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

void matarESI(){
	EnviarDatosTipo(socket_planificador, ESI, NULL, 0, t_ABORTARESI);
}

void parsear(int argc, char **argv) {
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	void* datos;
	int tamanio;

	t_esi_operacion parsed;

	fp = fopen("/home/utnso/Proyectos/tp-2018-1c-PC-citos/ESI/script.esi", "r");
	if (fp == NULL) {
		log_error(logger,"Error al abrir el archivo: %s",strerror(errno));
		matarESI();
		log_info(logger,"Se le envio al planificador la orden de matar al ESI.");
		log_info(logger,"Se va a pasar a cerrar el archivo %s",fp);
		fclose(fp);
	}

	while ((read = getline(&line, &len, fp)) != -1) {
		parsed = parse(line);
		if (parsed.valido) {
			switch (parsed.keyword) {
			case GET:
				tamanio = strlen(IDEsiActual)
						+ strlen(parsed.argumentos.GET.clave) + 2;
				datos = malloc(tamanio);
				strcpy(datos, IDEsiActual);
				datos += strlen(IDEsiActual) + 1;
				strcpy(datos, parsed.argumentos.GET.clave);
				datos += strlen(parsed.argumentos.GET.clave) + 1;
				datos -= tamanio;
				EnviarDatosTipo(socket_coordinador, ESI, datos, tamanio, t_GET);
				log_info(logger,"Para el ESI con el id: %s, se ejecuto el comando GET, para la clave %s",
						IDEsiActual,parsed.argumentos.GET.clave);
				break;
			case SET:

				tamanio = strlen(IDEsiActual)
						+ strlen(parsed.argumentos.SET.clave)
						+ strlen(parsed.argumentos.SET.valor) + 3;
				datos = malloc(tamanio);
				strcpy(datos, IDEsiActual);
				datos += strlen(IDEsiActual) + 1;
				strcpy(datos, parsed.argumentos.SET.clave);
				datos += strlen(parsed.argumentos.SET.clave) + 1;
				strcpy(datos, parsed.argumentos.SET.valor);
				datos += strlen(parsed.argumentos.SET.valor) + 1;
				datos -= tamanio;
				EnviarDatosTipo(socket_coordinador, ESI, datos, tamanio, t_SET);
				log_info(logger,"Para el ESI con el id: %s, se ejecuto el comando SET, para la clave %s y el valor %s",
						IDEsiActual,parsed.argumentos.SET.clave,parsed.argumentos.SET.valor);
				break;
			case STORE:
				tamanio = strlen(IDEsiActual)
						+ strlen(parsed.argumentos.STORE.clave) + 2;
				datos = malloc(tamanio);
				strcpy(datos, IDEsiActual);
				datos += strlen(IDEsiActual) + 1;
				strcpy(datos, parsed.argumentos.STORE.clave);
				datos += strlen(parsed.argumentos.STORE.clave) + 1;
				datos -= tamanio;
				EnviarDatosTipo(socket_coordinador, ESI, datos, tamanio,
						t_STORE);

				printf("STORE\tclave: <%s>\n", parsed.argumentos.STORE.clave);
				log_info(logger,"Para el ESI con el id: %s, se ejecuto el comando STORE, para la clave %s",
						IDEsiActual,parsed.argumentos.STORE.clave);
				break;
			default:
				matarESI();
				log_info(logger,"No pude interpretar <%s>\n", line);
				log_info(logger,"Se le envio al planificador la orden de matar al ESI.");
				fclose(fp);
			}
			destruir_operacion(parsed);
		} else {
			matarESI();
			log_info(logger,"La linea <%s> no es valida\n", line);
			log_info(logger,"Se le envio al planificador la orden de matar al ESI.");
			fclose(fp);
		}

	}
	log_info(logger,"Se va a pasar a cerrar el archivo %s",fp);
	fclose(fp);
	if (line){
		free(line);
		log_info(logger,"Se libero la memoria de la linea actual.");
	}
}
