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


	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	Paquete paquete;

	while (RecibirPaqueteCliente(socket_planificador, ESI, &paquete) > 0) {
		switch (paquete.header.tipoMensaje) {
		case t_SIGUIENTELINEA: {
			if ((read = getline(&line, &len, fp)) != EOF) {
				parsear(line);
			}else{
				if (line) {
					free(line);
					log_info(logger, "Se libero la memoria de la linea actual.");
				}
			}
		}
		break;
		case t_ABORTARESI: {
			matarESI();
		}
		break;
		case t_HANDSHAKE: {
			//revisar handshake
		}
		break;
		}
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

void parsear(char* line) {
	void* datos;
	int tamanio;
	t_esi_operacion parsed;

	parsed = parse(line);
	if (parsed.valido) {
		switch (parsed.keyword) {
		case GET:
			tamanio = strlen(parsed.argumentos.GET.clave)+1;
			datos = malloc(tamanio);
			strcpy(datos, parsed.argumentos.GET.clave);
			//EnviarDatosTipo(socket_coordinador, ESI, datos, tamanio, t_GET);
			log_info(logger,"Para el script: %s se ejecuto el comando GET, para la clave %s",
					filename,parsed.argumentos.GET.clave);
			break;
		case SET:
			tamanio = strlen(parsed.argumentos.SET.clave) + strlen(parsed.argumentos.SET.valor) + 2;
			datos = malloc(tamanio);
			strcpy(datos, parsed.argumentos.SET.clave);
			strcpy(datos +(strlen(parsed.argumentos.SET.clave) + 1), parsed.argumentos.SET.valor);
			//EnviarDatosTipo(socket_coordinador, ESI, datos, tamanio, t_SET);
			log_info(logger, "Para el script: %s se ejecuto el comando SET, para la clave %s y el valor %s",
					filename, parsed.argumentos.SET.clave,parsed.argumentos.SET.valor);
			break;
		case STORE:
			tamanio = strlen(parsed.argumentos.STORE.clave) + 1;
			datos = malloc(tamanio);
			strcpy(datos, parsed.argumentos.STORE.clave);
			//EnviarDatosTipo(socket_coordinador, ESI, datos, tamanio,t_STORE);
			log_info(logger,"Para el script: %s se ejecuto el comando STORE, para la clave %s",
					filename, parsed.argumentos.STORE.clave);
			break;
		default:
			log_info(logger, "No pude interpretar <%s>\n", line);
			log_info(logger,"Se le envio al planificador la orden de matar al ESI.");
			//matarESI();
		}
		destruir_operacion(parsed);
	} else {
		log_info(logger, "La linea <%s> no es valida\n", line);
		log_info(logger,"Se le envio al planificador la orden de matar al ESI.");
		//matarESI();
	}

	printf("%s\n",(char*)datos);
	free(datos);
}

void matarESI(){
	close(socket_coordinador);
	close(socket_planificador);
	exit(1);
}

void abrirArchivo(char* path){
	fp = fopen(path, "r");
	if (fp == NULL) {
		log_error(logger, "Error al abrir el archivo: %s",strerror(errno));
		log_info(logger,"Se le envio al planificador la orden de matar al ESI.");
		perror("Error al abrir el archivo: ");
		exit(1);
		//matarESI();
	}
	filename = get_filename(path);
}

void foo(){
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	while ((read = getline(&line, &len, fp)) != EOF) {
		parsear(line);
	}
	if (line) {
		free(line);
		log_info(logger, "Se libero la memoria de la linea actual.");
	}
}

const char* get_filename(const char* path){
	const char *file = strrchr(path,'/');
	if(!file || file == path) return "";
	return file +1;
}
