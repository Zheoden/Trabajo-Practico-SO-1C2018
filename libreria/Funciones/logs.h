#ifndef LOGS_H_
#define LOGS_H_
#include <commons/log.h>


t_log * logger;

void crearLogger(char* logPath,  char * logMemoNombreArch, bool consolaActiva);


#endif /* LOGS_H_ */
