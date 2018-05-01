#ifndef CLIENTE_H_
#define CLIENTE_H_
#include <string.h>
#include <netdb.h> // Para getaddrinfo
#include <unistd.h>
#include <arpa/inet.h>
#include <commons/config.h>
#include <Funciones/logs.h>
#include <parsi/parser.h>
#include <commons/collections/list.h>

int crearCliente(void);
char* client_ip;
int client_puerto;

typedef struct {
	int id;
	t_esi_operacion* operaciones;
} esi;

void leerArchivoDeConfiguracion(char * configPath);
void leerConfig(char * configPath);
void setearValores(t_config * archivoConfig);
int cantidadDeApariciones(char * cadena, char separador);

#endif /* CLIENTE_H_ */

