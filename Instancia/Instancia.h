#ifndef INSTANCIA_H_
#define INSTANCIA_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h> // Para malloc
#include <openssl/md5.h> // Para calcular el MD5
#include <netdb.h> // Para getaddrinfo
#include <unistd.h> // Para close
#include <sys/socket.h> // Para crear sockets, enviar, recibir, etc
#include <readline/readline.h> // Para usar readline
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>

// #define IP "127.10.10.10"
//  #define PUERTO 8443

int crearCliente(void);
char* client_ip;
int client_puerto;

t_log * logger;

void crearLogger(char* logPath,  char * logMemoNombreArch, bool consolaActiva);
void leerArchivoDeConfiguracion(char * configPath);
void leerConfig(char * configPath);
void setearValores(t_config * archivoConfig);


#endif /* INSTANCIA_H_ */
