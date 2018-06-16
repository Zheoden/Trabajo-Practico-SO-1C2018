#include "Coordinador.h"

int main(void) {

	char* coordinador_log_operaciones = strdup("/home/utnso/Proyectos/tp-2018-1c-PC-citos/Coordinador/OperacionesCoordinador.log");
	crearLogger(coordinador_log_operaciones, "OperacionesCoordinador", 0);
	char* coordinador_log_ruta = strdup("/home/utnso/Proyectos/tp-2018-1c-PC-citos/Coordinador/Coordinador.log");
	crearLogger(coordinador_log_ruta, "Coordinador", 0);
	char* coordinador_config_ruta = strdup("/home/utnso/Proyectos/tp-2018-1c-PC-citos/Coordinador/Coordinador.cfg");
	leerConfig(coordinador_config_ruta,logger);
	inicializar();
	servidor();
	return EXIT_SUCCESS;
}
