#ifndef INSTANCIA_H_
#define INSTANCIA_H_
#include <string.h>
#include <netdb.h> // Para getaddrinfo
#include <unistd.h>
#include <arpa/inet.h>
#include <commons/config.h>
#include <Funciones/logs.h>


// #define IP "127.10.10.10"
//  #define PUERTO 8443

int crearCliente(void);
char* client_ip;
int client_puerto;

void leerArchivoDeConfiguracion(char * configPath);
void leerConfig(char * configPath);
void setearValores(t_config * archivoConfig);


#endif /* INSTANCIA_H_ */
