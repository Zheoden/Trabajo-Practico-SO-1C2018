#include "Instancia.h"

/* Se inicializan las entradas administrativas */
void inicializar(){
	entradas_administrativas = list_create();
	envio_compactacion = false;
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
			tamanio_entrada = *((int*) datos);
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
			int aux = strlen(paquete.mensaje) + 1;
			char*clave = malloc(aux);
			strcpy(clave, paquete.mensaje);


			//Funcion Auxiliar
			bool buscarClave(t_AlmacenamientoEntradaAdministrativa* unaEntrada){
				return !strcmp(unaEntrada->clave, clave);
			}

			//Verifico si la clave ya existe en la tabla, si existe limpio su valor y la borro de la tabla
			t_AlmacenamientoEntradaAdministrativa*  instanciaAReemplazar = (t_AlmacenamientoEntradaAdministrativa*)list_find(entradas_administrativas, (void*)buscarClave);
			guardarAArchivo(instanciaAReemplazar);
			liberarMemoria(instanciaAReemplazar);

			log_info(logger,"Se proceso correctamente el STORE.");
			EnviarDatosTipo(socket_coordinador, INSTANCIA, paquete.mensaje, aux, t_RESPUESTASTORE);
			EnviarDatosTipo(socket_coordinador, INSTANCIA, paquete.mensaje, aux, t_CLAVEBORRADA);
			free(clave);
		}
		break;
		case t_SET: {
			log_info(logger,"Se recibio un SET del Coordinador, se va a pasar a procesar.");

			char*clave = malloc(strlen(datos) + 1);
			strcpy(clave, datos);
			char* valor = malloc(strlen(datos) + 1);
			datos += strlen(datos) +1;
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
			char*clave = malloc(strlen(paquete.mensaje) + 1);
			strcpy(clave, paquete.mensaje);
			leerArchivo(clave);
			printf("Se recibio una solicitud de leer una clave: %s.\n",clave);
		}
		break;
		case t_SOLICITARMEMORIATOTAL: {
			int entradasLibres = cantidad_de_entradas_libres();

			void *datosEntradas = malloc(sizeof(int));
			*((int*) datosEntradas) = entradasLibres;
			EnviarDatosTipo(socket_coordinador, INSTANCIA, (void*) datosEntradas, sizeof(int), t_RESPUESTAMEMORIA);
		}
		break;
		case t_VALORDECLAVE: {
			char* clave = malloc(strlen(paquete.mensaje) + 1);
			strcpy(clave, paquete.mensaje);

			bool buscarClave(t_AlmacenamientoEntradaAdministrativa* unaEntrada){
				return !strcmp(unaEntrada->clave, clave);
			}
			t_AlmacenamientoEntradaAdministrativa* nueva = (t_AlmacenamientoEntradaAdministrativa*)list_find(entradas_administrativas,(void*)buscarClave);
			int j;
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
			//responde el valor
			EnviarDatosTipo(socket_coordinador, INSTANCIA, valor, nueva->tamanio + 1, t_VALORDECLAVE);
		}
		break;

		case t_COMPACTACIONINSTANCIA: {
			if(!envio_compactacion){
				compactacion();
			}else{
				envio_compactacion = false;
			}
		}
		break;
		}
		if (paquete.mensaje != NULL) {
			free(paquete.mensaje);
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
		mkdir(punto_de_montaje, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH);
		log_info(logger,"El punto de montaje se creo exitosamente.");
	}
	if (directorio_de_montaje != NULL){
		/* Directorio Existe. */
		closedir(directorio_de_montaje);
	}else{
		log_error(logger, "Se detectó el siguiente error al abrir el directorio: %s", strerror(errno));
	}
}



//no se hace en coordinador, ya que es propio de la instancia, y no depende del coordinador. es un metodo de backup.
void dump(){
	while(1){
		usleep(intervalo_de_dump * SEGUNDO); //intervalo_de_dump segundos :D!
		int i;
		//Recoro las entradas para saber cuales tengo
		for (i=0;  i< list_size(entradas_administrativas); i++) {
			//Agarro las entradas de a 1
			t_AlmacenamientoEntradaAdministrativa* actual = (t_AlmacenamientoEntradaAdministrativa*)list_get(entradas_administrativas, i);
			guardarAArchivo(actual);
		}
	}
}

