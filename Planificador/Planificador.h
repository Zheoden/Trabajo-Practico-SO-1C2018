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

t_list* ESI_clavesBloqueadas;
t_list* ESI_listos;
t_list* ESI_ejecucion;
t_list* ESI_bloqueados;
t_list* ESI_finalizados;

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


void planificar();
void crearServidorSencillo();
void atenderESI();
void atenderCoordinador();
void iniciarConsola();
void sigchld_handler(int s);
int servidorConSelect(void);
void crearCliente(void);
void setearValores(t_config * archivoConfig);
void iniciarPlanificacion();
void inicializar();
t_ESIPlanificador* inicializarESI(char* ID,	int rafagas_ejecutadas);
void aplicarSJF();
void aplicarSJFConDesalojo();
void aplicarFIFO();
void imprimir(t_list* self);
void ejecutarEsi();
bool ComparadorDeRafagas(t_ESIPlanificador* unESI, t_ESIPlanificador* otroESI);

#endif /* PLANIFICADOR_H_ */
