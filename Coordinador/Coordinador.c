#include "Coordinador.h"


void sigchld_handler(int s)
 {
     while(wait(NULL) > 0);
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
						log_info(logger,"selectserver: new connection from %s on "
								"socket %d", inet_ntoa(remoteaddr.sin_addr), newfd);
					}
				} else {
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

void setearValores(t_config * archivoConfig) {

	server_puerto = config_get_int_value(archivoConfig, "SERVER_PUERTO");
	server_ip = strdup(config_get_string_value(archivoConfig, "SERVER_IP"));
	algoritmo_de_distribucion = strdup(config_get_string_value(archivoConfig, "ALGORITMO_DE_DISTRIBUCION"));
	cantidad_entradas = config_get_int_value(archivoConfig, "CANTIDAD_DE_ENTRADAS");
	tamanio_entradas = config_get_int_value(archivoConfig, "TAMAÑO_DE_ENTRADAS");
	retardo = config_get_int_value(archivoConfig, "RETARDO");
}

void inicializar(){
	lista_ESIs = list_create();
	todas_las_claves = list_create();
	instancias = list_create();
}

void coordinar(void* socket) {
	int socketActual = *(int*) socket;
	Paquete paquete;
	void* datos;
	while (RecibirPaqueteServidor(socketActual, COORDINADOR,&paquete) > 0) {
		switch (paquete.header.quienEnvia) {
		case INSTANCIA:
			switch (paquete.header.tipoMensaje) {
			case t_HANDSHAKE: {
				EnviarDatosTipo(socketActual, COORDINADOR, 1, 0, t_SOLICITUDNOMBRE);
				int tamanioDatosEntradas = sizeof(int) * 2;
				void *datosEntradas = malloc(tamanioDatosEntradas);
				*((int*) datosEntradas) = tamanio_entradas;
				datosEntradas += sizeof(int);
				*((int*) datosEntradas) = cantidad_entradas;
				datosEntradas += sizeof(int);
				datosEntradas -= tamanioDatosEntradas;
				EnviarDatosTipo(socketActual, COORDINADOR, datosEntradas,tamanioDatosEntradas, t_CONFIGURACIONINSTANCIA);
				free(datosEntradas);
			}
			break;
			case t_IDENTIFICACIONINSTANCIA: {
				char *nombreInstancia = malloc(paquete.header.tamanioMensaje);
				strcpy(nombreInstancia, (char*) paquete.mensaje);
				t_Instancia* instancia = malloc(sizeof(t_Instancia));
				instancia->socket = socketActual;
				instancia->nombre = malloc(strlen(nombreInstancia) + 1);
				instancia->activo = false;
				instancia->claves = list_create();
				strcpy(instancia->nombre, nombreInstancia);
				list_add(instancias, instancia);
			}
			break;
			case t_RESPUESTASET: {
				int tiene_socket(t_Instancia *instancia) {
					if (instancia->socket == socketActual){
						return instancia->socket != socketActual;
					}
				}
				list_add(((t_Instancia*) list_find(instancias,(void*) tiene_socket))->claves, (char*) paquete.mensaje);
				break;
			}
			}
			break;
			case ESI:
				switch (paquete.header.tipoMensaje) {
				case t_HANDSHAKE: {
					socket_ESI = socketActual;
					break;
				}
				case t_NUEVOESI: {
					t_listaDeESICoordinador* nuevo = malloc(sizeof(t_listaDeESICoordinador));
					strcpy(nuevo->ID, (char*) paquete.mensaje);
					nuevo->clave = list_create();
					list_add(lista_ESIs, nuevo);
				}
				break;
				case t_SET: {
					usleep(retardo);
					t_ESICoordinador* nuevo = malloc(sizeof(t_ESICoordinador));
					datos = paquete.mensaje;
					strcpy(nuevo->ID, datos);
					datos += strlen(datos) + 1;
					strcpy(nuevo->clave, datos);
					datos += strlen(datos) + 1;
					strcpy(nuevo->valor, datos);
					int tamanioID = strlen(nuevo->valor) +1;


					//				id = realloc(id, strlen(id) + 1);
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
						if (!list_any_satisfy(instancias, (void*) verificarClave)) {
							//clave existe en el sistema, pero la instancia esta caida
							printf(
									"Se intenta bloquear la clave %s pero en este momento no esta disponible",
									nuevo->clave);
							fflush(stdout);
							EnviarDatosTipo(socket_planificador, COORDINADOR, nuevo->ID,
									sizeof(int), t_ABORTARESI);
						} else {
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
									//							printf("%s\n", socketSiguiente);
									//							fflush(stdout);
									EnviarDatosTipo(socketSiguiente, COORDINADOR,
											sendInstancia, tam, t_SET);
								} else {
									//error, no hay instancias conectadas al sistema
								}
							}
							free(sendInstancia);
						}
					} else {
						//clave no existe en el sistema
						//					printf("Se intenta bloquear la clave %s pero no existe",nuevo->clave);
						EnviarDatosTipo(socket_planificador, COORDINADOR, (void*)nuevo->ID, tamanioID, t_ABORTARESI);
					}
				}
				break;
				case t_GET: {
					usleep(retardo);

					t_ESICoordinador* nuevo = malloc(sizeof(t_ESICoordinador));
					datos = paquete.mensaje;
					strcpy(nuevo->ID, datos);
					datos += strlen(datos) + 1;
					strcpy(nuevo->clave, datos);

					bool verificarExistenciaEnListaDeClaves(char*e) {
						return !strcmp(e, nuevo->clave);
					}

					if (!list_any_satisfy(todas_las_claves,
							(void*) verificarExistenciaEnListaDeClaves)) {
						list_add(todas_las_claves, (char*) nuevo->clave);
					}


					int tamSend = strlen(paquete.mensaje) + strlen(nuevo->ID) + 2;
					void* sendPlanificador = malloc(tamSend);
					strcpy(sendPlanificador, paquete.mensaje);
					sendPlanificador += strlen(paquete.mensaje) + 1;
					strcpy(sendPlanificador, nuevo->ID);
					sendPlanificador += strlen(nuevo->ID) + 1;
					sendPlanificador -= tamSend;
					EnviarDatosTipo(socket_planificador, COORDINADOR, sendPlanificador,tamSend, t_GET);
				}
				break;
				case t_STORE: {
					usleep(retardo);
				}
				break;
				}
				break;
				case PLANIFICADOR:
					switch (paquete.header.tipoMensaje) {
					case t_HANDSHAKE: {
						socket_planificador = socketActual;
					}
					break;
					}
		}
		if (paquete.mensaje != NULL)
			free(paquete.mensaje);
	}
	close(socketActual);
	sacar_instancia(socketActual);
}

int obtenerProximaInstancia() {
	if (list_size(instancias) == 0){
		return 0;
	}
	t_Instancia* aux;

	void inicializar(t_Instancia * elemento) {
		elemento->activo=false;
	}
	bool verificarVacio(t_Instancia * elemento) {
		return elemento->activo==true;
	}

	if (list_all_satisfy(instancias,(void*)verificarVacio)){
		list_iterate(instancias, (void*)inicializar );
		aux = list_get(instancias, 0);
		aux->activo = true;
		list_replace(instancias, 0, aux);
		return aux->socket;
	}

	int i = -1;

	bool proximo(t_Instancia *elemento) {
		i++;
		return !elemento->activo;
	}

	list_find(instancias, (void*) proximo);
	aux = list_get(instancias, i);
	aux->activo = true;
	list_replace(instancias, i, aux);
	return aux->socket;
}

void sacar_instancia(int socket) {
	int tiene_socket(t_Instancia *instancia) {
		if (instancia->socket == socket)
			return instancia->socket != socket;
	}
	instancias = list_filter(instancias, (void*) tiene_socket);
}

