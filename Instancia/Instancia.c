#include "Instancia.h"

/* Se inicializan las entradas administrativas */
void inicializar(){
	entradas_administrativas = list_create();
}

/* Se setean los valores en el archivo de configuración */
void setearValores(t_config * archivoConfig) {
 	coordinador_puerto = config_get_int_value(archivoConfig, "COORDINADOR_PUERTO");
 	coordinador_ip = strdup(config_get_string_value(archivoConfig, "COORDINADOR_IP"));
	algoritmo_de_reemplazo = strdup(config_get_string_value(archivoConfig, "ALGORITMO_DE_REEMPLAZO"));
	punto_de_montaje = strdup(config_get_string_value(archivoConfig, "PUNTO_DE_MONTAJE"));
	nombre_de_la_instancia = strdup(config_get_string_value(archivoConfig, "NOMBRE_DE_LA_INSTANCIA"));
	intervalo_de_dump = config_get_int_value(archivoConfig, "INTERVALO_DE_DUMP");

	log_info(logger,"Se inicio cargo correctamente el archivo de configuración.");
	log_info(logger,"Se inicio la Instancia con el siguiente Algoritmo de Reemplazo: %s",algoritmo_de_reemplazo);
 }

/* Creación de hilos */
void iniciarManejoDeEntradas(){
	pthread_t hilo;
	log_info(logger,"Se inicio un hilo para el manejo de Entradas.");
	pthread_create(&hilo, NULL, (void *) manejarEntradas, NULL);
	pthread_detach(hilo);
}

void iniciarDump(){
	pthread_t hilo;
	log_info(logger,"Se inicio un hilo para el manejo de Entradas.");
	pthread_create(&hilo, NULL, (void *) dump, NULL);
	pthread_detach(hilo);
}

void atenderCoordinador(){
	pthread_t unHilo;
	log_info(logger,"Se inicio un hilo para manejar la comunicacion con el Coordinador.");
	pthread_create(&unHilo, NULL, (void *) crearCliente,NULL);
	pthread_detach(unHilo);
}

/* Conexión con el Coordinador */
void crearCliente() {
	socket_coordinador = ConectarAServidor(coordinador_puerto,coordinador_ip);
	printf("Me conecté al Coordinador\n");
	EnviarHandshake(socket_coordinador,INSTANCIA);
	log_info(logger,"Se envio un Handshake al Coordiandor");
	//iniciarManejoDeEntradas();
	manejarEntradas();
}

void manejarEntradas() {

	Paquete paquete;
	void* datos;
	while (RecibirPaqueteCliente(socket_coordinador, INSTANCIA, &paquete) > 0) {
		datos = malloc(paquete.header.tamanioMensaje);
		datos = paquete.mensaje;
		switch (paquete.header.tipoMensaje) {
		case t_CONFIGURACIONINSTANCIA: {
			tamanio_entrada = *((int*) paquete.mensaje);
			datos += sizeof(int);
			cantidad_de_entradas = *((int*) datos);
			inicializarTabla();
			verificarPuntoMontaje();
		}
		break;
		case t_SOLICITUDNOMBRE: {
			int tamanioNombre = strlen(nombre_de_la_instancia)+1;
			void *nombre = malloc(tamanioNombre);
			strcpy(nombre,nombre_de_la_instancia);
			EnviarDatosTipo(socket_coordinador, INSTANCIA, nombre, tamanioNombre, t_IDENTIFICACIONINSTANCIA);
			log_info(logger,"Se recibio una solicitud de Nombre.");
			free(nombre);
		}
		break;
		case t_STORE: {
			log_info(logger,"Se recibio un STORE del Coordinador, se va a pasar a procesar.\n");

			printf("Se recibio un STORE del Coordinador, se va a pasar a procesar.\n");
			int aux = strlen(datos) + 1;
			char*clave = malloc(aux);
			strcpy(clave, datos);


			//Funcion Auxiliar
			bool buscarClave(t_AlmacenamientoEntradaAdministrativa* unaEntrada){
				return !strcmp(unaEntrada->clave, clave);
			}

			//Verifico si la clave ya existe en la tabla, si existe limpio su valor y la borro de la tabla
			t_AlmacenamientoEntradaAdministrativa*  instanciaAReemplazar = (t_AlmacenamientoEntradaAdministrativa*)list_find(entradas_administrativas, (void*)buscarClave);
			guardarAArchivo(instanciaAReemplazar);
			liberarMemoria(instanciaAReemplazar);

			log_info(logger,"Se proceso correctamente el STORE.");
			EnviarDatosTipo(socket_coordinador, INSTANCIA, datos, aux, t_RESPUESTASTORE);
			EnviarDatosTipo(socket_coordinador, INSTANCIA, datos, aux, t_CLAVEBORRADA);
			free(clave);
		}
		break;
		case t_SET: {
			printf("Se recibio un SET del Coordinador, se va a pasar a procesar.\n");
			log_info(logger,"Se recibio un SET del Coordinador, se va a pasar a procesar.");
			char*clave = malloc(strlen(datos) + 1);
			strcpy(clave, datos);
			char* valor = malloc(strlen(datos) + 1);
			datos += strlen(datos) + 1;
			strcpy(valor, datos);

			cargarDatos(clave,valor);
			EnviarDatosTipo(socket_coordinador, INSTANCIA, clave, strlen(clave) + 1, t_RESPUESTASET);
			log_info(logger,"Se proceso correctamente el SET y se envio al Coordinador la respuesta del SET.");
			printf("Se proceso correctamente el SET y se envio al Coordinador la respuesta del SET.\n");
			free(clave);
			free(valor);
		}
		break;
		case t_LEERCLAVE: {
			char*clave = malloc(strlen(datos) + 1);
			strcpy(clave, datos);
			printf("Se recibio una solicitud de leer una clave: %s.\n",clave);
		}
		break;
		case t_SOLICITARMEMORIATOTAL: {
			int i;
			int entradaslibres;
			for (i = 0; i < cantidad_de_entradas; i++) {
				if(!strcmp(tabla_entradas[i],"null")){
					entradaslibres++;
				}
			}
			EnviarDatosTipo(socket_coordinador, INSTANCIA, (void*) entradaslibres, sizeof(int), t_RESPUESTAMEMORIA);
		}
		break;

		}
		if (paquete.mensaje != NULL) {
			free(paquete.mensaje);
			log_info(logger,"Se libero la memoria del paquete.");
		}

	}
	close(socket_coordinador);//Cierro el socket porque dejo de recibir info
}

