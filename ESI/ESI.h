#ifndef CLIENTE_H_
#define CLIENTE_H_
#include <string.h>
#include <netdb.h> // Para getaddrinfo
#include <unistd.h>
#include <arpa/inet.h>
#include <commons/config.h>
#include <Funciones/logs.h>
#include <parsi/parser.h>
#include <pthread.h>
#include <commons/collections/list.h>


char* planificador_ip;
int planificador_puerto;
char* coordinador_ip;
int coordinador_puerto;

typedef struct{
	char* ip;
	int puerto;
}argumentos;
typedef struct {
	int id;
	t_esi_operacion* operaciones;
} esi;

int crearClientePlanif();
int crearClienteCoor();

void atenderPlanificador();
void atenderCoordinador();
void crearCliente();
void leerArchivoDeConfiguracion(char * configPath);
void leerConfig(char * configPath);
void setearValores(t_config * archivoConfig);
int cantidadDeApariciones(char * cadena, char separador);

#endif /* CLIENTE_H_ */

