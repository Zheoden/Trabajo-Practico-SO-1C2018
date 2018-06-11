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

			t_prueba_hilo* itemNuevo = malloc(sizeof(t_prueba_hilo));
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
	retardo = retardo_real / 1000; // Para pasar de mili segundos a segundos

	log_info(logger,"Se inicio cargo correctamente el archivo de configuración.");
	log_info(logger,"Se inicio la Instancia con el siguiente Algoritmo de Distribución: %s",algoritmo_de_distribucion);
}

/* Incialización de las listas de ESIS, claves e INSTANCIAS */
void inicializar(){
	lista_ESIs = list_create();
	todas_las_claves = list_create();
	instancias = list_create();
	hilos = list_create();

	log_info(logger,"Se inicio inicializaron las listas correctamente.");
}

/* Operaciones COORDINADOR */
void coordinar(void* socket) {
	int socketActual = *(int*) socket;
	printf("Se va a proceder a Coordinar el socket: %d\n", socketActual);
	log_info(logger,"Se va a proceder a Coordinar el socket: %d", socketActual);
	Paquete paquete;
	void* datos;
	while (RecibirPaqueteServidor(socketActual, COORDINADOR ,&paquete) > 0) {
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

/* Para EL */
int obtenerProximaInstancia() {
	log_info(logger,"Se va a prodecer de buscar la proxima Instancia disponible para el Algoritmo Circular.");
	if (list_size(instancias) == 0){
		return 0;
	}
	t_Instancia* aux;

	void inicializar(t_Instancia * elemento) {
		elemento->flagEL=false;
	}
	bool verificarVacio(t_Instancia * elemento) {
		return elemento->flagEL==true;
	}

	if (list_all_satisfy(instancias,(void*)verificarVacio)){
		list_iterate(instancias, (void*)inicializar );
		aux = list_get(instancias, 0);
		aux->flagEL = true;
		list_replace(instancias, 0, aux);
		return aux->socket;
	}

	int i = -1;

	bool proximo(t_Instancia *elemento) {
		i++;
		return !elemento->flagEL;
	}

	list_find(instancias, (void*) proximo);
	aux = list_get(instancias, i);
	aux->flagEL = true;
	list_replace(instancias, i, aux);
	log_info(logger,"Se encontro que la instancia %s, es la proxima disponible.",aux->nombre);
	return aux->socket;
}

/* Para Desconexiones */
void sacar_instancia(int socket) {
	int tiene_socket(t_Instancia *instancia) {
		if (instancia->socket == socket)
			return instancia->socket != socket;
	}
	instancias = list_filter(instancias, (void*) tiene_socket);
}

/* Para Coordinar los distintos procesos, los hago polimorficos (revisar si vale la pena) */
void coordinarInstancia(int socket, Paquete paquete, void* datos){

	log_info(logger,"Llego un mensaje de una Instancia");
	switch (paquete.header.tipoMensaje) {
	case t_HANDSHAKE: {
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
		t_Instancia* instancia = malloc(sizeof(t_Instancia));
		instancia->socket = socket;
		instancia->nombre = malloc(strlen(nombreInstancia) + 1);
		instancia->flagEL = false;
		instancia->claves = list_create();
		strcpy(instancia->nombre, nombreInstancia);
		list_add(instancias, instancia);
		log_info(logger,"Se agrego la Instancia: %s, a la lista de Instancias.", instancia->nombre);
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
	}
	break;
	case t_RESPUESTASTORE: {
		printf("Se recibio una respuesta store de una Instancia\n");
		EnviarDatosTipo(socket_planificador, COORDINADOR, datos , strlen(datos)+1, t_STORE);
	}
	break;

	}
}

void coordinarESI(int socket, Paquete paquete, void* datos){

	switch (paquete.header.tipoMensaje) {
	case t_HANDSHAKE: {
		printf("Se recibio un Handshake de un ESI\n");
		log_info(logger,"Se recibio un Handshake del ESI");
		log_info(logger,"Se guardo el sockte del ESI, con el numero: %d",socket);
	}
	break;
	case t_SET: {
		printf("Se recibio un SET de un ESI\n");
		usleep(retardo * SEGUNDO);
		t_ESICoordinador* nuevo = malloc(sizeof(t_ESICoordinador));

		nuevo->clave = malloc(strlen(datos) + 1);
		strcpy(nuevo->clave, datos);


		nuevo->valor = malloc(strlen(datos) + 1);
		datos += strlen(datos) + 1;
		strcpy(nuevo->valor, datos);


		bool verificarExistenciaEnListaDeClaves(char*e) {
			return !strcmp(e, nuevo->clave);
		}

		int verificarClaveDeAUna(char *clave) {
			return !strcmp(clave, nuevo->clave);
		}
		bool verificarClave(t_Instancia *e) {
			return list_any_satisfy(e->claves,(void*)verificarClaveDeAUna);
		}

		if (list_any_satisfy(todas_las_claves,
				(void*) verificarExistenciaEnListaDeClaves)) {
			/*if (!list_any_satisfy(instancias, (void*) verificarClave)) {
				//clave existe en el sistema, pero no esta en ninguna instancia
				log_info(logger,"Se intenta bloquear la clave %s pero en este momento no esta disponible.", nuevo->clave);
				EnviarDatosTipo(socket_planificador, COORDINADOR, NULL ,0, t_ABORTARESI);
			} else {*/
			int tam = strlen(nuevo->clave) + strlen(nuevo->valor) + 2;
			void*sendInstancia = malloc(tam);
			strcpy(sendInstancia, nuevo->clave);
			sendInstancia += strlen(nuevo->clave) + 1;
			strcpy(sendInstancia, nuevo->valor);
			sendInstancia += strlen(nuevo->valor) + 1;
			sendInstancia -= tam;
			if (!strcmp(algoritmo_de_distribucion, "EL")) {
				int socketSiguiente = obtenerProximaInstancia();
				if (socketSiguiente != 0) {
					EnviarDatosTipo(socketSiguiente, COORDINADOR, sendInstancia, tam, t_SET);
				} else {
					//error, no hay instancias conectadas al sistema
				}
			}
			free(sendInstancia);
			//}
		} else {
			//clave no existe en el sistema
			printf("Se intenta bloquear la clave %s pero no existe",nuevo->clave);
			EnviarDatosTipo(socket_planificador, COORDINADOR, NULL , 0, t_ABORTARESI);
		}
	}
	break;
	case t_GET: {
		printf("Se recibio un GET de un ESI\n");
		usleep(retardo * SEGUNDO);

		t_ESICoordinador* nuevo = malloc(sizeof(t_ESICoordinador));
		nuevo->clave = malloc(paquete.header.tamanioMensaje);
		strcpy(nuevo->clave, (char*) paquete.mensaje);

		bool verificarExistenciaEnListaDeClaves(char*e) {
			return !strcmp(e, nuevo->clave);
		}

		if (!list_any_satisfy(todas_las_claves,
				(void*) verificarExistenciaEnListaDeClaves)) {
			list_add(todas_las_claves, (char*) nuevo->clave);
		}


		int tamSend = strlen(nuevo->clave) + +1;
		void* sendPlanificador = malloc(tamSend);
		strcpy(sendPlanificador, nuevo->clave);
		EnviarDatosTipo(socket_planificador, COORDINADOR, sendPlanificador,tamSend, t_GET);
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
		} else {
			EnviarDatosTipo(aux->socket, COORDINADOR, datos, strlen(datos) + 1, t_STORE);
		}
	}
	break;
	}

}

void coordinarPlanificador(int socket, Paquete paquete, void* datos){

	switch (paquete.header.tipoMensaje) {
	case t_HANDSHAKE: {
		printf("Se recibio un Handshake de un Planificador\n");
		socket_planificador = socket;
	}
	break;

	}
}
