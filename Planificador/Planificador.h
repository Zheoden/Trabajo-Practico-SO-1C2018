#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_
#include <string.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <commons/config.h>
#include <Funciones/logs.h>



void consola();

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
