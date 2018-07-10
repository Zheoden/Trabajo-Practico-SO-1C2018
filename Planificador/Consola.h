#ifndef CONSOLA_H_
#define CONSOLA_H_
#include <string.h>
#include <Funciones/logs.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <Funciones/lista.h>
#include <Funciones/serializacion.h>
#include <pthread.h>


bool planificacion_activa;
int socket_coordinador;

/* Estructuras */
t_list* ESI_bloqueados;
t_list* ESI_listos;
t_list* ESI_ejecucion;
t_list* ESI_finalizados;
t_list* ESI_clavesBloqueadas;

t_list* esis_en_deadlock;

/* Semaforos */
pthread_mutex_t t_status;

/* Cosas para el Status */
char* nombreInstancia;
char* instancia_que_tendria_la_clave;

void consola();
void pausarOContinuar();
void bloquear(char* clave, char* id);
void desbloquear(char* clave);
void listar(char* recurso);
void killProceso(char* id);
void status(char* clave);
void deadlock();
void verificar_si_alguien_tiene_el_recurso(char* clave);
bool verificar_si_hay_circulo();
bool tiene_clave_tomada(t_ESIPlanificador* esi, char* clave );
bool comparador_de_esis(t_ESIPlanificador* unESI, t_ESIPlanificador* otroESI);

#endif /* CONSOLA_H_ */