//funciones para probar el dump
bool comparadorDeClaves(t_AlmacenamientoEntradaAdministrativa* unaEntrada, t_AlmacenamientoEntradaAdministrativa* otraEntrada){
	return !strcmp(unaEntrada->clave, otraEntrada->clave);
}

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
	nueva->entradasOcupadas = ceilDivision(strlen(valor) + 1);
	nueva->tamanio = strlen(valor) + 1;

	//Funcion Auxiliar
	bool buscarClave(t_AlmacenamientoEntradaAdministrativa* unaEntrada){
		return !strcmp(unaEntrada->clave, nueva->clave);
	}

	//Verifico si la clave ya existe en la tabla, si existe limpio su valor y la borro de la tabla
	t_AlmacenamientoEntradaAdministrativa*  instanciaAReemplazar = (t_AlmacenamientoEntradaAdministrativa*)list_find(entradas_administrativas, (void*)buscarClave);
	liberarMemoria(instanciaAReemplazar);

	//Busco el index ahora, que si ya existia, libere la posicion que ocupaba
	nueva->index = getFirstIndex(nueva->entradasOcupadas);

	if(nueva->index == -1){
		verificarEspacio(nueva);
		nueva->index = getFirstIndex(nueva->entradasOcupadas);
	}

	//Agrego la clave con su valor
	list_add(entradas_administrativas, nueva);
//	log_info(logger,"Se agrego la nueva entrada en la lista de Entradas Administrativas.");
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
	int aux = strlen(valor)/tamanio_entrada;
	if(aux != 0){
		valueAux -= (aux*tamanio_entrada);
	}


	free(clave);
	free(valor);
	free(valueAux);
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
		free(valor);
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

	fclose(file_a_crear);
	free(directorio_actual);

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
	free(clave_a_liberar);
}

void leerArchivo(char* filename){

	if( (strncmp(filename, ".", 1)) ){
		FILE * fp;
		char * line = NULL;
		size_t len = 0;
		ssize_t read;

		char* ruta = malloc(strlen(punto_de_montaje) + strlen(filename) + 1);
		strcpy(ruta, punto_de_montaje);
		strcpy(ruta + strlen(punto_de_montaje),filename);

		fp = fopen(ruta, "r");
		if (fp == NULL) {
			log_error(logger, "Error al abrir el archivo: %s",strerror(errno));
			log_info(logger,"Se le envio al planificador la orden de matar al ESI.");
			fclose(fp);
		}else{
			while ((read = getline(&line, &len, fp)) != EOF) {
				cargarDatos(filename,line);
			}
			free(ruta);
			free(line);
			fclose(fp);
		}
	}

}

bool verificarEspacio(t_AlmacenamientoEntradaAdministrativa* entrada_a_almacenar){

	int cantidad_de_entradas_disponibles = cantidad_de_entradas_libres();

	if(cantidad_de_entradas_disponibles >= entrada_a_almacenar->entradasOcupadas){

		EnviarDatosTipo(socket_coordinador, INSTANCIA, NULL , 0, t_COMPACTACIONINSTANCIA);
		compactacion();
		envio_compactacion = true;

		return true;

	}else{
		if (!strcmp(algoritmo_de_reemplazo, "LRU")) {

			LRU((entrada_a_almacenar->entradasOcupadas - cantidad_de_entradas_disponibles));

		} else if (!strcmp(algoritmo_de_reemplazo, "CIRC")) {

			CIRC((entrada_a_almacenar->entradasOcupadas - cantidad_de_entradas_disponibles));

		} else if (!strcmp(algoritmo_de_reemplazo, "BSU")) {

			BSU((entrada_a_almacenar->entradasOcupadas - cantidad_de_entradas_disponibles));
		}

		cantidad_de_entradas_disponibles = cantidad_de_entradas_libres();

		if(cantidad_de_entradas_disponibles == entrada_a_almacenar->entradasOcupadas){
			EnviarDatosTipo(socket_coordinador, INSTANCIA, NULL , 0, t_COMPACTACIONINSTANCIA);
			compactacion();
			envio_compactacion = true;

			return true;
		}
	}

	return false;
}

