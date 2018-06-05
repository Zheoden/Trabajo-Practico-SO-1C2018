#ifndef COORDINADOR_H_
#define COORDINADOR_H_
#include <string.h>
#include <netdb.h> // Para getaddrinfo
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include <Funciones/logs.h>
#include <Funciones/configs.h>
#include <Funciones/serializacion.h>

/* Estructuras */

t_list* instancias;
t_list* lista_ESIs;
t_list* todas_las_claves;

/* Variables globales */
char* server_ip;
int server_puerto;

int socket_planificador;
int socket_ESI;

char* algoritmo_de_distribucion;
int cantidad_entradas;
int tamanio_entradas;
int retardo;

/* Funciones */

void sigchld_handler(int s);
void servidorConSelect(void);
void setearValores(t_config * archivoConfig);
void inicializar();
void coordinar(void* socket);
int obtenerProximaInstancia();
void sacar_instancia(int socket);


#endif /* COORDINADOR_H_ */
