#include "Coordinador.h"


void sigchld_handler(int s){
     while(wait(NULL) > 0);
 }

void servidorConSelect(void) {
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
		log_error(logger,"Socket: %s",strerror(errno));
	}
	// obviar el mensaje "address already in use" (la dirección ya se está usando)
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		log_error(logger,"Setsockopt: %s",strerror(errno));
	}
	// enlazar
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = inet_addr(server_ip);
	myaddr.sin_port = htons(server_puerto);
	memset(&(myaddr.sin_zero), '\0', 8);
	if (bind(listener, (struct sockaddr *) &myaddr, sizeof(myaddr)) == -1) {
		log_error(logger,"Bind: %s",strerror(errno));
	}
	// escuchar
	if (listen(listener, 10) == -1) {
		log_error(logger,"Listen: %s",strerror(errno));
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
			log_error(logger,"Select: %s",strerror(errno));
		}
		// explorar conexiones existentes en busca de datos que leer
		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) { // ¡¡tenemos datos!!
				if (i == listener) {
					// gestionar nuevas conexiones
					addrlen = sizeof(remoteaddr);
					if ((newfd = accept(listener,
							(struct sockaddr *) &remoteaddr, &addrlen)) == -1) {
						log_error(logger,"Accept: %s",strerror(errno));
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
							log_error(logger,"Recv: %s",strerror(errno));
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
										log_error(logger,"Send: %s",strerror(errno));
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
}

void setearValores(t_config * archivoConfig) {

	server_puerto = config_get_int_value(archivoConfig, "SERVER_PUERTO");
	server_ip = strdup(config_get_string_value(archivoConfig, "SERVER_IP"));
	algoritmo_de_distribucion = strdup(config_get_string_value(archivoConfig, "ALGORITMO_DE_DISTRIBUCION"));
	cantidad_entradas = config_get_int_value(archivoConfig, "CANTIDAD_DE_ENTRADAS");
	tamanio_entradas = config_get_int_value(archivoConfig, "TAMAÑO_DE_ENTRADAS");
	retardo = config_get_int_value(archivoConfig, "RETARDO");

	log_info(logger,"Se inicio cargo correctamente el archivo de configuración.");
	log_info(logger,"Se inicio la Instancia con el siguiente Algoritmo de Distribución: %s",algoritmo_de_distribucion);
}

void inicializar(){
	lista_ESIs = list_create();
	todas_las_claves = list_create();
	instancias = list_create();

	log_info(logger,"Se inicio inicializaron las listas correctamente.");
}

void coordinar(void* socket) {
	int socketActual = *(int*) socket;
	log_info(logger,"Se va a proceder a Coordinar el socket: %d", socketActual);
	Paquete paquete;
	void* datos;
	while (RecibirPaqueteServidor(socketActual, COORDINADOR,&paquete) > 0) {
		switch (paquete.header.quienEnvia) {
		case INSTANCIA:
			//log_info(logger,"Llego un mensaje de una Instancia");
			switch (paquete.header.tipoMensaje) {
			case t_HANDSHAKE: {
				log_info(logger,"Se recibio un Handshake de una Instancia");
				//Evaluar si es mejor que mande directamente el nombre en el handshake o no
				EnviarDatosTipo(socketActual, COORDINADOR, (void*)NULL, 0, t_SOLICITUDNOMBRE);
				log_info(logger,"Se le envio a la Instancia una solicitud de Nombre");
				int tamanioDatosEntradas = sizeof(int) * 2;
				void *datosEntradas = malloc(tamanioDatosEntradas);
				*((int*) datosEntradas) = tamanio_entradas;
				datosEntradas += sizeof(int);
				*((int*) datosEntradas) = cantidad_entradas;
				datosEntradas += sizeof(int);
				datosEntradas -= tamanioDatosEntradas;
				log_info(logger,"Se le envio a la Instancia la informacion de las entradas con la que se va a trabajar");
				EnviarDatosTipo(socketActual, COORDINADOR, datosEntradas,tamanioDatosEntradas, t_CONFIGURACIONINSTANCIA);
				free(datosEntradas);
				log_info(logger,"Se libero la memoria utilizada para enviar los datos a la instancia");
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
				log_info(logger,"Se agrego la Instancia: %s, a la lista de Instancias.", instancia->nombre);
			}
			break;
			case t_RESPUESTASET: {
				int tiene_socket(t_Instancia *instancia) {
					if (instancia->socket == socketActual){
						return instancia->socket != socketActual;
					}
				}
				t_Instancia* aux = ((t_Instancia*) list_find(instancias,(void*) tiene_socket));
				list_add(aux->claves, (char*) paquete.mensaje);
				log_info(logger,"Se le agrego a la Instancia: %s, la clave %s.", aux->nombre,(char*) paquete.mensaje);
				break;
			}
			}
			break;
			case ESI:
				switch (paquete.header.tipoMensaje) {
				case t_HANDSHAKE: {
					socket_ESI = socketActual;
					log_info(logger,"Se recibio un Handshake del ESI");
					log_info(logger,"Se guardo el sockte del ESI, con el numero: %d",socket_ESI);
					break;
				}
				case t_NUEVOESI: {
					t_ESI* nuevo = malloc(sizeof(t_ESI));
					strcpy(nuevo->ID, (char*) paquete.mensaje);
					log_info(logger,"Se recibio informacion de un nuevo ESI con el id: %s",nuevo->ID);
					nuevo->clave = list_create();
					list_add(lista_ESIs, nuevo);
					log_info(logger,"Se agrego al ESI: %s, a la lista de ESIs.", nuevo->ID);

				}
				break;
				case t_SET: {
					usleep(retardo);
					t_ESICoordinador* nuevo = malloc(sizeof(t_ESICoordinador));
					datos = paquete.mensaje;
					strcpy(nuevo->ID, datos);
					log_info(logger,"Se recibio un SET del ESI: %s",nuevo->ID);
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
							log_info(logger,"Se intenta bloquear la clave %s pero en este momento no esta disponible.", nuevo->clave);
							EnviarDatosTipo(socket_planificador, COORDINADOR, nuevo->ID,sizeof(int), t_ABORTARESI);
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
	log_info(logger,"Se va a prodecer de buscar la proxima Instancia disponible para el Algoritmo Circular.");
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
	log_info(logger,"Se encontro que la instancia %s, es la proxima disponible.",aux->nombre);
	return aux->socket;
}

void sacar_instancia(int socket) {
	int tiene_socket(t_Instancia *instancia) {
		if (instancia->socket == socket)
			return instancia->socket != socket;
	}
	instancias = list_filter(instancias, (void*) tiene_socket);
}

