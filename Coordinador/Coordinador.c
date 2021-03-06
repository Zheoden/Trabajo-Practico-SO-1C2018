#include "Coordinador.h"


void sigchld_handler(int s){
     while(wait(NULL) > 0);
 }

/* Servidor */
void servidor() {
	int sockfd; // Escuchar sobre sock_fd, nuevas conexiones sobre new_fd
		struct sockaddr_in my_addr;    // información sobre mi dirección
		struct sockaddr_in their_addr; // información sobre la dirección del cliente
		int sin_size;
		struct sigaction sa;
		int yes = 1;

		if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
			log_error(logger,"Socket: %s",strerror(errno));
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			log_error(logger,"Setsockopt: %s",strerror(errno));
		}

		my_addr.sin_family = AF_INET;         // Ordenación de bytes de la máquina
		my_addr.sin_port = htons(server_puerto);    // short, Ordenación de bytes de la red
		my_addr.sin_addr.s_addr = inet_addr(server_ip); // Rellenar con mi dirección IP
		memset(&(my_addr.sin_zero), '\0', 8); // Poner a cero el resto de la estructura

		if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1) {
			log_error(logger,"Bind: %s",strerror(errno));
		}
		printf("Servidor levantado! \n");
		log_info(logger,"El Servidor esta levantado esperando conexiones.");
		if (listen(sockfd, 10) == -1) {//revisar esto que solo acepta 10 conexiones
			log_error(logger,"Listen: %s",strerror(errno));
		}

		sa.sa_handler = sigchld_handler; // Eliminar procesos muertos
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = SA_RESTART;

		if (sigaction(SIGCHLD, &sa, NULL) == -1) {
			log_error(logger,"Sigaction: %s",strerror(errno));
		}

		while (true) {  // main accept() loop
			sin_size = sizeof(struct sockaddr_in);
			if ((socketFD = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size)) == -1) {
				log_error(logger,"Accept: %s",strerror(errno));
				continue;
			}

			log_info(logger,"Se recibio una conexion de: %s",inet_ntoa(their_addr.sin_addr));

			t_hilo* itemNuevo = malloc(sizeof(t_hilo));
			itemNuevo->socket = socketFD;
			pthread_create(&(itemNuevo->hilo), NULL, (void*)coordinar, &socketFD);
			list_add(hilos, itemNuevo);

		}

		close(socketFD);  // El proceso padre no lo necesita
		log_info(logger,"Se cerró el socket %d.",socketFD);
}

/* Archivo de Configuración */
void setearValores(t_config * archivoConfig) {

	server_puerto = config_get_int_value(archivoConfig, "SERVER_PUERTO");
	server_ip = strdup(config_get_string_value(archivoConfig, "SERVER_IP"));
	algoritmo_de_distribucion = strdup(config_get_string_value(archivoConfig, "ALGORITMO_DE_DISTRIBUCION"));
	cantidad_entradas = config_get_int_value(archivoConfig, "CANTIDAD_DE_ENTRADAS");
	tamanio_entradas = config_get_int_value(archivoConfig, "TAMAÑO_DE_ENTRADAS");
	retardo_real = config_get_int_value(archivoConfig, "RETARDO");
	retardo = retardo_real / (float) 1000; // Para pasar de mili segundos a segundos

	log_info(logger,"Se inicio cargo correctamente el archivo de configuración.");
	log_info(logger,"Se inicio la Instancia con el siguiente Algoritmo de Distribución: %s",algoritmo_de_distribucion);
}

/* Incialización de las listas de ESIS, claves e INSTANCIAS */
void inicializar(){
	lista_ESIs = list_create();
	todas_las_claves = list_create();
	instancias = list_create();
	hilos = list_create();
	pthread_mutex_init(&t_set,NULL);
	pthread_mutex_lock(&t_set);
	pthread_mutex_init(&recibir_tamanio,NULL);
	pthread_mutex_init(&recibir_valor_clave,NULL);


	log_info(logger,"Se inicio inicializaron las listas correctamente.");
}

