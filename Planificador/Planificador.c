#include "Planificador.h"


/* Se crean las listas de ESIS listos, ejecución, bloqueados, finalizados y claves bloqueadas */
void inicializar(){
	ESI_clavesBloqueadas = list_create();
    ESI_listos = list_create();
	ESI_ejecucion = list_create();
	ESI_bloqueados = list_create();
	ESI_finalizados = list_create();
	hilos = list_create();
	pthread_mutex_init(&siguiente_linea,NULL);
	planificacion_activa=true;
	ultimo_ID_Asignado = malloc(4);
	strcpy(ultimo_ID_Asignado,"000");

	log_info(logger,"Se inicio inicializaron las variables correctamente.");
}

/* Se setea el archivo de configuración */
void setearValores(t_config * archivoConfig) {
 	server_puerto = config_get_int_value(archivoConfig, "SERVER_PUERTO");
 	server_ip = strdup(config_get_string_value(archivoConfig, "SERVER_IP"));
 	coordinador_puerto = config_get_int_value(archivoConfig, "COORDINADOR_PUERTO");
 	coordinador_ip = strdup(config_get_string_value(archivoConfig, "COORDINADOR_IP"));
 	alfa_planificacion = config_get_int_value(archivoConfig, "ALFA_PLANIFICACION");
 	alfa_planificacion = alfa_planificacion/100;
 	algoritmo_planificacion = strdup(config_get_string_value(archivoConfig, "ALGORITMO_DE_PLANIFICACION"));
 	estimacion_inicial = config_get_int_value(archivoConfig, "ESTIMACION_INICIAL");
 	claves_bloqueadas = config_get_array_value(archivoConfig, "CLAVES_BLOQUEADAS");

 	log_info(logger,"Se inicio cargo correctamente el archivo de configuración.");
 	log_info(logger,"Se inicio el Planificador con el siguiente Algoritmo de Planificación: %s",algoritmo_planificacion);

 }

/* Se inicia la consola del PLANIFICADOR */
void iniciarConsola() {
	pthread_t hilo;
	log_info(logger,"Se inicio un hilo para manejar la consola.");
	pthread_create(&hilo, NULL, (void *) consola, NULL);
	pthread_detach(hilo);
}

/* Hilos */
void atenderESI(){
	pthread_t hilo;
	log_info(logger,"Se inicio un hilo para manejar la comunicación con el ESI.");
	pthread_create(&hilo, NULL, (void *) crearServidor, NULL);
	pthread_detach(hilo);
}

void atenderCoordinador(){
	pthread_t unHilo;
	log_info(logger,"Se inicio un hilo para manejar la comunicacion con el Coordinador.");
	pthread_create(&unHilo, NULL, (void *) crearCliente,NULL);
	pthread_detach(unHilo);
}

void iniciarPlanificacion(){
	pthread_t hilo;
	log_info(logger,"Se inicio un hilo para manejar la Planificación.");
	pthread_create(&hilo, NULL, (void *) planificar, NULL);
	pthread_detach(hilo);
}

/* Conexiones */
void sigchld_handler(int s){
     while(wait(NULL) > 0);
 }

void crearServidor() {
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
		if ((socket_esi = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size)) == -1) {
			log_error(logger,"Accept: %s",strerror(errno));
			continue;
		}
		log_info(logger,"Se recibio una conexion de: %s",inet_ntoa(their_addr.sin_addr));

		t_prueba_hilo* itemNuevo = malloc(sizeof(t_prueba_hilo));
		itemNuevo->socket = socket_esi;
		pthread_create(&(itemNuevo->hilo), NULL, (void*)accion, &socket_esi);
		list_add(hilos, itemNuevo);

	}
	close(socket_esi);  // El proceso padre no lo necesita
	log_info(logger,"Se cerró el socket %d.",socket_esi);
}

void accion(void* socket){
	int socket_actual = *(int*) socket;
	Paquete paquete;

	while (RecibirPaqueteServidor(socket_actual, PLANIFICADOR, &paquete) > 0) {
		if (paquete.header.quienEnvia == ESI) {
			switch(paquete.header.tipoMensaje){
				case t_HANDSHAKE:{
					printf("Recibi un Handshake %s\n","Del ESI");
					/*Inicializo un ESI nuevo*/
					ultimo_ID_Asignado = incrementarID(ultimo_ID_Asignado);
					t_ESIPlanificador* nuevoEsi = inicializarESI(ultimo_ID_Asignado,socket_actual);
					list_add(ESI_listos, nuevoEsi);
					log_info(logger,"Se agrego al ESI: %s, a la lista de Listos.", nuevoEsi->ID);
					printf("LISTOS--------------------------------\n");
					imprimir(ESI_listos);
				}
					break;
				case t_ABORTARESI:{
					printf("Creo que sufri un %s\n","Aborto");
					t_ESIPlanificador* esiAAbortar = (t_ESIPlanificador*) list_remove(ESI_ejecucion,0 );
					//liberarrecursos()
					list_add(ESI_finalizados, esiAAbortar);
					log_info(logger,"Se aborto correctamente el ESI %s, y se agrego a la lista de Terminados.",esiAAbortar->ID);
				}
				break;
			}
		}else{
			log_error(logger,"No es ningún proceso ESI.");
		}
		if (paquete.mensaje != NULL){
			free(paquete.mensaje);
			log_info(logger,"Se libero la memoria del paquete.");
		}
	}
}

