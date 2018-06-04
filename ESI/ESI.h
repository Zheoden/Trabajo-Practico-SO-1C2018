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
#include <dirent.h>

char* planificador_ip;
int planificador_puerto;
char* coordinador_ip;
int coordinador_puerto;
int socket_planificador;
int socket_coordinador;
FILE* fp;
const char* filename;

void abrirArchivo(char* path);
void crearClientePlanif();
void crearClienteCoor();
void parsear();
void atenderPlanificador();
void atenderCoordinador();
void setearValores(t_config * archivoConfig);
void matarESI();
void foo();
const char* get_filename(const char* path);

#endif /* CLIENTE_H_ */

