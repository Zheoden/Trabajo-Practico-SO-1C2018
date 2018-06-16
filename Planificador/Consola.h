#ifndef CONSOLA_H_
#define CONSOLA_H_
#include <string.h>
#include <Funciones/logs.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <Funciones/lista.h>
#include <Funciones/serializacion.h>


bool planificacion_activa;

/* Estructuras */
t_list* ESI_bloqueados;


void consola();
void pausarOContinuar();
void bloquear();
void desbloquear();
void listar(char* recurso);
void killProceso();
void status();
void deadlock();

#endif /* CONSOLA_H_ */
