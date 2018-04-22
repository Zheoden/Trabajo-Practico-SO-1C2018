#include "Coordinador.h"


void sigchld_handler(int s)
 {
     while(wait(NULL) > 0);
 }

int crearServidor(void)
 {
     int sockfd, cliente;  // Escuchar sobre sock_fd, nuevas conexiones sobre new_fd
     struct sockaddr_in my_addr;    // información sobre mi dirección
     struct sockaddr_in their_addr; // información sobre la dirección del cliente
     int sin_size;
     struct sigaction sa;
     int yes=1;

     if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
         perror("socket");
         exit(1);
     }

     if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
         perror("setsockopt");
         exit(1);
     }

     my_addr.sin_family = AF_INET;         // Ordenación de bytes de la máquina
     my_addr.sin_port = htons(server_puerto);     // short, Ordenación de bytes de la red
     my_addr.sin_addr.s_addr = inet_addr(server_ip); // Rellenar con mi dirección IP
     memset(&(my_addr.sin_zero), '\0', 8); // Poner a cero el resto de la estructura

     if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr))
                                                                    == -1) {
         perror("bind");
         exit(1);
     }

 	printf("Estoy Escuchando en %s\n", inet_ntoa(my_addr.sin_addr));

     if (listen(sockfd, SOMAXCONN) == -1) {
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

     while(1) {  // main accept() loop
         sin_size = sizeof(struct sockaddr_in);
         if ((cliente = accept(sockfd, (struct sockaddr *)&their_addr,
                                                        &sin_size)) == -1) {
             perror("accept");
             continue;
         }
         printf("server: Llego una conexion de: %s\n",
                                            inet_ntoa(their_addr.sin_addr));

         while(1){
        	 char* buffer = malloc(30);
        	 int bytesRecibidos = recv(cliente,buffer, 25, 0);
        	 if (bytesRecibidos <= 0 || buffer[0] == 't') {
        		 break;
        	 }
        	 buffer[bytesRecibidos] = (char)'\0';
        	 printf("me llegaron %d bytes con %s\n", bytesRecibidos, buffer);
        	 free(buffer);

        	 send(cliente, "Hello, world!", 14, 0);

         }

         if (!fork()) { // Este es el proceso hijo
             close(sockfd); // El hijo no necesita este descriptor
             if (send(cliente, "Hello, world!", 14, 0) == -1)
            	 perror("send");
             	 perror("Voy a cerrar la conexion!!!");
             close(cliente);
             exit(0);
         }
         printf("Voy a cerrar la conexion con %d!!\n", cliente);
         close(cliente);  // El proceso padre no lo necesita
     }

     return 0;
 }

int servidorConSelect(void) {
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
					}
				} else {
					char* buf = malloc(30);
					// gestionar datos de un cliente
					if ((nbytes = recv(i, buf, 30 , 0)) <= 0) {
						// error o conexión cerrada por el cliente
						if (nbytes == 0) {
							// conexión cerrada
							printf("selectserver: socket %d hung up\n", i);
						} else {
							perror("recv");
						}
						close(i); // bye!
						FD_CLR(i, &master); // eliminar del conjunto maestro
					} else {
						buf[nbytes] = (char)'\0';
						printf("me llegaron %d bytes con %s\n", nbytes, buf);
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

void configure_logger() {
  /*
    1.  Creemos el logger con la funcion de las commons log_create.
        Tiene que: guardarlo en el archivo tp0.log, mostrar 'tp0' al loggear,
        mostrarse por pantalla y mostrar solo los logs de nivel info para arriba
        (info, warning y error!)
  */
  logger = log_create("coordinador.log", "Coordinador", true, LOG_LEVEL_INFO);
}
void crearLogger(char* logPath,  char * logMemoNombreArch, bool consolaActiva) {
	logger = log_create(logPath, logMemoNombreArch, consolaActiva, LOG_LEVEL_INFO);
	free(logPath);
}

void leerConfig(char * configPath) {
	leerArchivoDeConfiguracion(configPath);
//free(configPath);
	log_info(logger, "Archivo de configuracion leido correctamente");
}

void leerArchivoDeConfiguracion(char * configPath) {
	t_config * archivoConfig;

	archivoConfig = config_create(configPath);

	if (archivoConfig == NULL){
		perror("[ERROR] Archivo de configurarchcion no encontrado");
		log_error(logger,"Archivo de configurarchcion no encontrado");
	}

	setearValores(archivoConfig);
	config_destroy(archivoConfig);
}

void setearValores(t_config * archivoConfig) {
	server_puerto = config_get_int_value(archivoConfig, "SERVER_PUERTO");
	server_ip = strdup(config_get_string_value(archivoConfig, "SERVER_IP"));
}
