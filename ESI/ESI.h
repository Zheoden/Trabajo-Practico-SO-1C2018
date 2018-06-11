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
#include <Funciones/conexiones.h>
#include <Funciones/archivos.h>
#include <dirent.h>

/* Variables globales */

char* planificador_ip;
int planificador_puerto;

char* coordinador_ip;
int coordinador_puerto;

int socket_planificador;
int socket_coordinador;

FILE* fp;
const char* filename;

/* Funciones*/

void crearClienteCoor();
void crearClientePlanif();
void atenderPlanificador();
void atenderCoordinador();
void setearValores(t_config * archivoConfig);
void parsear();
void matarESI();
void abrirArchivo(char* path);
void foo();


#endif /* CLIENTE_H_ */

