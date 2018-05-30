#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_
#include <string.h>
#include <signal.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <commons/config.h>
#include <Funciones/configs.h>
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
char* algoritmo_planificacion;
int estimacion_inicial;
char* claves_bloqueadas;

bool estadoDePlanificacion;

int socket_esi;
int socket_coordinador;

void planificar();
void crearServidorSencillo();
void atenderESI();
void atenderCoordinador();
void consola();
void iniciarConsola();
void sigchld_handler(int s);
void sigchld_handler(int s);
int servidorConSelect(void);
void crearCliente(void);
void setearValores(t_config * archivoConfig);
void iniciarPlanificacion();
void inicializar();

#endif /* PLANIFICADOR_H_ */
