#ifndef CLIENTE_H_
#define CLIENTE_H_
#include <string.h>
#include <netdb.h> // Para getaddrinfo
#include <commons/config.h>
#include <Funciones/logs.h>

int crearCliente(void);
char* client_ip;
int client_puerto;

void leerArchivoDeConfiguracion(char * configPath);
void leerConfig(char * configPath);
void setearValores(t_config * archivoConfig);

#endif /* CLIENTE_H_ */