/* Operaciones COORDINADOR */
void coordinar(void* socket) {
	int socketActual = *(int*) socket;
	log_info(logger,"Se va a proceder a Coordinar el socket: %d", socketActual);
	int datosRecibidos = 0;
	Paquete paquete;
	void* datos;
	while (datosRecibidos = RecibirPaqueteServidor(socketActual, COORDINADOR ,&paquete) > 0) {
		datos = paquete.mensaje;
		switch (paquete.header.quienEnvia) {
		case INSTANCIA:{
			coordinarInstancia(socketActual,paquete,datos);
		}
		break;
		case ESI:{
			coordinarESI(socketActual,paquete,datos);
		}
		break;
		case PLANIFICADOR:{
			coordinarPlanificador(socketActual,paquete,datos);
		}
		break;

		}
		if (paquete.mensaje != NULL){
			free(paquete.mensaje);
		}
	}

	close(socketActual);
	sacar_instancia(socketActual);
}

/* Obtiene proxima instancia en base al algoritmo de distribucion */
int obtenerProximaInstancia(char* clave){

	if (!strcmp(algoritmo_de_distribucion, "EL")) {

		return EL();

	} else if (!strcmp(algoritmo_de_distribucion, "LSU")) {

		return LSU();

	} else if (!strcmp(algoritmo_de_distribucion, "KE")) {

		return KE(clave);

	}
	return 0;
}

/* Para EL */
int EL() {
	log_info(logger,"Se va a prodecer de buscar la proxima Instancia disponible para el Algoritmo EL.");

	if(list_is_empty(instancias)){
		return 0;
	}
	t_Instancia* aux;

	void inicializar(t_Instancia * elemento) {
		if(elemento->estado_de_conexion || elemento->socket){
			elemento->flagEL = false;
		}
	}
	bool verificarVacio(t_Instancia * elemento) {
		if(elemento->estado_de_conexion || elemento->socket){
			return elemento->flagEL;
		}else{
			return true;
		}
	}

	if (list_all_satisfy(instancias,(void*)verificarVacio)){
		list_iterate(instancias, (void*)inicializar);
		for(int i = 0; i < list_size(instancias); i++){
			aux = list_get(instancias, i);
			if(aux->estado_de_conexion){
				aux->flagEL = true;
				int auxInt = list_get_index(instancias,aux,(void*)comparador_de_socket);
				list_replace(instancias, auxInt, aux);
				log_info(logger,"Se encontro que la instancia %s, es la proxima disponible.",aux->nombre);
				printf("Se encontro que la instancia %s, es la proxima disponible.\n",aux->nombre);
				return aux->socket;
			}
		}
	}

	bool proximo(t_Instancia *elemento) {
		if(elemento->estado_de_conexion || elemento->socket){
			return !elemento->flagEL;
		}else{
			return false;
		}
	}

	aux =  list_find(instancias, (void*) proximo);
	if(aux != NULL){
		aux->flagEL = true;
		int auxInt = list_get_index(instancias,aux,(void*)comparador_de_socket);
		list_replace(instancias, auxInt, aux);
		log_info(logger,"Se encontro que la instancia %s, es la proxima disponible.",aux->nombre);
		printf("Se encontro que la instancia %s, es la proxima disponible.\n",aux->nombre);
		return aux->socket;
	}else{
		printf("No se encontraron instancias dentro del sistema.\n");
	}
	return 0;
}

