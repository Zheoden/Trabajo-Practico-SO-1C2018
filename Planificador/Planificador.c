#include "Planificador.h"


void iniciarConsola() {
	pthread_t hilo;
	pthread_create(&hilo, NULL, (void *) consola, NULL);
	pthread_detach(hilo);
}

void consola(){
  char * linea;
  while(1) {
    linea = readline(">");

    if(linea){
      add_history(linea);
    }

    if((!strncmp(linea, "pausar", 6)) || (!strncmp(linea, "continuar", 9)) ) {

    	printf("//Hay que ejecutar pausar() o continuar()\n");
    	log_info(logger,"//Hay que ejecutar pausar() o continuar()");

    }else    if(!strncmp(linea, "bloquear", 8)) {

    	printf("//Hay que ejecutar bloquear()\n");
    	char **parametros = string_split(linea, " ");

    	printf("Se bloqueó el proceso ESI de id %s, en la cola del recurso %s.\n", parametros[2], parametros[1]);
    	log_info(logger,"Se bloqueó el proceso ESI de id %s, en la cola del recurso %s.", parametros[2], parametros[1]);

    	string_iterate_lines(parametros,free);
		free(parametros);

    }else	if(!strncmp(linea, "desbloquear", 11)) {

    	printf("//Hay que ejecutar desbloquear()\n");

		char **parametros = string_split(linea, " ");

		printf("Se desbloqueó el primer proceso ESI en la cola del recurso %s.\n", parametros[1]);
		log_info(logger,"Se desbloqueó el primer proceso ESI en la cola del recurso %s.", parametros[1]);

		string_iterate_lines(parametros,free);
		free(parametros);


    }else	if(!strncmp(linea, "listar", 6)) {

    	printf("//Hay que ejecutar listar()\n");

		char **parametros = string_split(linea, " ");

		printf("Se listan los procesos bloqueados esperando el recurso %s.\n", parametros[1]);
		log_info(logger,"Se listan los procesos bloqueados esperando el recurso %s.", parametros[1]);

		string_iterate_lines(parametros,free);
		free(parametros);

    }else	if(!strncmp(linea, "kill", 4)) {

    	printf("//Hay que ejecutar kill()\n");

		char **parametros = string_split(linea, " ");

		printf("Se finaliza el proceso de id %s.\n", parametros[1]);
		log_info(logger,"Se finaliza el proceso de id %s.", parametros[1]);

		string_iterate_lines(parametros,free);
		free(parametros);


    }else	if(!strncmp(linea, "status", 6)) {

    	printf("//Hay que ejecutar status()\n");

		char **parametros = string_split(linea, " ");

		printf("Se muestra el estado de la clave %s.\n", parametros[1]);
		log_info(logger,"Se muestra el estado de la clave %s.", parametros[1]);

		string_iterate_lines(parametros,free);
		free(parametros);


    }else	if(!strncmp(linea, "deadlock", 8)) {
    	printf("//Hay que ejecutar deadlock()\n");
    	log_info(logger,"//Hay que ejecutar deadlock()");

    }else	if(!strncmp(linea, "exit", 4)) {
    	printf("Se salio de la consola.\n");
    	log_info(logger,"Se salio de la consola.");
        free(linea);
        break;
    }else{
    	printf("No se reconoce el comando ingresado: %s\n",linea);
    	log_info(logger,"No se reconoce el comando ingresado: %s", linea);

    }
    free(linea);
  }
}

void sigchld_handler(int s)
 {
     while(wait(NULL) > 0);
 }

