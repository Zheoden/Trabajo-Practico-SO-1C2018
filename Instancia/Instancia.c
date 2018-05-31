#include "Instancia.h"

void inicializar(){
	entradas_administrativas = list_create();
}

void crearCliente(void) {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(client_ip);
	direccionServidor.sin_port = htons(client_puerto);

	socket_coordinador = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(socket_coordinador,(void*) &direccionServidor,sizeof(direccionServidor))!=0){
		log_error(logger,"No se pudo conectar: %s",strerror(errno));
	}
	log_info(logger,"Se establecio conexion con el Coordinador correctamente");
	iniciarManejoDeEntradas();
	/*
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
	}*/

}

void setearValores(t_config * archivoConfig) {
 	client_puerto = config_get_int_value(archivoConfig, "CLIENT_PUERTO");
 	client_ip = strdup(config_get_string_value(archivoConfig, "CLIENT_IP"));
	algoritmo_de_reemplazo = strdup(config_get_string_value(archivoConfig, "ALGORITMO_DE_REEMPLAZO"));
	punto_de_montaje = strdup(config_get_string_value(archivoConfig, "PUNTO_DE_MONTAJE"));
	nombre_de_la_instancia = strdup(config_get_string_value(archivoConfig, "NOMBRE_DE_LA_INSTANCIA"));
	intervalo_de_dump = config_get_int_value(archivoConfig, "INTERVALO_DE_DUMP");

	log_info(logger,"Se inicio cargo correctamente el archivo de configuración.");
	log_info(logger,"Se inicio la Instancia con el siguiente Algoritmo de Reemplazo: %s",algoritmo_de_reemplazo);
 }

bool handshakeInstanciaCoordinador(){
	EnviarDatosTipo(socket_coordinador, INSTANCIA,(void*)cantidad_de_entradas, sizeof(cantidad_de_entradas),t_HANDSHAKE);
	EnviarDatosTipo(socket_coordinador, INSTANCIA,(void*)tamanio_entrada, sizeof(tamanio_entrada),t_HANDSHAKE);
	return true;
}

void iniciarManejoDeEntradas(){
	pthread_t hilo;
	log_info(logger,"Se inicio un hilo para el manejo de Entradas.");
//	pthread_create(&hilo, NULL, (void *) manejarEntradas, NULL);
//	pthread_detach(hilo);
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
			tabla_entradas = malloc((cantidad_de_entradas * tamanio_entrada)+1);
			int i;
			for (i = 0; i < cantidad_de_entradas; i++) {
				tabla_entradas[i] = malloc(tamanio_entrada);
				strcpy(tabla_entradas[i], "null");
			}
			handshakeInstanciaCoordinador();
			log_info(logger,"Se envio un Handshake al Coordiandor");
		}
			break;
		case t_STORE: {
			log_info(logger,"Se recibio un STORE del Coordinador, se va a pasar a procesar.");
			log_info(logger,"Se proceso correctamente el STORE.");
		}
			break;
		case t_SET: {
			log_info(logger,"Se recibio un SET del Coordinador, se va a pasar a procesar.");
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
			list_add(entradas_administrativas, nueva);
			log_info(logger,"Se agrego la nueva entrada en la lista de Entradas Administrativas.");
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
			log_info(logger,"Se proceso correctamente el SET y se envio al Coordinador la respuesta del SET.");
			free(clave);
			free(valor);
			free(valueAux);

		}
			break;

		}
		if (paquete.mensaje != NULL) {
			free(paquete.mensaje);
			log_info(logger,"Se libero la memoria del paquete.");
		}

	}

}