/* Para LSU */
int LSU() {

	log_info(logger,"Se va a prodecer de buscar la proxima Instancia disponible para el Algoritmo LSU.");

	int estaHabilitada(t_Instancia* elemento) {
		return elemento->estado_de_conexion;
	}
	int tamanioOcupado(t_Instancia_con_tamanio* elemento1, t_Instancia_con_tamanio* elemento2){
		return elemento1->tamanio > elemento2->tamanio;
	}
	t_Instancia_con_tamanio* obtenerTamanio(t_Instancia* elem){
		t_Instancia_con_tamanio* instancia_a_manejar = malloc(sizeof(t_Instancia_con_tamanio));
		EnviarDatosTipo(elem->socket, COORDINADOR,(void*)NULL, 0, t_SOLICITARMEMORIATOTAL);
		pthread_mutex_lock(&recibir_tamanio);

		instancia_a_manejar->tamanio = tamanio_buffer;
		instancia_a_manejar->dato = (t_Instancia*)elem;
		return instancia_a_manejar;
	}

	if(!list_is_empty(instancias)){
		t_list* instancias_habilitadas = list_filter (instancias, (void*) estaHabilitada);
		log_info(logger,"Se le envia a las Instancias solicitudes para conocer su disponibilidad de memoria");
		t_list* instancias_a_tomar = list_map (instancias_habilitadas,(void*) obtenerTamanio);
		list_sort(instancias_a_tomar,(void*)tamanioOcupado);
		t_Instancia_con_tamanio* instancia_a_usar = list_get(instancias_a_tomar, 0);
		list_destroy(instancias_habilitadas);
		list_destroy(instancias_a_tomar);
		return instancia_a_usar->dato->socket;
	}
	return 0;
}

/* Para KE */
int KE(char* clave){

	log_info(logger,"Se va a prodecer de buscar la proxima Instancia disponible para el Algoritmo KE.");
	int cantidad_de_instancias_activas = 0;
	if(!list_is_empty(instancias)){
		//calculo la cantidad de instancias habilitadas en el sistema
		for (int i = 0; i < list_size(instancias); ++i) {
			t_Instancia* actual = list_get(instancias,i);
			if(actual->estado_de_conexion){
				cantidad_de_instancias_activas++;
			}
		}
		int rango = (26/cantidad_de_instancias_activas);
		int index_clave = getIndexLetra(clave[0]);
		int index_instancia = (index_clave/rango);
		int aux = 0;
		for (int i = 0; i < list_size(instancias); ++i) {
			t_Instancia* actual = list_get(instancias,i);
			if(aux == index_instancia){
				if(actual->estado_de_conexion){
					return actual->socket;
				}else{ //el index de la instancia no esta habilitada
					for (int j = 0; j < list_size(instancias); ++j) {
						t_Instancia* instancia_aux = list_get(instancias,j);
						if(instancia_aux->estado_de_conexion){
							return instancia_aux->socket;
						}
					}
				}
			}
			if(actual->estado_de_conexion){
				aux++;
			}
		}
	}
	return 0;
}

bool comparador_de_socket(t_Instancia* unaInstancia, t_Instancia* otraInstancia) {
	return unaInstancia->socket == otraInstancia->socket;
}
/* Para Desconexiones */
void sacar_instancia(int socket) {

	log_info(logger,"Se sacó una instancia utilizando la función sacar_instancia.");

	bool tiene_socket(t_Instancia *instancia) {
		return instancia->socket == socket;
	}

	t_Instancia* instancia = list_find(instancias, (void*) tiene_socket);
	if( instancia != NULL ){
		int indexInstancia = list_get_index(instancias,instancia,(void*)comparador_de_socket);
		instancia->estado_de_conexion = false;
		instancia->socket = 0;
		list_replace(instancias, indexInstancia, instancia);
		printf("Se Desconecto la Instancia %s, con el index %d.\n",instancia->nombre,indexInstancia);
	}
}

