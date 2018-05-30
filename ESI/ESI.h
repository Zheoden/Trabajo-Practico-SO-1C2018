#ifndef CLIENTE_H_
#define CLIENTE_H_

#include <string.h>
#include <netdb.h> // Para getaddrinfo
#include <unistd.h>
#include <arpa/inet.h>
#include <commons/config.h>
#include <parsi/parser.h>
#include <pthread.h>
#include <commons/collections/list.h>
#include <Funciones/logs.h>
#include <Funciones/configs.h>
#include <Funciones/serializacion.h>

char* planificador_ip;
int planificador_puerto;
char* coordinador_ip;
int coordinador_puerto;
int socket_planificador;
int socket_coordinador;
char* IDEsiActual;


void crearClientePlanif();
void crearClienteCoor();
void parsear();
void atenderPlanificador();
void atenderCoordinador();
void setearValores(t_config * archivoConfig);
int cantidadDeApariciones(char * cadena, char separador);
void matarESI();

#endif /* CLIENTE_H_ */

