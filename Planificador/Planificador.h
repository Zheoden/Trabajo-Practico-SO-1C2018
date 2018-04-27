#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h> // Para malloc
#include <netdb.h> // Para getaddrinfo
#include <unistd.h> // Para close
#include <signal.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/md5.h> // Para calcular el MD5
#include <sys/socket.h> // Para crear sockets, enviar, recibir, etc
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h> // Para usar readline
#include <readline/history.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
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
