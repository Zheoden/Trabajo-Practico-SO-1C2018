#include "logs.h"

void crearLogger(char* logPath,  char * logMemoNombreArch, bool consolaActiva) {
	logger = log_create(logPath, logMemoNombreArch, consolaActiva, LOG_LEVEL_INFO);
	free(logPath);
}


void crearLoggerOperaciones(char* logPath,  char * logMemoNombreArch, bool consolaActiva) {
	loggerOperaciones = log_create(logPath, logMemoNombreArch, consolaActiva, LOG_LEVEL_INFO);
	free(logPath);
}
