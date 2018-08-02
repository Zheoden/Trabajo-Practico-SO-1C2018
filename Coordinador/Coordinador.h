#ifndef COORDINADOR_H_
#define COORDINADOR_H_
#include <string.h>
#include <netdb.h> // Para getaddrinfo
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include <Funciones/logs.h>
#include <Funciones/configs.h>
#include <Funciones/serializacion.h>
#include <Funciones/lista.h>

#define SEGUNDO 1000000 //El valor de un segundo para la funcion usleep

/* Estructuras */

t_list* instancias;
t_list* lista_ESIs;
t_list* todas_las_claves;

t_list* hilos;

/* Variables globales */
char* server_ip;
int server_puerto;

int socket_planificador;
int socketFD;

char* algoritmo_de_distribucion;
int cantidad_entradas;
int tamanio_entradas;
int retardo_real;
float retardo;
int tamanio_instancia;
int instancias_manejadas;

/* Semaforos */
pthread_mutex_t t_set;
pthread_mutex_t recibir_tamanio;
pthread_mutex_t recibir_valor_clave;

/* Auxiliares */
int tamanio_buffer;
char* valor_clave;

/* Funciones */

void sigchld_handler(int s);
void servidor();
void setearValores(t_config * archivoConfig);
void inicializar();
void coordinar(void* socket);
int obtenerProximaInstancia(char* clave);
int EL();
int LSU();
int KE(char* clave);
bool comparador_de_socket(t_Instancia* unaInstancia, t_Instancia* otraInstancia);
void sacar_instancia(int socket);
int buscarInstanciaPorClave(char* clave);
int buscarInstanciaQueTendriaClave(char* clave);
void reiniciarInstanciaPorSocket(int socket);
t_Instancia* buscarInstanciaPorSocket(int socket);
void reiniciarEL(t_Instancia* instancia_a_reiniciar);
int getIndexLetra(char letra);

void coordinarInstancia(int socket, Paquete paquete, void* datos);
void coordinarESI(int socket, Paquete paquete, void* datos);
void coordinarPlanificador(int socket, Paquete paquete, void* datos);

#endif /* COORDINADOR_H_ */
