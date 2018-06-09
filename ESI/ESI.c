#include "ESI.h"

/* Conexiones */
void crearClienteCoor() {
	ConectarAServidor(coordinador_puerto,coordinador_ip);
	EnviarHandshake(socket_coordinador,ESI);
}

void crearClientePlanif() {

	socket_planificador = ConectarAServidor(planificador_puerto,planificador_ip);
	EnviarHandshake(socket_planificador,ESI);

	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	Paquete paquete;

	while (RecibirPaqueteCliente(socket_planificador, ESI, &paquete) > 0) {
		switch (paquete.header.tipoMensaje) {
		case t_SIGUIENTELINEA: {
			printf("Recibi un siguiente linea %s\n","Wachin");
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
			printf("Recibi un Handshake de %s\n","Planificador");
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


/* Archivo configuración */
void setearValores(t_config * archivoConfig) {
 	planificador_puerto = config_get_int_value(archivoConfig, "PLANIFICADOR_PUERTO");
 	planificador_ip = strdup(config_get_string_value(archivoConfig, "PLANIFICADOR_IP"));
 	coordinador_puerto = config_get_int_value(archivoConfig, "COORDINADOR_PUERTO");
 	coordinador_ip = strdup(config_get_string_value(archivoConfig, "COORDINADOR_IP"));

 	log_info(logger,"Se inicio cargo correctamente el archivo de configuración.");
 }


/* Operaciones ESI */
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
			EnviarDatosTipo(socket_coordinador, ESI, datos, tamanio, t_GET);
			log_info(logger,"Para el script: %s se ejecuto el comando GET, para la clave %s",
					filename,parsed.argumentos.GET.clave);
			break;
		case SET:
			tamanio = strlen(parsed.argumentos.SET.clave) + strlen(parsed.argumentos.SET.valor) + 2;
			datos = malloc(tamanio);
			strcpy(datos, parsed.argumentos.SET.clave);
			strcpy(datos +(strlen(parsed.argumentos.SET.clave) + 1), parsed.argumentos.SET.valor);
			EnviarDatosTipo(socket_coordinador, ESI, datos, tamanio, t_SET);
			log_info(logger, "Para el script: %s se ejecuto el comando SET, para la clave %s y el valor %s",
					filename, parsed.argumentos.SET.clave,parsed.argumentos.SET.valor);
			break;
		case STORE:
			tamanio = strlen(parsed.argumentos.STORE.clave) + 1;
			datos = malloc(tamanio);
			strcpy(datos, parsed.argumentos.STORE.clave);
			EnviarDatosTipo(socket_coordinador, ESI, datos, tamanio,t_STORE);
			log_info(logger,"Para el script: %s se ejecuto el comando STORE, para la clave %s",
					filename, parsed.argumentos.STORE.clave);
			break;
		default:
			log_info(logger, "No pude interpretar <%s>\n", line);
			log_info(logger,"Se le envio al planificador la orden de matar al ESI.");
			EnviarDatosTipo(socket_planificador, ESI, NULL, 0, t_ABORTARESI);
			matarESI();
		}
		destruir_operacion(parsed);
	} else {
		log_info(logger, "La linea <%s> no es valida\n", line);
		log_info(logger,"Se le envio al planificador la orden de matar al ESI.");
		EnviarDatosTipo(socket_planificador, ESI, NULL, 0, t_ABORTARESI);
		matarESI();
	}
	free(datos);
}

void matarESI(){
	close(socket_coordinador);
	close(socket_planificador);
	exit(1);
}


/* Manejo de Archivos*/
const char* get_filename(const char* path){

	const char *file = strrchr(path,'/');
	if(!file || file == path) return "";
	return file +1;
}

void abrirArchivo(char* path){
	fp = fopen(path, "r");
	if (fp == NULL) {
		log_error(logger, "Error al abrir el archivo: %s",strerror(errno));
		log_info(logger,"Se le envio al planificador la orden de matar al ESI.");
		perror("Error al abrir el archivo: ");
		exit(1);
	}
	filename = get_filename(path);
}

/* Función PRUEBA*/
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