/* Para Coordinar los distintos procesos, los hago polimorficos (revisar si vale la pena) */
void coordinarInstancia(int socket, Paquete paquete, void* datos){
	log_info(logger,"Llego un mensaje de una Instancia");
	switch (paquete.header.tipoMensaje) {
	case t_HANDSHAKE: {
		printf("Se recibio un Handshake de una Instancia\n");
		log_info(logger,"Se recibio un Handshake de una Instancia");
		//Evaluar si es mejor que mande directamente el nombre en el handshake o no
		EnviarDatosTipo(socket, COORDINADOR, (void*)NULL, 0, t_SOLICITUDNOMBRE);
		log_info(logger,"Se le envio a la Instancia una solicitud de Nombre");
		int tamanioDatosEntradas = (sizeof(int) * 2);
		void *datosEntradas = malloc(tamanioDatosEntradas);
		*((int*) datosEntradas) = tamanio_entradas;
		datosEntradas += sizeof(int);
		*((int*) datosEntradas) = cantidad_entradas;
		datosEntradas += sizeof(int);
		datosEntradas -= tamanioDatosEntradas;
		log_info(logger,"Se le envio a la Instancia la informacion de las entradas con la que se va a trabajar");
		EnviarDatosTipo(socket, COORDINADOR, datosEntradas,tamanioDatosEntradas, t_CONFIGURACIONINSTANCIA);
		free(datosEntradas);
		log_info(logger,"Se libero la memoria utilizada para enviar los datos a la instancia");
	}
	break;
	case t_IDENTIFICACIONINSTANCIA: {
		char *nombreInstancia = malloc(paquete.header.tamanioMensaje);
		strcpy(nombreInstancia, (char*) paquete.mensaje);

		bool BuscarNombre(t_Instancia* elemento){
			return !strcmp(elemento->nombre,nombreInstancia);
		}

		bool ComparadorDeNombre(t_Instancia* unaInstancia, t_Instancia* otraInstancia){
			return !strcmp(unaInstancia->nombre, otraInstancia->nombre);
		}

		t_Instancia* aux = list_find(instancias,(void*)BuscarNombre);
		if(aux != NULL){//La instancia ya existia, se esta reincorporando

			int indexInstancia = list_get_index(instancias,aux,(void*)ComparadorDeNombre);
			aux->estado_de_conexion = true;
			aux->socket = socket;
			aux->flagEL = false;
			list_replace(instancias, indexInstancia, aux);

			printf("Se reincorporo la instancia: %s\n",aux->nombre);
			printf("Se le va a enviar las claves que tenia almacenadas.\n");

			int cantidad_de_claves = list_size(aux->claves);
			int i;
			for (i = 0; i < cantidad_de_claves; i++) {
				int tamanio = strlen(list_get(aux->claves,i))+1;
				char* clave = malloc(tamanio);
				strcpy(clave,list_get(aux->claves,i));
				EnviarDatosTipo(socket, COORDINADOR, clave, tamanio, t_LEERCLAVE);
				free(clave);
			}
		}else{
			t_Instancia* instancia = malloc(sizeof(t_Instancia));
			instancia->socket = socket;
			instancia->nombre = malloc(strlen(nombreInstancia) + 1);
			instancia->estado_de_conexion = true;
			instancia->flagEL = false;
			instancia->claves = list_create();
			strcpy(instancia->nombre, nombreInstancia);
			list_add(instancias, instancia);
			log_info(logger,"Se agrego la Instancia: %s, a la lista de Instancias.", instancia->nombre);
		}
		free(nombreInstancia);
	}
	break;
	case t_RESPUESTASET: {
		printf("Se recibio una respuesta set de una Instancia\n");
		bool tiene_socket(t_Instancia *instancia) {
			return instancia->socket == socket;
		}

		char*clave = malloc(strlen(datos) + 1);
		strcpy(clave, datos);


		t_Instancia* aux = ((t_Instancia*) list_find(instancias,(void*) tiene_socket));
		list_add(aux->claves, clave);

		log_info(logger,"Se le agrego a la Instancia: %s, la clave %s.", aux->nombre, clave);
		EnviarDatosTipo(socket_planificador, COORDINADOR, NULL,0, t_SET);

		pthread_mutex_unlock(&t_set);
	}
	break;
	case t_RESPUESTASTORE: {
		printf("Se recibio una respuesta store de una Instancia\n");
		EnviarDatosTipo(socket_planificador, COORDINADOR, datos , strlen(datos)+1, t_STORE);
	}
	break;
	case t_CLAVEBORRADA: {

		char* clave_a_borrar = malloc(strlen(datos)+1);
		strcpy(clave_a_borrar,datos);

		bool buscar_por_socket(t_Instancia* unaInstancia,t_Instancia* otraInstancia) {
			return unaInstancia->socket == otraInstancia->socket;
		}

		bool tiene_socket(t_Instancia* instancia) {
			return instancia->socket == socket;
		}

		bool buscador_de_claves(char* unaClave, char* otraClave){
			return !strcmp(unaClave,otraClave);
		}

		t_Instancia* instancia = list_find(instancias, (void*) tiene_socket);
		if( instancia != NULL ){
			int indexInstancia = list_get_index(instancias,instancia,(void*)buscar_por_socket);
			int indexClave = list_get_index(instancia->claves,clave_a_borrar,(void*)buscador_de_claves);
			printf("Se recibio una solicitud de borrar clave para la instancia %s\n",instancia->nombre);

			free(list_remove(instancia->claves,indexClave));
			list_replace(instancias, indexInstancia, instancia);
		}else{
			printf("La instancia no existe\n");
		}
		free(clave_a_borrar);
	}
	break;
	case t_RESPUESTAMEMORIA: {
		tamanio_buffer = *((int*) paquete.mensaje);
		pthread_mutex_unlock (&recibir_tamanio);
	}
	break;
	case t_VALORDECLAVE: {
		valor_clave = malloc(strlen(datos) + 1);
		strcpy(valor_clave, datos);
		printf("Respuesta valor: %s\n",valor_clave);
		pthread_mutex_unlock(&recibir_valor_clave);
	}
	break;
	case t_COMPACTACIONINSTANCIA: {
		int cantidad_de_entradas = list_size(instancias);
		for (int i = 0; i < cantidad_de_entradas; i++) {
			t_Instancia* instancia_actual = (t_Instancia*)list_get(instancias,i);
			EnviarDatosTipo(instancia_actual->socket, COORDINADOR, NULL , 0, t_COMPACTACIONINSTANCIA);

		}
	}
	break;
	}
}

