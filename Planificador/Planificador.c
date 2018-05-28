#include "Planificador.h"


void iniciarConsola() {
	pthread_t hilo;
	log_info(logger,"Se inicio un hilo para manejar la consola.");
	pthread_create(&hilo, NULL, (void *) consola, NULL);
	pthread_detach(hilo);
}

void atenderESI(){
	pthread_t hilo;
	log_info(logger,"Se inicio un hilo para manejar la comunicación con el ESI.");
	pthread_create(&hilo, NULL, (void *) crearServidorSencillo, NULL);
	pthread_detach(hilo);
}

void iniciarPlanificacion(){
	pthread_t hilo;
	log_info(logger,"Se inicio un hilo para manejar la Planificación.");
	pthread_create(&hilo, NULL, (void *) planificar, NULL);
	pthread_detach(hilo);
}

void atenderCoordinador(){
	pthread_t unHilo;
	log_info(logger,"Se inicio un hilo para manejar la comunicacion con el Coordinador.");
	pthread_create(&unHilo, NULL, (void *) crearCliente,NULL);
	pthread_detach(unHilo);
}

void sigchld_handler(int s){
     while(wait(NULL) > 0);
 }

void crearServidorSencillo() {
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
	if (listen(sockfd, 10) == -1) {
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
		EnviarHandshake(socket_esi,PLANIFICADOR);
		if (!fork()) { // Este es el proceso hijo
			close(sockfd); // El hijo no necesita este descriptor
			log_info(logger,"Se cerró el socket %d.",sockfd);
			int socket_esi = *(int*) socket;
			Paquete paquete;

			while (RecibirPaqueteServidor(socket_esi, PLANIFICADOR, &paquete) > 0) {
				if (paquete.header.quienEnvia == ESI) {
					switch(paquete.header.tipoMensaje){
						case t_HANDSHAKE:
							log_info(logger,"El proceso ESI es %s.", (char*)paquete.mensaje);
							t_ESIPlanificador* nuevoEsi =  malloc(sizeof(t_ESIPlanificador));
							nuevoEsi->ID = malloc(strlen(paquete.mensaje) + 1);
							strcpy(nuevoEsi->ID, paquete.mensaje);
							list_add(ESI_listos, nuevoEsi);
							log_info(logger,"Se agrego al ESI: %s, a la lista de Listos.", nuevoEsi->ID);
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
			close(socket_esi);
			log_info(logger,"Se cerró el socket %d.",socket_esi);
		}
		close(socket_esi);  // El proceso padre no lo necesita
		log_info(logger,"Se cerró el socket %d.",socket_esi);
	}

}

void planificar() {
	while (!estadoDePlanificacion) {
		if (!strcmp(algoritmo_planificacion, "FIFO")) {
			t_ESIPlanificador* esiAEjecutar = (t_ESIPlanificador*) list_remove(ESI_listos, 0);
			list_add(ESI_ejecucion, esiAEjecutar);

		} else if (!strcmp(algoritmo_planificacion, "SJF")) {


		} else if (!strcmp(algoritmo_planificacion, "HRRN")) {


		}
	}
}

void crearCliente(void) {
	Paquete paquete;
	void* datos;
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(coordinador_ip);
	direccionServidor.sin_port = htons(coordinador_puerto);

	socket_coordinador = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(socket_coordinador, (void*) &direccionServidor,
			sizeof(direccionServidor)) != 0) {
		log_error(logger,"No se pudo conectar: %s",strerror(errno));
	}

	log_info(logger,"Se establecio conexion con el Coordinador correctamente");
	EnviarHandshake(socket_coordinador, PLANIFICADOR);
	log_info(logger,"Se envio un Handshake al Coordiandor");


	while (RecibirPaqueteCliente(socket_coordinador, PLANIFICADOR, &paquete) > 0) {
		datos = paquete.mensaje;
		switch (paquete.header.tipoMensaje) {
		case t_GET: {
			//Se fija si la clave que recibio está en la lista de claves bloqueadas

			bool vericarClavesBloqueadas(t_ESIPlanificador* esi) {
				return !strcmp(esi->clave, paquete.mensaje);
			}

			if(list_any_satisfy(ESI_clavesBloqueadas,(void*) vericarClavesBloqueadas)) {
				//Si está, bloquea al proceso ESI
				bool buscarEsiPorID(t_ESIPlanificador* esi) {
					return !strcmp(esi->ID, paquete.mensaje + strlen(paquete.mensaje)+1);
				}
				t_ESIPlanificador* esiABloquear = (t_ESIPlanificador*) list_remove_by_condition(ESI_ejecucion,(void*)buscarEsiPorID );
				list_add(ESI_bloqueados, esiABloquear);
				log_info(logger,"Se bloqueo correctamente el ESI: %s, y se agrego a la lista de Bloqueados.",esiABloquear->ID);
			}
			else {
				//Sino, agrega la clave a claves bloqueadas
				//primero recibe ID despues CLAVE
				t_ESIPlanificador* claveABloquear = malloc(sizeof(t_ESIPlanificador));
				strcpy(claveABloquear->ID, paquete.mensaje);
				strcpy(claveABloquear->clave, paquete.mensaje+ strlen(paquete.mensaje) + 1);
				list_add(ESI_clavesBloqueadas, claveABloquear);
				log_info(logger,"Se bloqueo correctamente la clave: %s, y se agrego a la lista de claves Bloqueadas.",claveABloquear->clave);
			}

		}
		break;

		case t_ABORTARESI:{
			bool buscarEsiPorID(t_ESIPlanificador* esi) {
				return !strcmp(esi->ID, datos);
			}
			t_ESIPlanificador* esiAAbortar = (t_ESIPlanificador*) list_remove_by_condition(ESI_ejecucion,(void*)buscarEsiPorID );
			EnviarDatosTipo(socket_esi,PLANIFICADOR, NULL, 0, t_ABORTARESI);
			//liberarrecursos()
			list_add(ESI_finalizados, esiAAbortar);
			log_info(logger,"Se aborto correctamente el ESI %s, y se agrego a la lista de Terminados.",esiAAbortar->ID);
		}
		break;
		}

	}
	if (paquete.mensaje != NULL) {
		free(paquete.mensaje);
		log_info(logger,"Se libero la memoria del paquete.");
	}
}

void inicializar(){
	ESI_clavesBloqueadas = list_create();
    ESI_listos = list_create();
	ESI_ejecucion = list_create();
	ESI_bloqueados = list_create();
	ESI_finalizados = list_create();

	log_info(logger,"Se inicio inicializaron las listas correctamente.");
}

void setearValores(t_config * archivoConfig) {
 	server_puerto = config_get_int_value(archivoConfig, "SERVER_PUERTO");
 	server_ip = strdup(config_get_string_value(archivoConfig, "SERVER_IP"));
 	coordinador_puerto = config_get_int_value(archivoConfig, "COORDINADOR_PUERTO");
 	coordinador_ip = strdup(config_get_string_value(archivoConfig, "COORDINADOR_IP"));
 	algoritmo_planificacion = strdup(config_get_string_value(archivoConfig, "ALGORITMO_DE_PLANIFICACION"));
 	estimacion_inicial = config_get_int_value(archivoConfig, "ESTIMACION_INICIAL");
 	claves_bloqueadas = config_get_array_value(archivoConfig, "CLAVES_BLOQUEADAS");

 	log_info(logger,"Se inicio cargo correctamente el archivo de configuración.");
 	log_info(logger,"Se inicio el Planificador con el siguiente Algoritmo de Planificación: %s",algoritmo_planificacion);

 }
