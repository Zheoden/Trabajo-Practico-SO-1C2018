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


char* client_ip;
int client_puerto;
char* algoritmo_de_reemplazo;
char* punto_de_montaje;
char* nombre_de_la_instancia;
int intervalo_de_dump;
int tamanio_entrada;
int cantidad_de_entradas;
int socket_coordinador;


char** tabla_entradas;
t_list* entradas_administrativas;

void iniciarDump();
void imprimirTabla();
void dump();
void cargarDatos(char* unaClave, char* unValor);
void inicializarTabla();
void verificarPuntoMontaje();
void inicializar();
void iniciarManejoDeEntradas();
void manejarEntradas();
void crearCliente(void);
void setearValores(t_config * archivoConfig);
bool handshakeInstanciaCoordinador();
int ceilDivision(int lengthValue);
int getFirstIndex (int entradasValue);
bool comparadorDeClaves(t_AlmacenamientoEntradaAdministrativa* unaEntrada, t_AlmacenamientoEntradaAdministrativa* otraEntrada);



#endif /* INSTANCIA_H_ */