void coordinarESI(int socket, Paquete paquete, void* datos){

	switch (paquete.header.tipoMensaje) {
	case t_HANDSHAKE: {
		printf("Se recibio un Handshake de un ESI\n");
		log_info(logger,"Se recibio un Handshake del ESI");
		log_info(logger,"Se guardo el socket del ESI, con el numero: %d",socket);
	}
	break;
	case t_SET: {
		printf("Se recibio un SET de un ESI\n");
		usleep(retardo * SEGUNDO);

		char* clave = malloc(strlen(datos) + 1);
		strcpy(clave, datos);

		datos += strlen(datos) + 1;
		char* valor = malloc(strlen(datos) + 1);
		strcpy(valor, datos);

		int tam = strlen(clave) + strlen(valor) + 2;

		bool verificarExistenciaEnListaDeClaves(char*e) {
			return !strcmp(e, clave);
		}

		int verificarClaveDeAUna(char *otraClave) {
			return !strcmp(otraClave, clave);
		}
		bool verificarClave(t_Instancia *e) {
			return list_any_satisfy(e->claves,(void*)verificarClaveDeAUna);
		}


		if (list_any_satisfy(todas_las_claves,(void*) verificarExistenciaEnListaDeClaves)) {
			if (!list_any_satisfy(instancias, (void*) verificarClave)) {
				//clave existe en el sistema, pero no esta en ninguna instancia, es clave nueva
				int socketSiguiente = obtenerProximaInstancia(clave);
				if (socketSiguiente != 0) {
					EnviarDatosTipo(socketSiguiente, COORDINADOR, paquete.mensaje, tam, t_SET);
				} else {
					//error, no hay instancias conectadas al sistema
					printf("No hay Instancias en el sistema. No se puede procesar el pedido.\n");
				};

			} else {
				//clave existe en el sistema, y esta en alguna instancia, hay que buscar en que instancia y enviarlo.
				int socketInstanciaConClave = buscarInstanciaPorClave(clave);
				if (socketInstanciaConClave != 0) {
					EnviarDatosTipo(socketInstanciaConClave, COORDINADOR, paquete.mensaje, tam, t_SET);
				}
			}
		} else {
			//clave no existe en el sistema
			printf("Se intenta bloquear la clave %s pero no existe en el sistema.\n", clave);
			EnviarDatosTipo(socket_planificador, COORDINADOR, NULL , 0, t_ABORTARESI);
		}
		log_info(loggerOperaciones,"El ESI: %d, recibió operación SET con CLAVE: %s y VALOR: %s", socket, clave, valor);
		pthread_mutex_lock(&t_set);
		EnviarDatosTipo(socket, COORDINADOR, NULL, 0, t_RESPUESTALINEACORRECTA);

		free(clave);
		free(valor);


	}
	break;
	case t_GET: {
		printf("Se recibio un GET de un ESI\n");
		usleep(retardo * SEGUNDO);

		char* clave = malloc(paquete.header.tamanioMensaje);
		strcpy(clave, (char*) paquete.mensaje);

		bool verificarExistenciaEnListaDeClaves(char*e) {
			return !strcmp(e, clave);
		}

		if (!list_any_satisfy(todas_las_claves,(void*) verificarExistenciaEnListaDeClaves)) {
			list_add(todas_las_claves, (char*) clave);
		}


		int tamSend = strlen(clave) + +1;
		void* sendPlanificador = malloc(tamSend);
		strcpy(sendPlanificador, clave);
		EnviarDatosTipo(socket_planificador, COORDINADOR, sendPlanificador,tamSend, t_GET);
		EnviarDatosTipo(socket, COORDINADOR, NULL, 0, t_RESPUESTALINEACORRECTA);
		log_info(loggerOperaciones,"El ESI: %d, recibió operación GET con CLAVE: %s", socket, clave);

		free(sendPlanificador);
	}
	break;
	case t_STORE: {
		printf("Se recibio un STORE de un ESI\n");
		usleep(retardo * SEGUNDO);

		char* clave = malloc(strlen(datos) + 1);
		strcpy(clave, datos);

		int verificarClaveDeAUna(char *unaClave) {
			return !strcmp(unaClave, clave);
		}
		bool verificarClave(t_Instancia *e) {
			return list_any_satisfy(e->claves,(void*)verificarClaveDeAUna);
		}

		t_Instancia* aux = (t_Instancia*) list_find(instancias, (void*) verificarClave);

		if ( aux == NULL ) {
			EnviarDatosTipo(socket_planificador, COORDINADOR, NULL ,0, t_ABORTARESI);
			EnviarDatosTipo(socket, COORDINADOR, NULL, 0, t_RESPUESTALINEAINCORRECTA);
		} else {
			EnviarDatosTipo(aux->socket, COORDINADOR, clave, strlen(clave) + 1, t_STORE);
			EnviarDatosTipo(socket, COORDINADOR, NULL, 0, t_RESPUESTALINEACORRECTA);
		}
		log_info(loggerOperaciones,"El ESI: %d, recibió operación STORE con CLAVE: %s", socket, datos);

		free(clave);
	}
	break;
	}

}