void crearCliente() {
	Paquete paquete;
	void* datos;

	socket_coordinador = ConectarAServidor(coordinador_puerto,coordinador_ip);

	log_info(logger,"Se establecio conexion con el Coordinador correctamente");
	EnviarHandshake(socket_coordinador, PLANIFICADOR);
	log_info(logger,"Se envio un Handshake al Coordiandor");


	while (RecibirPaqueteCliente(socket_coordinador, PLANIFICADOR, &paquete) > 0) {
		datos = paquete.mensaje;
		switch (paquete.header.tipoMensaje) {
		case t_GET: {
			printf("me llego %s\n","un get");
			//Se fija si la clave que recibio está en la lista de claves bloqueadas

			bool vericarClavesBloqueadas(t_PlanificadorCoordinador* esi) {
				return !strcmp(esi->clave, paquete.mensaje);
			}

			if(list_any_satisfy(ESI_clavesBloqueadas,(void*) vericarClavesBloqueadas)) {
				//Si está, bloquea al proceso ESI
				bool buscarEsiPorID(t_ESIPlanificador* esi) {
					return !strcmp(esi->ID, paquete.mensaje + strlen(paquete.mensaje)+1);
				}
				t_ESIPlanificador* esiABloquear = (t_ESIPlanificador*) list_remove(ESI_ejecucion, 0);
				list_add(ESI_bloqueados, esiABloquear);
				log_info(logger,"Se bloqueo correctamente el ESI: %s, y se agrego a la lista de Bloqueados.",esiABloquear->ID);
			}
			else {
				//Sino, agrega la clave a claves bloqueadas
				t_PlanificadorCoordinador* claveABloquear = malloc(sizeof(t_PlanificadorCoordinador));
				claveABloquear->clave = malloc(strlen(paquete.mensaje) + 1);
				strcpy(claveABloquear->clave, paquete.mensaje);
				list_add(ESI_clavesBloqueadas, claveABloquear);
				log_info(logger,"Se bloqueo correctamente la clave: %s, y se agrego a la lista de claves Bloqueadas.",claveABloquear->clave);
			}

		}
		break;

		case t_STORE:{
			printf("me llego %s\n","un store");
			char* clave = malloc(strlen(datos)+1);
			strcpy(clave, datos);
			liberarClave(clave);
			log_info(logger,"Se libero la clave: %s", clave);
		}
		break;

		case t_SET:{
			printf("me llego %s\n","un set correcto");
		}
		break;


		case t_ABORTARESI:{
			printf("me llego %s\n","un abortar esi");
			t_ESIPlanificador* esiAAbortar = (t_ESIPlanificador*) list_remove(ESI_ejecucion,0 );
			EnviarDatosTipo(socket_esi,PLANIFICADOR, NULL, 0, t_ABORTARESI);

			//liberarrecursos()
			list_add(ESI_finalizados, esiAAbortar);
			log_info(logger,"Se aborto correctamente el ESI %s, y se agrego a la lista de Terminados.",esiAAbortar->ID);
		}
		break;

		}
		pthread_mutex_unlock(&siguiente_linea);
	}
	if (paquete.mensaje != NULL) {
		free(paquete.mensaje);
		log_info(logger,"Se libero la memoria del paquete.");
	}
}

void planificar() {
	while(1){
		while (planificacion_activa) {
			if( !list_is_empty(ESI_listos) || !list_is_empty(ESI_ejecucion) ){
				if (!strcmp(algoritmo_planificacion, "FIFO")) {
					aplicarFIFO();
					ejecutarEsi();
				} else if (!strcmp(algoritmo_planificacion, "SJF/SD")) {
					aplicarSJF();
					ejecutarEsi();

				} else if (!strcmp(algoritmo_planificacion, "SJF/CD")) {
					aplicarSJFConDesalojo();
					ejecutarEsi();
				} else if (!strcmp(algoritmo_planificacion, "HRRN")) {


				}
			}
		}
	}
}

