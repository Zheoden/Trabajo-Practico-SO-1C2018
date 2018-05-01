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
#include <pthread.h>
#include <Funciones/logs.h>



void consola();
void iniciarConsola();

int crearServidor(void);
void sigchld_handler(int s);

char* server_ip;
int server_puerto;
char* client_ip;
int client_puerto;

void leerArchivoDeConfiguracion(char * configPath);
void leerConfig(char * configPath);
void sigchld_handler(int s);
int servidorConSelect(void);
int crearCliente(void);
void setearValores(t_config * archivoConfig);

#endif /* PLANIFICADOR_H_ */
