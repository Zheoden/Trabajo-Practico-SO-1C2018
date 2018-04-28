#ifndef COORDINADOR_H_
#define COORDINADOR_H_
#include <string.h>
#include <netdb.h> // Para getaddrinfo
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <commons/config.h>
#include <Funciones/logs.h>


char* server_ip;
int server_puerto;


void leerArchivoDeConfiguracion(char * configPath);
void leerConfig(char * configPath);
void sigchld_handler(int s);
int servidorConSelect(void);
void setearValores(t_config * archivoConfig);


#endif /* COORDINADOR_H_ */