void coordinarPlanificador(int socket, Paquete paquete, void* datos){

	switch (paquete.header.tipoMensaje) {
	case t_HANDSHAKE: {
		printf("Se recibio un Handshake de un Planificador\n");
		log_info(logger,"Se recibió un handshake de un PLANIFICADOR.");
		socket_planificador = socket;
	}
	break;
	case t_VALORDECLAVE: {
		char* clave = malloc(strlen(datos) + 1);
		strcpy(clave, datos);

		int socket_de_la_instancia = buscarInstanciaPorClave(clave);
		if(socket_de_la_instancia != 0){//existe la instancia

			EnviarDatosTipo(socket_de_la_instancia, COORDINADOR, clave, strlen(clave)+1, t_VALORDECLAVE);
			pthread_mutex_lock(&recibir_valor_clave);
			printf("Respuesta valor: %s\n",valor_clave);
			EnviarDatosTipo(socket_planificador, COORDINADOR, valor_clave, strlen(valor_clave)+1, t_VALORDECLAVE);
		}else{
			EnviarDatosTipo(socket_planificador, COORDINADOR, "Esta clave no tiene valor", strlen("Esta clave no tiene valor") + 1, t_VALORDECLAVE);
		}
	}
	break;
	case t_INSTANCIACONCLAVE: {
		char* clave = malloc(strlen(datos) + 1);
		strcpy(clave, datos);

		int socket_de_la_instancia = buscarInstanciaPorClave(clave);
		if(socket_de_la_instancia != 0){//existe la instancia

			bool BuscarSocket(t_Instancia* elemento){
				return elemento->socket == socket_de_la_instancia;
			}
			t_Instancia* instancia = (t_Instancia*)list_find(instancias,(void*)BuscarSocket);

			int tamSend = strlen(instancia->nombre) + +1;
			void* sendPlanificador = malloc(tamSend);
			strcpy(sendPlanificador, instancia->nombre);

			EnviarDatosTipo(socket_planificador, COORDINADOR, sendPlanificador, tamSend, t_INSTANCIACONCLAVE);
		}else{
			EnviarDatosTipo(socket_planificador, COORDINADOR, "Ninguna Instancia tiene esta clave", strlen("Ninguna Instancia tiene esta clave") + 1, t_INSTANCIACONCLAVE);
		}

	}
	break;
	case t_INSTANCIAQUETENDRIALACLAVE: {
		char* clave = malloc(strlen(datos) + 1);
		strcpy(clave, datos);
		int socket_de_la_instancia = buscarInstanciaQueTendriaClave(clave);
		if(socket_de_la_instancia != 0){//existe la instancia

			bool BuscarSocket(t_Instancia* elemento){
				return elemento->socket == socket_de_la_instancia;
			}
			t_Instancia* instancia = (t_Instancia*)list_find(instancias,(void*)BuscarSocket);

			int tamSend = strlen(instancia->nombre) + +1;
			void* sendPlanificador = malloc(tamSend);
			strcpy(sendPlanificador, instancia->nombre);

			EnviarDatosTipo(socket_planificador, COORDINADOR, sendPlanificador, tamSend, t_INSTANCIAQUETENDRIALACLAVE);
		}else{
			EnviarDatosTipo(socket_planificador, COORDINADOR, "No hay instancias en el sistema", strlen("No hay instancias en el sistema") + 1, t_INSTANCIAQUETENDRIALACLAVE);
		}

	}
	break;

	}
}

