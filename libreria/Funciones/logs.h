#ifndef LOGS_H_
#define LOGS_H_
#include <stdlib.h>
#include <commons/log.h>


t_log * logger;
t_log * loggerOperaciones;

void crearLogger(char* logPath,  char * logMemoNombreArch, bool consolaActiva);
void crearLoggerOperaciones(char* logPath,  char * logMemoNombreArch, bool consolaActiva);


#endif /* LOGS_H_ */
