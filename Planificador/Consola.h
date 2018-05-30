#ifndef CONSOLA_H_
#define CONSOLA_H_
#include <string.h>
#include <Funciones/logs.h>
#include <readline/readline.h>
#include <readline/history.h>

bool planificacion_activa;

void consola();
void pausarOContinuar();
void bloquear();
void desbloquear();
void listar();
void killProceso();
void status();
void deadlock();

#endif /* CONSOLA_H_ */