int crearServidor(void) {
	fd_set master;   // conjunto maestro de descriptores de fichero
	fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
	struct sockaddr_in myaddr;     // dirección del servidor
	struct sockaddr_in remoteaddr; // dirección del cliente
	int fdmax;        // número máximo de descriptores de fichero
	int listener;     // descriptor de socket a la escucha
	int newfd;        // descriptor de socket de nueva conexión aceptada
//	char buf[256];
	int nbytes;
	int yes = 1;        // para setsockopt() SO_REUSEADDR, más abajo
	int addrlen;
	int i, j;
	FD_ZERO(&master);    // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);
	// obtener socket a la escucha
	if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}
	// obviar el mensaje "address already in use" (la dirección ya se está usando)
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
			== -1) {
		perror("setsockopt");
		exit(1);
	}
	// enlazar
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = inet_addr(server_ip);
	myaddr.sin_port = htons(server_puerto);
	memset(&(myaddr.sin_zero), '\0', 8);
	if (bind(listener, (struct sockaddr *) &myaddr, sizeof(myaddr)) == -1) {
		perror("bind");
		exit(1);
	}
	// escuchar
	if (listen(listener, 10) == -1) {
		perror("listen");
		exit(1);
	}
	printf("Estoy Escuchando en %s\n", inet_ntoa(myaddr.sin_addr));
	log_info(logger,"Estoy escuchando en %s:",inet_ntoa(myaddr.sin_addr));
	// añadir listener al conjunto maestro
	FD_SET(listener, &master);
	// seguir la pista del descriptor de fichero mayor
	fdmax = listener; // por ahora es éste
	// bucle principal
	for (;;) {
		read_fds = master; // cópialo
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}
		// explorar conexiones existentes en busca de datos que leer
		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) { // ¡¡tenemos datos!!
				if (i == listener) {
					// gestionar nuevas conexiones
					addrlen = sizeof(remoteaddr);
					if ((newfd = accept(listener,
							(struct sockaddr *) &remoteaddr, &addrlen)) == -1) {
						perror("accept");
					} else {
						FD_SET(newfd, &master); // añadir al conjunto maestro
						if (newfd > fdmax) {    // actualizar el máximo
							fdmax = newfd;
						}
						printf("selectserver: new connection from %s on "
								"socket %d\n", inet_ntoa(remoteaddr.sin_addr),
								newfd);
						log_info(logger,"selectserver: new connection from %s on "
								"socket %d", inet_ntoa(remoteaddr.sin_addr), newfd);
					}
				} else {
					if(RecibirHandshake(i,ESI)){
						atenderESI();
					}
					char* buf = malloc(30);
					// gestionar datos de un cliente
					if ((nbytes = recv(i, buf, 30 , 0)) <= 0) {
						// error o conexión cerrada por el cliente
						if (nbytes == 0) {
							// conexión cerrada
							printf("selectserver: socket %d hung up\n", i);
							log_info(logger,"selectserver: socket %d hung up", i);

						} else {
							perror("recv");
						}
						close(i); // bye!
						FD_CLR(i, &master); // eliminar del conjunto maestro
					} else {
						buf[nbytes] = (char)'\0';
						printf("me llegaron %d bytes con %s\n", nbytes, buf);
						log_info(logger, "me llegaron %d bytes con %s", nbytes, buf);
						// tenemos datos de algún cliente
						for (j = 0; j <= fdmax; j++) {
							// ¡enviar a todo el mundo!
							if (FD_ISSET(j, &master)) {
								// excepto al listener y a nosotros mismos
								if (j != listener && j == i) {
									if (send(j, buf, nbytes+1, 0) == -1) {
										perror("send");
									}
								}
							}
						}
					}
					free(buf);
				} // Esto es ¡TAN FEO!
			}
		}
	}

	return 0;
}