void LRU(int entradas_a_liberar) {
	int i,j;
	for (j = 0; j < entradas_a_liberar; j++) {
		int tamanioEntradasAdministrativas = list_size(entradas_administrativas);
		for (i = 0; i < tamanioEntradasAdministrativas; i++) {
			t_AlmacenamientoEntradaAdministrativa* actual = (t_AlmacenamientoEntradaAdministrativa*) list_get(entradas_administrativas, i);
			if (actual->entradasOcupadas == 1) {
				EnviarDatosTipo(socket_coordinador, INSTANCIA, actual->clave,strlen(actual->clave) + 1, t_CLAVEBORRADA);
				printf("Se Reemplazo la Clave: %s.\n",actual->clave);
				liberarMemoria(actual);
				break;
			}
		}
	}
}

void CIRC(int entradas_a_liberar) {
	int i,j;
	for (j = 0; j < entradas_a_liberar; j++) {
		for (i = 0; i < cantidad_de_entradas; i++){
			t_AlmacenamientoEntradaAdministrativa* actual = (t_AlmacenamientoEntradaAdministrativa*)esAtomico(i);
			if(actual != NULL){
				EnviarDatosTipo(socket_coordinador, INSTANCIA, actual->clave,strlen(actual->clave) + 1, t_CLAVEBORRADA);
				printf("Se Reemplazo la Clave: %s.\n",actual->clave);
				liberarMemoria(actual);
				break;
			}else{
				free(actual);
			}
		}
	}
}

void BSU(int entradas_a_liberar) {
	int i,j;
	for (j = 0; j < entradas_a_liberar; j++) {
		t_AlmacenamientoEntradaAdministrativa* candidato = NULL;
		candidato->tamanio=-1;
		for (i=0; i < cantidad_de_entradas; i++) {
			t_AlmacenamientoEntradaAdministrativa* actual = (t_AlmacenamientoEntradaAdministrativa*)esAtomico(i);
			if(actual != NULL){
				if(candidato->tamanio < actual->tamanio){
					candidato = actual;
				}
			}
		}
		if(candidato->tamanio != -1){
			EnviarDatosTipo(socket_coordinador, INSTANCIA, candidato->clave,strlen(candidato->clave) + 1, t_CLAVEBORRADA);
			printf("Se Reemplazo la Clave: %s.\n",candidato->clave);
			liberarMemoria(candidato);
		}
	}
}

t_AlmacenamientoEntradaAdministrativa* esAtomico(int index){

	int buscarClave(t_AlmacenamientoEntradaAdministrativa* elemento){
		return (elemento->index == index && elemento->entradasOcupadas == 1);
	}

	return list_find(entradas_administrativas,(void*)buscarClave);
}

void compactacion(){

	printf("Se inicia el proceso de compactación \n");
	t_list* clavesDelSistema = list_create();

	int i, j;

	int tamanioEntradasAdmin = list_size(entradas_administrativas);

	for(i=0; i < tamanioEntradasAdmin ; i++){
		t_AlmacenamientoEntradaAdministrativa* entradaActual = list_get(entradas_administrativas, 0);
		char* clave_aux = malloc(strlen(entradaActual->clave)+1);
		strcpy(clave_aux,entradaActual->clave);
		guardarAArchivo(entradaActual);
		liberarMemoria(entradaActual);

		list_add(clavesDelSistema, clave_aux);
	}

	for(j=0; j < tamanioEntradasAdmin ; j++){

		char* claveActual = list_get(clavesDelSistema, j);
		leerArchivo(claveActual);

		}

	list_destroy_and_destroy_elements(clavesDelSistema, free);

}

int cantidad_de_entradas_libres(){

	int i;
	int cont = 0;
	for (i=0;  i< cantidad_de_entradas; i++) {
		if(!strcmp(tabla_entradas[i],"null")){
			cont ++;
		}
	}
	return cont;
}