/* Verificar punto de montaje */
void verificarPuntoMontaje(){

	DIR* directorio_de_montaje = opendir(punto_de_montaje);
	if (ENOENT == errno){
		/* Directorio No Existe. */
		log_info(logger,"No existe el punto de montaje, se va a proceder a crearlo.");
		mkdir(punto_de_montaje,0777);
		log_info(logger,"El punto de montaje se creo exitosamente.");
	}
	if (directorio_de_montaje != NULL){
		/* Directorio Existe. */
		struct dirent *ent;
		while( (ent = readdir(directorio_de_montaje)) != NULL ){
			if( (strncmp(ent->d_name, ".", 1)) ){
				FILE * fp;
				char * line = NULL;
				size_t len = 0;
				ssize_t read;

				char* ruta = malloc(strlen(punto_de_montaje) + strlen(ent->d_name) + 1);
				strcpy(ruta, punto_de_montaje);
				strcpy(ruta + strlen(punto_de_montaje),ent->d_name);

				fp = fopen(ruta, "r");
				if (fp == NULL) {
					log_error(logger, "Error al abrir el archivo: %s",strerror(errno));
					log_info(logger,"Se le envio al planificador la orden de matar al ESI.");
					fclose(fp);
				}else{
					while ((read = getline(&line, &len, fp)) != EOF) {
						cargarDatos(ent->d_name,line);
					}
					free(ruta);
					free(line);
					fclose(fp);
				}
			}
		}
		closedir(directorio_de_montaje);
	}else{
		log_error(logger, "Se detectó el siguiente error al abrir el directorio: %s", strerror(errno));
	}
}

//no se hace en coordinador, ya que es propio de la instancia, y no depende del coordinador. es un metodo de backup.
void dump(){
	while(1){
		usleep(intervalo_de_dump * SEGUNDO); //intervalo_de_dump segundos :D!
		//imprimirTabla(); //Esto no se necesita para el TP es solo para Debug
		int i;
		//Recoro las entradas para saber cuales tengo
		for (i=0;  i< list_size(entradas_administrativas); i++) {
			//Agarro las entradas de a 1
			t_AlmacenamientoEntradaAdministrativa* actual = (t_AlmacenamientoEntradaAdministrativa*)list_get(entradas_administrativas, i);
			guardarAArchivo(actual);
		}
		//printf("%s\n",">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");//Esto no se necesita para el TP es solo para Debug
	}
}

//funciones para probar el dump
void cargarDatos(char* unaClave, char* unValor) {

	//Guardo los valores que recibo por parametro en variables locales
	char*clave =malloc(strlen(unaClave) + 1);
	strcpy(clave, unaClave);
	char* valor = malloc(strlen(unValor) + 1);
	strcpy(valor, unValor);

	//Inicializo una nueva posicion para la tabla administrativa
	t_AlmacenamientoEntradaAdministrativa* nueva = malloc(sizeof(t_AlmacenamientoEntradaAdministrativa));
	nueva->clave = malloc(strlen(clave) + 1);
	strcpy(nueva->clave, clave);
	nueva->entradasOcupadas = ceilDivision(strlen(valor));
	nueva->tamanio = strlen(valor);
	nueva->index = getFirstIndex(nueva->entradasOcupadas);

	//Funcion Auxiliar
	bool buscarClave(t_AlmacenamientoEntradaAdministrativa* unaEntrada){
		return !strcmp(unaEntrada->clave, nueva->clave);
	}

	//Verifico si la clave ya existe en la tabla, si existe limpio su valor y la borro de la tabla
	t_AlmacenamientoEntradaAdministrativa*  instanciaAReemplazar = (t_AlmacenamientoEntradaAdministrativa*)list_find(entradas_administrativas, (void*)buscarClave);
	liberarMemoria(instanciaAReemplazar);

	//Busco el index ahora, que si ya existia, libere la posicion que ocupaba
	nueva->index = getFirstIndex(nueva->entradasOcupadas);
	//Agrego la clave con su valor
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
	free(clave);
	free(valor);
}