/* Algoritmos de planificación */
void aplicarFIFO(){
	if (list_is_empty(ESI_ejecucion) && (!list_is_empty(ESI_listos)) ) {
		t_ESIPlanificador* esiAEjecutar = (t_ESIPlanificador*) list_remove(ESI_listos, 0);
		list_add(ESI_ejecucion, esiAEjecutar);
	}
}

void aplicarSJFConDesalojo(){
	if (!list_is_empty(ESI_ejecucion)) {
		t_ESIPlanificador* esiEnEjecucion = list_remove(ESI_ejecucion, 0);
		list_add(ESI_listos, esiEnEjecucion);
	}
	aplicarSJF();
}

void aplicarSJF() {

	t_list* aux = list_map(ESI_listos, (void*) CalcularEstimacion);
	list_sort(aux, (void*) ComparadorDeRafagas);

	t_ESIPlanificador* esiAux = (t_ESIPlanificador*) list_remove(aux, 0);

	bool comparator(t_ESIPlanificador* unESI, t_ESIPlanificador* otroESI){
		return !strcmp(unESI->ID, otroESI->ID);
	}

	int index = list_get_index(ESI_listos,esiAux,(void*)comparator);
	t_ESIPlanificador* esiAEjecutar = list_remove(ESI_listos,index);
	list_add(ESI_ejecucion, esiAEjecutar);

}

t_ESIPlanificador* CalcularEstimacion(t_ESIPlanificador* unEsi) {
	unEsi->rafagas_estimadas = (alfa_planificacion * estimacion_inicial)
			+ ((1 - alfa_planificacion) * (unEsi->rafagas_ejecutadas));
	return unEsi;
}

bool ComparadorDeRafagas(t_ESIPlanificador* unESI, t_ESIPlanificador* otroESI) {
	return unESI->rafagas_estimadas <= otroESI->rafagas_estimadas;
}

/* Ejecución de ESI */
void ejecutarEsi() {
	if(!list_is_empty(ESI_ejecucion)){
		t_ESIPlanificador* esiAEjecutar = (t_ESIPlanificador*) list_get(ESI_ejecucion, 0);
		esiAEjecutar->rafagas_ejecutadas++;
		EnviarDatosTipo(esiAEjecutar->socket, PLANIFICADOR, NULL, 0, t_SIGUIENTELINEA);

		Paquete paquete;
		while (RecibirPaqueteCliente(esiAEjecutar->socket, PLANIFICADOR, &paquete) > 0) {
			switch (paquete.header.tipoMensaje) {
			case t_RESPUESTALINEACORRECTA: {
				printf("Se ejecuto correctamente la linea actual");
			}
			break;
			case t_RESPUESTALINEAINCORRECTA: {
				EnviarDatosTipo(esiAEjecutar->socket, PLANIFICADOR, NULL, 0, t_ABORTARESI);
			}
			break;
			}
			break;
		}

	}
}

/*Funcion de Prueba*/
t_ESIPlanificador* inicializarESI(char* ID,int socket){
	t_ESIPlanificador*aux = malloc(sizeof(t_ESIPlanificador)+25);
	aux->ID = malloc(4);
	strcpy(aux->ID, ID);
	aux->bloqueado=false;
	aux->socket = socket;
	aux->rafagas_ejecutadas = 0;
	aux->rafagas_estimadas = 0;
	return aux;
}

/* Impresión de los ESIS ejecutadas */
void imprimir(t_list* self){
	int longitud_de_lista = list_size(self);
	int i;
	for(i = 0; i < longitud_de_lista; i++ ){
		t_ESIPlanificador* aux = (t_ESIPlanificador*) list_get(self,i);
		printf("ID: %s\n", aux->ID);
		printf("rafagas_ejecutadas: %d\n", aux->rafagas_ejecutadas);
		printf("rafagas_estimadas: %f\n", aux->rafagas_estimadas);
	}
}

/*Funcion que genera los IDs*/
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

void liberarClave(char* clave){

	int buscarClave(char* aux){
		return !strcmp(aux, clave);
	}

//	list_remove_by_condition(ESI_clavesBloqueadas,(void*)buscarClave);
	list_remove(ESI_clavesBloqueadas,0);

	void compararClave(t_ESIPlanificador* aux){
		if(aux->bloqueado && (!strcmp(aux->razon_bloqueo, clave))){
			aux->bloqueado=false;
			strcpy(aux->razon_bloqueo,"");
		}
	}

	list_iterate(ESI_bloqueados,(void*)compararClave);
	int tamanioInicial = list_size(ESI_bloqueados);
	int i;
	for(i = 0 ; i < tamanioInicial ; i++){
		t_ESIPlanificador* elemento = (t_ESIPlanificador*) list_remove(ESI_bloqueados,i);
		if( !elemento->bloqueado ){
			list_add(ESI_listos,elemento);
		}else{
			list_add(ESI_bloqueados,elemento);
		}

	}
}

