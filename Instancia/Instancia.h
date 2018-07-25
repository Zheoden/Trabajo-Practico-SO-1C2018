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
#include <Funciones/conexiones.h>
#include <dirent.h>

#define SEGUNDO 1000000 //El valor de un segundo para la funcion usleep


/* Variables globales */

char* coordinador_ip;
int coordinador_puerto;
int socket_coordinador;

char* algoritmo_de_reemplazo;
char* punto_de_montaje;
char* nombre_de_la_instancia;

int intervalo_de_dump;
int tamanio_entrada;
int cantidad_de_entradas;

char** tabla_entradas;
t_list* entradas_administrativas;
bool envio_compactacion;

void iniciarDump();
void imprimirTabla();
void dump();
void cargarDatos(char* unaClave, char* unValor);
void inicializarTabla();
void verificarPuntoMontaje();
void atenderCoordinador();

/* Funciones */
void inicializar();
void crearCliente();
void setearValores(t_config * archivoConfig);
bool handshakeInstanciaCoordinador();
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
void liberarMemoria(t_AlmacenamientoEntradaAdministrativa* clave_a_liberar);
void leerArchivo(char* filename);
t_AlmacenamientoEntradaAdministrativa* esAtomico(int index);
void LRU(int entradas_a_liberar);
void CIRC(int entradas_a_liberar);
void BSU(int entradas_a_liberar);
void compactacion();
int cantidad_de_entradas_libres();
bool verificarEspacio(t_AlmacenamientoEntradaAdministrativa* entrada_a_almacenar);

#endif /* INSTANCIA_H_ */
