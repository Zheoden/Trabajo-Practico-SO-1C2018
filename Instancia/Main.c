#include "Instancia.h"

int main() {

	char* instancia_log_ruta = strdup("/home/utnso/Proyectos/tp-2018-1c-PC-citos/Instancia/Instancia.log");
	crearLogger(instancia_log_ruta, "Instancia", 0);
	char* instancia_config_ruta = strdup("/home/utnso/Proyectos/tp-2018-1c-PC-citos/Instancia/Instancia.cfg");
	leerConfig(instancia_config_ruta,logger);
	crearCliente();
	return EXIT_SUCCESS;
}