bool comparadorDeClaves(t_AlmacenamientoEntradaAdministrativa* unaEntrada, t_AlmacenamientoEntradaAdministrativa* otraEntrada){
	return !strcmp(unaEntrada->clave, otraEntrada->clave);
}

//funcion para probar el dump
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

void inicializarTabla(){
	tabla_entradas = malloc((cantidad_de_entradas * tamanio_entrada)+1);
	int i;
	for (i = 0; i < cantidad_de_entradas; i++) {
		tabla_entradas[i] = malloc(tamanio_entrada);
		strcpy(tabla_entradas[i], "null");
	}
}

void imprimirTabla(){
	int i,j;
	for (i = 0; i < list_size(entradas_administrativas); i++) {
		t_AlmacenamientoEntradaAdministrativa* nueva = (t_AlmacenamientoEntradaAdministrativa*)list_get(entradas_administrativas,i);
		printf("Clave: %s\n",nueva->clave);
		printf("Entradas Que Ocupa: %d\n",nueva->entradasOcupadas);
		printf("Index: %d\n",nueva->index);
		printf("Tamanio: %d\n",nueva->tamanio);

		char* valor=malloc(nueva->tamanio);
		int tamanioPegado=0;

		for (j = nueva->index; j < (nueva->index + nueva->entradasOcupadas);j++) {
			if((nueva->index + nueva->entradasOcupadas) -1 == j){
				strcpy(valor+tamanioPegado, tabla_entradas[j]);
			}else{
				strcpy(valor+tamanioPegado, tabla_entradas[j]);
				tamanioPegado+=tamanio_entrada;
			}
		}
		printf("Valor: %s\n",valor);
		printf("%s\n","------------------");
	}
}

void guardarAArchivo(t_AlmacenamientoEntradaAdministrativa* clave_a_store){

	char* directorio_actual = malloc(strlen(punto_de_montaje) + strlen(clave_a_store->clave) + 2);
	strcpy(directorio_actual, punto_de_montaje);
	strcpy(directorio_actual+strlen(punto_de_montaje),clave_a_store->clave);

	char* valor=malloc(clave_a_store->tamanio);
	int tamanioPegado=0; //Variable auxiliar para cuando el nodo ocupa mas de 2 entradas

	int i;
	for (i = clave_a_store->index; i < (clave_a_store->index + clave_a_store->entradasOcupadas);i++) {
		if((clave_a_store->index + clave_a_store->entradasOcupadas) -1 == i){
			strcpy(valor+tamanioPegado, tabla_entradas[i]);
		}else{
			strcpy(valor+tamanioPegado, tabla_entradas[i]);
			tamanioPegado+=tamanio_entrada;
		}
	}
	FILE* file_a_crear = fopen(directorio_actual,"w+");
	fwrite(valor,clave_a_store->tamanio,sizeof(char),file_a_crear);


	//free(valor);
	fclose(file_a_crear);
}

void liberarMemoria(t_AlmacenamientoEntradaAdministrativa* clave_a_liberar){
	if( clave_a_liberar != NULL ){
		int indexClave = list_get_index(entradas_administrativas,clave_a_liberar,(void*)comparadorDeClaves);
		list_remove(entradas_administrativas,indexClave);
		int j;
		for (j = clave_a_liberar->index ; j < ( clave_a_liberar->index + clave_a_liberar->entradasOcupadas); j++){
			strcpy(tabla_entradas[j],"null");
		}
	}
}

void algoritmoCircular(t_AlmacenamientoEntradaAdministrativa* aux) {

	if (aux->entradasOcupadas <= cantidad_de_entradas) {
		int i;
		int tamanioEntradasAdministrativas = list_size(entradas_administrativas);
		for (i = 0; i < tamanioEntradasAdministrativas; i++) {
			t_AlmacenamientoEntradaAdministrativa* aux = (t_AlmacenamientoEntradaAdministrativa*) list_get(entradas_administrativas, i);
			if (aux->entradasOcupadas == 1) {
				EnviarDatosTipo(socket_coordinador, INSTANCIA, aux->clave,strlen(aux->clave) + 1, t_CLAVEBORRADA);
				liberarMemoria(aux);
			}
		}
	}
}


