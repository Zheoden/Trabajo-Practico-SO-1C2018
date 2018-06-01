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

void matarESI(){
	EnviarDatosTipo(socket_planificador, ESI, NULL, 0, t_ABORTARESI);
}

void parsear() {
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	void* datos;
	int tamanio;

	IDEsiActual = malloc(strlen("000") + 1);
	strcpy(IDEsiActual, "000");

	t_esi_operacion parsed;

	while (1) {

		usleep(5 * 1000000); //1 segundo
		char* file = getNextFile();
		if ((strncmp(file, "" "", 2))) {

			IDEsiActual = incrementarID(IDEsiActual);

			char* ruta =
					malloc(
							strlen(
									"/home/utnso/Proyectos/tp-2018-1c-PC-citos/ESI/Esis/")
									+ strlen(file) + 1);
			strcpy(ruta, "/home/utnso/Proyectos/tp-2018-1c-PC-citos/ESI/Esis/");
			strcpy(
					ruta
							+ strlen(
									"/home/utnso/Proyectos/tp-2018-1c-PC-citos/ESI/Esis/"),
					file);

			fp = fopen(ruta, "r");
			if (fp == NULL) {
				log_error(logger, "Error al abrir el archivo: %s",
						strerror(errno));
				//matarESI();
				log_info(logger,
						"Se le envio al planificador la orden de matar al ESI.");
				fclose(fp);
			} else {
				log_info(logger,
						"Cargo correctamente el nuevo archivo. Se va a proceder a procesarlo.");
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
						//EnviarDatosTipo(socket_coordinador, ESI, datos, tamanio, t_GET);*/
						log_info(logger,
								"Para el ESI con el id: %s, se ejecuto el comando GET, para la clave %s",
								//	"00","clave");
								IDEsiActual, parsed.argumentos.GET.clave);
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
						//EnviarDatosTipo(socket_coordinador, ESI, datos, tamanio, t_SET);*/
						log_info(logger,
								"Para el ESI con el id: %s, se ejecuto el comando SET, para la clave %s y el valor %s",
								IDEsiActual, parsed.argumentos.SET.clave,
								parsed.argumentos.SET.valor);
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
						//EnviarDatosTipo(socket_coordinador, ESI, datos, tamanio,t_STORE);*/
						log_info(logger,
								"Para el ESI con el id: %s, se ejecuto el comando STORE, para la clave %s",
								IDEsiActual, parsed.argumentos.STORE.clave);
						break;
					default:
						//matarESI();
						log_info(logger, "No pude interpretar <%s>\n", line);
						log_info(logger,
								"Se le envio al planificador la orden de matar al ESI.");
						fclose(fp);
					}
					destruir_operacion(parsed);
				} else {
					//matarESI();
					log_info(logger, "La linea <%s> no es valida\n", line);
					log_info(logger,
							"Se le envio al planificador la orden de matar al ESI.");
					fclose(fp);
				}
			}
			fclose(fp);
			free(file);
			free(datos);
			free(ruta);
		}
	}

	if (line) {
		free(line);
		log_info(logger, "Se libero la memoria de la linea actual.");
	}
}

char* incrementarID(char *ID){
	int i, begin, tail, len;
	int neg = (*ID == '-');
	char tgt = neg ? '0' : '9';

	/* special case: "-1" */
	if (!strcmp(ID, "-1")) {
		ID[0] = '0', ID[1] = '\0';
		return ID;
	}

	len = strlen(ID);
	begin = (*ID == '-' || *ID == '+') ? 1 : 0;

	/* find out how many digits need to be changed */
	for (tail = len - 1; tail >= begin && ID[tail] == tgt; tail--);

	if (tail < begin && !neg) {
		/* special case: all 9s, string will grow */
		if (!begin) ID = realloc(ID, len + 2);
		ID[0] = '1';
		for (i = 1; i <= len - begin; i++) ID[i] = '0';
		ID[len + 1] = '\0';
	} else if (tail == begin && neg && ID[1] == '1') {
		/* special case: -1000..., so string will shrink */
		for (i = 1; i < len - begin; i++) ID[i] = '9';
		ID[len - 1] = '\0';
	} else { /* normal case; change tail to all 0 or 9, change prev digit by 1*/
		for (i = len - 1; i > tail; i--)
			ID[i] = neg ? '9' : '0';
		ID[tail] += neg ? -1 : 1;
	}

	return ID;
}

char* getNextFile(){

	char* ruta = malloc(strlen("/home/utnso/Proyectos/tp-2018-1c-PC-citos/ESI/Esis/")+1);
	strcpy(ruta,"/home/utnso/Proyectos/tp-2018-1c-PC-citos/ESI/Esis/");
	DIR* directorio = opendir("/home/utnso/Proyectos/tp-2018-1c-PC-citos/ESI/Esis/");

	if (directorio != NULL){

		struct dirent *ent;
		while( (ent = readdir(directorio)) != NULL ){
			if( (strncmp(ent->d_name, ".", 1)) && (strncmp(get_filename_extension(ent->d_name), "bak", 3)) ){
				char* nuevo_nombre = malloc(strlen(ent->d_name)+strlen(".bak")+1);
				strcpy(nuevo_nombre, ent->d_name);
				strcpy(nuevo_nombre+strlen(ent->d_name),".bak");

				char* directorio_nuevo = malloc(strlen(ruta) + strlen(nuevo_nombre) + 2);
				strcpy(directorio_nuevo, ruta);
				strcpy(directorio_nuevo+strlen(ruta),nuevo_nombre);

				char* directorio_viejo = malloc(strlen(ruta) + strlen(ent->d_name) + 2);
				strcpy(directorio_viejo, ruta);
				strcpy(directorio_viejo+strlen(ruta),ent->d_name);

				rename(directorio_viejo,directorio_nuevo);

				printf("%s\n", nuevo_nombre);

				closedir(directorio);
				free(directorio_viejo);
				free(directorio_nuevo);
				free(ruta);

				return nuevo_nombre;
			}

		}
	    closedir(directorio);
	}else{
		log_error(logger, "Se detectó el siguiente error al abrir el directorio: %s", strerror(errno));
	}
	return "";
}

const char* get_filename_extension(const char* filename){
	const char *dot = strrchr(filename,'.');
	if(!dot || dot == filename) return "";
	return dot +1;
}