int ceilDivision(int lengthValue) {
	double cantidadEntradas;
	cantidadEntradas = (lengthValue + tamanio_entrada -1 )/ tamanio_entrada;
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

void verificarPuntoMontaje(){

	DIR* directorio_de_montaje = opendir(punto_de_montaje);
	if (ENOENT == errno){
			//el directorio no existe
			log_info(logger,"Existe el punto de montaje, se va a proceder a crear el punto de montaje.");
			mkdir(punto_de_montaje,0777);
			log_info(logger,"El punto de montaje se creo exitosamente.");
	}
	if (directorio_de_montaje != NULL){
	    /* Directory exists. */
		/* El directorio existe. Hay que recorrer todos sus archivos y por cada uno de ellos
		 *  verificar el nombre del archivo(clave) y lo que haya dentro es el value(valor)
		 * Despues hay que añadirlo a la tabla de entradas y mandarle un mensaje a COORDINADOR con las entradas que tiene */
	    closedir(directorio_de_montaje);
	}else{
		log_error(logger, "Se detectó el siguiente error al abrir el directorio: %s", strerror(errno));
	}
}

//no se hace en coordinador, ya que es propio de la instancia, y no depende del coordinador. es un metodo de backup.
void dump(){
	while(1){
		usleep(20000000);//20 segundos
		int i,j;
		//Recoro las entradas para saber cuales tengo
		for (i=0;  i< list_size(entradas_administrativas); i++) {
			//Agarro las entradas de a 1
			t_AlmacenamientoEntradaAdministrativa* actual = (t_AlmacenamientoEntradaAdministrativa*)list_get(entradas_administrativas, i);
			char* directorio_actual = malloc(strlen(punto_de_montaje) + strlen(actual->clave) + 2);
			strcpy(directorio_actual, punto_de_montaje);
			strcpy(directorio_actual+strlen(punto_de_montaje),actual->clave);

			//hago un malloc para el valor que voy a sacar de la tabla de entrada
			char* valor=malloc(actual->tamanio);
			int tamanioPegado=0; //Variable auxiliar para cuando el nodo ocupa mas de 2 entradas

			for (j = actual->index; j < (actual->index + actual->entradasOcupadas);j++) {
				if((actual->index + actual->entradasOcupadas) -1 == j){
					strcpy(valor+tamanioPegado, tabla_entradas[j]);
				}else{
					strcpy(valor+tamanioPegado, tabla_entradas[j]);
					tamanioPegado+=tamanio_entrada;
				}
			}

			FILE* file_a_crear = fopen(directorio_actual,"w+");
			fwrite(valor,actual->tamanio,sizeof(char),file_a_crear);

			free(valor);
			fclose(file_a_crear);
		}
	}
}

//funcion para probar el dump
void cargarDatosFicticios(char* unaClave, char* unValor) {

	log_info(logger,"Se recibio un SET del Coordinador, se va a pasar a procesar.");
	char*clave =malloc(strlen(unaClave) + 1);
	strcpy(clave, unaClave);
	char* valor = malloc(strlen(unValor) + 1);
	strcpy(valor, unValor);

	t_AlmacenamientoEntradaAdministrativa* nueva = malloc(sizeof(t_AlmacenamientoEntradaAdministrativa));
	nueva->clave = malloc(strlen(clave) + 1);
	strcpy(nueva->clave, clave);
	nueva->entradasOcupadas = ceilDivision(strlen(valor));
	nueva->tamanio = strlen(valor);//nueva->tamanio = strlen(valor) + strlen(clave);

	nueva->index = getFirstIndex(nueva->entradasOcupadas);
	list_add(entradas_administrativas, nueva);
	log_info(logger,"Se agrego la nueva entrada en la lista de Entradas Administrativas.");
	int i;
	char *valueAux = malloc(strlen(valor) + 1);
	strcpy(valueAux, valor);
	for (i = nueva->index; i < (nueva->index + nueva->entradasOcupadas);i++) {
		if ((nueva->index + nueva->entradasOcupadas) - 1 == i) {
			strcpy(tabla_entradas[i], valueAux);
			break;
		}
		strncpy(tabla_entradas[i], valueAux, tamanio_entrada);
		valueAux += tamanio_entrada;
	}
	log_info(logger,"Se proceso correctamente el SET y se envio al Coordinador la respuesta del SET.");
	free(clave);
	free(valor);
//	free(valueAux);


}
//funcion para probar el dump
void foo(){
	tabla_entradas = malloc((cantidad_de_entradas * tamanio_entrada)+1);
	int i;
	for (i = 0; i < cantidad_de_entradas; i++) {
		tabla_entradas[i] = malloc(tamanio_entrada);
		strcpy(tabla_entradas[i], "null");
	}
}
