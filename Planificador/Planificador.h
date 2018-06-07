#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_
#include "Consola.h"
#include <string.h>
#include <signal.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <commons/config.h>
#include <Funciones/configs.h>
#include <Funciones/lista.h>
#include <Funciones/logs.h>
#include <Funciones/serializacion.h>
#include <pthread.h>
#include <stdlib.h>
#include <commons/collections/dictionary.h>
#include <stdbool.h>


/* Estructuras */
t_list* ESI_clavesBloqueadas;
t_list* ESI_listos;
t_list* ESI_ejecucion;
t_list* ESI_bloqueados;
t_list* ESI_finalizados;

/* Variables globales */
char* server_ip;
int server_puerto;

char* coordinador_ip;
int coordinador_puerto;
int socket_coordinador;

char* algoritmo_planificacion;
char* claves_bloqueadas;
int estimacion_inicial;
int alfa_planificacion;

int socket_esi;

char* ultimo_ID_Asignado;

/* Semaforos */
pthread_mutex_t siguiente_linea;

/* Funciones */
void iniciarConsola();
void atenderESI();
void sigchld_handler(int s);
void crearServidorSencillo();
void crearCliente(void);
void atenderCoordinador();
void inicializar();
void setearValores(t_config * archivoConfig);
void iniciarPlanificacion();
void planificar();
void aplicarFIFO();
void aplicarSJFConDesalojo();
void aplicarSJF();
t_ESIPlanificador* CalcularEstimacion(t_ESIPlanificador* unEsi);
bool ComparadorDeRafagas(t_ESIPlanificador* unESI, t_ESIPlanificador* otroESI);
t_ESIPlanificador* inicializarESI(char* ID,int socket);
void ejecutarEsi();
void imprimir(t_list* self);
char* incrementarID(char *ID);
void liberarClave(char* clave);

int servidorConSelect(void);  //Esta wea no se usa



#endif /* PLANIFICADOR_H_ */
