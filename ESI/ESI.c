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

void inicializar(){
	todos_los_esis = list_create();
	IDEsiActual = malloc(4);
	strcpy(IDEsiActual, "000");
	ESI_Actual=NULL;

	IDAux = malloc(4);
	strcpy(IDAux, "001");

	log_info(logger,"Se inicio inicializaron las listas correctamente.");
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
	Paquete paquete;

	t_esi_operacion parsed;

	//Pregunta si el planificador da la orden de leer
	//	while (RecibirPaqueteCliente(socket_coordinador, ESI, &paquete) > 0) {
	//		switch(paquete.header.quienEnvia){
	//		case PLANIFICADOR:{
	//			switch(paquete.header.tipoMensaje){
	//			case t_SIGUIENTELINEA:{
	//recibo el ID del ESI que tengo que ejecutar
	while(1){
		//char * IDAux = malloc(paquete.header.tamanioMensaje);
		//strcpy(IDAux,paquete.mensaje);
		usleep(0.5 * 1000000); //.5 segundos
		Esi* aux = nextEsi(IDAux);
		if(aux != NULL){
			char* ruta = malloc(strlen("/home/utnso/Proyectos/tp-2018-1c-PC-citos/ESI/Esis/") + strlen(aux->file) + 1);
			strcpy(ruta, "/home/utnso/Proyectos/tp-2018-1c-PC-citos/ESI/Esis/");
			strcpy(ruta + strlen("/home/utnso/Proyectos/tp-2018-1c-PC-citos/ESI/Esis/"),aux->file);

			fp = fopen(ruta, "r");
			if (fp == NULL) {
				log_error(logger, "Error al abrir el archivo: %s",strerror(errno));
				log_info(logger,"Se le envio al planificador la orden de matar al ESI.");
				fclose(fp);
			}

			printf("%s\n",aux->ID);
			printf("%s\n",aux->file);
			printf("%ld\n",aux->linea);
			printf("%s\n","----------------");

			fseek(fp,aux->linea,SEEK_SET);

			if ((read = getline(&line, &len, fp)) != EOF) {
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
						//EnviarDatosTipo(socket_coordinador, ESI, datos, tamanio, t_GET);
						log_info(logger,"Para el ESI con el id: %s, se ejecuto el comando GET, para la clave %s",
								aux->ID, parsed.argumentos.GET.clave);
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
						//EnviarDatosTipo(socket_coordinador, ESI, datos, tamanio, t_SET);
						log_info(logger, "Para el ESI con el id: %s, se ejecuto el comando SET, para la clave %s y el valor %s",
								aux->ID, parsed.argumentos.SET.clave,parsed.argumentos.SET.valor);
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
						//EnviarDatosTipo(socket_coordinador, ESI, datos, tamanio,t_STORE);
						log_info(logger,"Para el ESI con el id: %s, se ejecuto el comando STORE, para la clave %s",
								aux->ID, parsed.argumentos.STORE.clave);
						break;
					default:
						//matarESI();
						log_info(logger, "No pude interpretar <%s>\n", line);
						log_info(logger,"Se le envio al planificador la orden de matar al ESI.");
						fclose(fp);
					}
					destruir_operacion(parsed);
				} else {
					//matarESI();
					log_info(logger, "La linea <%s> no es valida\n", line);
					log_info(logger,"Se le envio al planificador la orden de matar al ESI.");
					fclose(fp);
				}

				//tengo que agregar el esi a la lista de todos los esis modificado
				fflush(fp);
				aux->linea = ftell(fp);
				list_add(todos_los_esis,aux);
				fclose(fp);

			} else{ /*Cierra el If de readLine*/

				//todo lo que necesito cuando se termina un esi

				fclose(fp);
				renombrarArchivo(aux->file,".bak");
				IDAux = incrementarID(IDAux);
				free(aux);
				//matarESI();
				//break;
			}
		}
	}

	/*			} //Cierra el case de la siguiente linea
			break;
			case t_CAMBIARESI:{ // Cambia al ESI con el ID indicador por el coordinador

			}
			break;
			case t_ABORTARESI:{ // Mata el ESI Actual
				char * IDAux = malloc(paquete.header.tamanioMensaje);
				strcpy(IDAux,paquete.mensaje);
				Esi* aux = nextEsi(IDAux);
				free(aux);

			}
			break;
			case t_HANDSHAKE:{ // Se carga el primer ESI
				ESI_Actual = list_get(todos_los_esis,0);
			}
			break;
			} //Cierra el switch del tipo de mensaje
		}
		break;
		case COORDINADOR:{

		}
		break;
		} //Cierra el switch de quien Envia

	} //Cierre del while del Planificador
	 */
	free(datos);
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
	DIR* directorio = opendir("/home/utnso/Proyectos/tp-2018-1c-PC-citos/ESI/Esis/");
	if (directorio != NULL){
		struct dirent *ent;
		while( (ent = readdir(directorio)) != NULL ){
			if( (strncmp(ent->d_name, ".", 1)) && (strncmp(get_filename_extension(ent->d_name), "bak", 3)) && (strncmp(get_filename_extension(ent->d_name), "pcs", 3)) ){
				char* nuevo_nombre = malloc(strlen(ent->d_name)+1);
				strcpy(nuevo_nombre, ent->d_name);
				closedir(directorio);
				return renombrarArchivo(nuevo_nombre,".pcs");
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

char* renombrarArchivo(char* file, char* ext){

	char* ruta = malloc(strlen("/home/utnso/Proyectos/tp-2018-1c-PC-citos/ESI/Esis/")+1);
	strcpy(ruta,"/home/utnso/Proyectos/tp-2018-1c-PC-citos/ESI/Esis/");

	char* nuevo_nombre = malloc(strlen(file)+strlen(ext)+1);
	strcpy(nuevo_nombre, file);
	strcpy(nuevo_nombre+strlen(file),ext);

	char* directorio_nuevo = malloc(strlen(ruta) + strlen(nuevo_nombre) + 2);
	strcpy(directorio_nuevo, ruta);
	strcpy(directorio_nuevo+strlen(ruta),nuevo_nombre);

	char* directorio_viejo = malloc(strlen(ruta) + strlen(file) + 2);
	strcpy(directorio_viejo, ruta);
	strcpy(directorio_viejo+strlen(ruta),file);

	rename(directorio_viejo,directorio_nuevo);


	free(directorio_viejo);
	free(directorio_nuevo);

	return nuevo_nombre;

}

void inicializarSiguienteEsi(){
	FILE * fp;
	while (1) {
		usleep(2 * 1000000); //2 segundos
		char* file = getNextFile();
		if ((strncmp(file, "" "", 2))) {
			//Carga basica del proximo esi que se va a ejecutar
			IDEsiActual = incrementarID(IDEsiActual);
			char* ruta = malloc(strlen("/home/utnso/Proyectos/tp-2018-1c-PC-citos/ESI/Esis/") + strlen(file) + 1);
			strcpy(ruta, "/home/utnso/Proyectos/tp-2018-1c-PC-citos/ESI/Esis/");
			strcpy(ruta + strlen("/home/utnso/Proyectos/tp-2018-1c-PC-citos/ESI/Esis/"),file);

			fp = fopen(ruta, "r");
			if (fp == NULL) {
				log_error(logger, "Error al abrir el archivo: %s",strerror(errno));
				//matarESI();
				log_info(logger,"Se le envio al planificador la orden de matar al ESI.");
				fclose(fp);
			} else {
				log_info(logger,"Cargo correctamente el nuevo archivo. Se va a proceder a procesarlo.");
				fclose(fp);

				Esi* EsiActual = malloc(sizeof(long int) + strlen(IDEsiActual) + strlen(file) + 3);

				EsiActual->ID = malloc(4);
				strcpy(EsiActual->ID,IDEsiActual);

				EsiActual->file = malloc(strlen(file));
				strcpy(EsiActual->file, file);

				EsiActual->linea = 0;

				list_add(todos_los_esis,EsiActual);
			}
		}
	}
}

void cargarEsis(){
	pthread_t unHilo;
	log_info(logger,"Se inicio un hilo para cargar todos los esis al Sistema.");
	pthread_create(&unHilo, NULL, (void *) inicializarSiguienteEsi,NULL);
	pthread_detach(unHilo);
}

Esi* nextEsi(char* ID){
	bool compareID(Esi* e){
		return !strncmp(e->ID, ID, 3);
	}
	return list_remove_by_condition(todos_los_esis,(void*)compareID);
}


