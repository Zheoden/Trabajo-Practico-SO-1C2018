#ifndef INSTANCIA_H_
#define INSTANCIA_H_
#include <string.h>
#include <netdb.h> // Para getaddrinfo
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <commons/config.h>
#include <Funciones/configs.h>
#include <Funciones/logs.h>
#include <Funciones/serializacion.h>
#include <Funciones/lista.h>
#include <dirent.h>


/* Variables globales */

char* client_ip;
int client_puerto;
int socket_coordinador;

char* algoritmo_de_reemplazo;
char* punto_de_montaje;
char* nombre_de_la_instancia;

int intervalo_de_dump;
int tamanio_entrada;
int cantidad_de_entradas;

char** tabla_entradas;
t_list* entradas_administrativas;

void iniciarDump();
void imprimirTabla();
void dump();
void cargarDatos(char* unaClave, char* unValor);
void inicializarTabla();
void verificarPuntoMontaje();

/* Funciones */
void inicializar();
void crearCliente(void);
void setearValores(t_config * archivoConfig);
bool handshakeInstanciaCoordinador();
void iniciarManejoDeEntradas();
void manejarEntradas();
void verificarPuntoMontaje();
void iniciarDump();
void dump();
void cargarDatos(char* unaClave, char* unValor);
int ceilDivision(int lengthValue);
int getFirstIndex (int entradasValue);
bool comparadorDeClaves(t_AlmacenamientoEntradaAdministrativa* unaEntrada, t_AlmacenamientoEntradaAdministrativa* otraEntrada);
void inicializarTabla();
void imprimirTabla();
void guardarAArchivo(t_AlmacenamientoEntradaAdministrativa* clave_a_store);


#endif /* INSTANCIA_H_ */