int buscarInstanciaPorClave(char* clave){

	int tieneClave(char* unaClave) {
		return !strcmp(unaClave, clave);
	}

	int i;
	int cantidad_de_instancias = list_size(instancias);
	for (i = 0; i < cantidad_de_instancias; i++) {
		t_Instancia* instancia_actual = (t_Instancia*)list_get(instancias,i);
		if (list_any_satisfy(instancia_actual->claves, (void*) tieneClave) && instancia_actual->estado_de_conexion) {
			return instancia_actual->socket;
		}
	}
	return 0;
}

int buscarInstanciaQueTendriaClave(char* clave){

	if(buscarInstanciaPorClave(clave) == 0){//es clave nueva
		int socket_auxiliar = obtenerProximaInstancia(clave);
		reiniciarInstanciaPorSocket(socket_auxiliar);
		return socket_auxiliar;
	}else{
		return buscarInstanciaPorClave(clave);
	}
	return 0;
}

t_Instancia* buscarInstanciaPorSocket(int socket) {
	bool tiene_socket(t_Instancia *instancia) {
		return instancia->socket == socket;
	}

	return list_find(instancias,(void*)tiene_socket);
}

void reiniciarInstanciaPorSocket(int socket){

	log_info(logger,"Se reinició Instancia por medio del socket: %d", socket);

	if (!strcmp(algoritmo_de_distribucion, "EL")) {
		t_Instancia* aux = buscarInstanciaPorSocket(socket);
		reiniciarEL(aux);
	}
}

void reiniciarEL(t_Instancia* instancia_a_reiniciar){
	instancia_a_reiniciar->flagEL=false;
}

int getIndexLetra(char letra){
	char letras[26] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};
	for (int i = 0; i < 26; ++i) {
		if(letra == letras[i]){
			return i;
		}
	}
	return -1;
};
