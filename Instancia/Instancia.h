#ifndef INSTANCIA_H_
#define INSTANCIA_H_
#include <string.h>
#include <netdb.h> // Para getaddrinfo
#include <unistd.h>
#include <arpa/inet.h>
#include <commons/config.h>
#include <Funciones/configs.h>
#include <Funciones/logs.h>


char* client_ip;
int client_puerto;
char* algoritmo_de_reemplazo;
char* punto_de_montaje;
char* nombre_de_la_instancia;
int intervalo_de_dump;


int crearCliente(void);
void setearValores(t_config * archivoConfig);


#endif /* INSTANCIA_H_ */