int crearServidorSencillo() {
	int sockfd, new_fd; // Escuchar sobre sock_fd, nuevas conexiones sobre new_fd
	struct sockaddr_in my_addr;    // información sobre mi dirección
	struct sockaddr_in their_addr; // información sobre la dirección del cliente
	int sin_size;
	struct sigaction sa;
	int yes = 1;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}

	my_addr.sin_family = AF_INET;         // Ordenación de bytes de la máquina
	my_addr.sin_port = htons(server_puerto);    // short, Ordenación de bytes de la red
	my_addr.sin_addr.s_addr = inet_addr(server_ip); // Rellenar con mi dirección IP
	memset(&(my_addr.sin_zero), '\0', 8); // Poner a cero el resto de la estructura

	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))
			== -1) {
		perror("bind");
		exit(1);
	}

	if (listen(sockfd, 10) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // Eliminar procesos muertos
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	while (1) {  // main accept() loop
		sin_size = sizeof(struct sockaddr_in);
		if ((new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size))
				== -1) {
			perror("accept");
			continue;
		}
		printf("server: got connection from %s\n",
				inet_ntoa(their_addr.sin_addr));

        t_dictionary* ESI_listos = dictionary_create();
		t_dictionary* ESI_ejecucion = dictionary_create();
		t_dictionary* ESI_bloqueados = dictionary_create();
		t_dictionary* ESI_finalizados = dictionary_create();
		char* primero;

		while (dictionary_is_empty(ESI_listos)) {
			//mensaje al ESI para que lea una linea
			RecibirPaqueteESI(sockfd, ESI, paquete);
			dictionary_put(ESI_listos, paquete.clave, Paquete.valor);
			primero = paquete.clave;
		}

		while (dictionary_is_empty(ESI_ejecucion)) {

			if (!dictionary_is_empty(ESI_bloqueados)) {
				if () {
					aux = dictionary_remove(ESI_bloqueados,
							key_del_desbloqueado)
					dictionary_put(ESI_ejecucion, desbloqueado.id,
							desbloqueado.data)
				}

			switch (algoritmo_planificacion) {
				default:
					aux = dictionary_remove(ESI_listos,
							primero);
					dictionary_put(ESI_ejecucion, aux.ID, aux.mensaje);
					break;

				case "SJF":
					break;
				case "HRRN":
					break;

				}

				if (ESI termina) {
					aux = dictionary_remove(ESI_ejecucion, FINALIZADO.ID,
							FINALIZADO.DATA)
					dictionary_put(ESI_finalizado, FINALIZADO.ID,
							FINALIZADO.DATA)

				}
				if (interaccion con coordinador) {
					aux =
					list_add_all(claves_bloqueadas, claves_a_bloquear);
				}

			}
		if (!fork()) { // Este es el proceso hijo
			close(sockfd); // El hijo no necesita este descriptor
			if (send(new_fd, "Hello, world!\n", 14, 0) == -1)
				perror("send");
			close(new_fd);
			exit(0);
		}
		close(new_fd);  // El proceso padre no lo necesita
	}

	return 0;
}

int crearCliente(void) {
 	struct sockaddr_in direccionServidor;
 	direccionServidor.sin_family = AF_INET;
 	direccionServidor.sin_addr.s_addr = inet_addr(coordinador_ip);
 	direccionServidor.sin_port = htons(coordinador_puerto);

 	int cliente = socket(AF_INET, SOCK_STREAM, 0);
 	if (connect(cliente,(void*) &direccionServidor,sizeof(direccionServidor))!=0){
 		perror("No se pudo conectar");
 		return 1;
 	}
 	EnviarHandshake(cliente,PLANIFICADOR);

 	while(1){
 		char mensaje [1000];
 		scanf("%s", mensaje);
 		send(cliente, mensaje, strlen(mensaje), 0);


      	char* buffer = malloc(30);
      	int bytesRecibidos = recv(cliente,buffer, 25, 0);
      	if (bytesRecibidos <= 0) {
      		perror("El chabon se desconecto o bla bla bla");
      		return 1;
      	}
      	buffer[bytesRecibidos] = '\0';
      	printf("me llegaron %d bytes con %s\n", bytesRecibidos, buffer);
      	free(buffer);
 	}

 	return 0;
 }

void atenderESI(){
	pthread_t hilo;
	pthread_create(&hilo, NULL, (void *) crearServidorSencillo, NULL);
	pthread_detach(hilo);
}

void test(){
	printf("SOY UN TEST!!!!!!! FUNCIONA EL HILO!!!!! HEY!!!!!");
}

void atenderCoordinador(){
	pthread_t unHilo;
	pthread_create(&unHilo, NULL, (void *) crearCliente,NULL);
	pthread_detach(unHilo);
}

void setearValores(t_config * archivoConfig) {

 	server_puerto = config_get_int_value(archivoConfig, "SERVER_PUERTO");
 	server_ip = strdup(config_get_string_value(archivoConfig, "SERVER_IP"));
 	coordinador_puerto = config_get_int_value(archivoConfig, "COORDINADOR_PUERTO");
 	coordinador_ip = strdup(config_get_string_value(archivoConfig, "COORDINADOR_IP"));
 	algoritmo_planificacion = strdup(config_get_string_value(archivoConfig, "ALGORITMO_DE_PLANIFICACION"));
 	estimacion_inicial = config_get_int_value(archivoConfig, "ESTIMACION_INICIAL");
 	claves_bloqueadas = config_get_array_value(archivoConfig, "CLAVES_BLOQUEADAS");

 }

