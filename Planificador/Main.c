#include "Planificador.h"


int main(int argc, char ** argv){
//	consola();
//	crearServidor();

//return EXIT_SUCCESS;
	char* planificador_log_ruta = strdup("/home/utnso/Proyectos/tp-2018-1c-PC-citos/Planificador/Planificador.log");
	crearLogger(planificador_log_ruta, "Planificador", 0);
	char* planificador_config_ruta = strdup("/home/utnso/Proyectos/tp-2018-1c-PC-citos/Planificador/Planificador.cfg");
	leerConfig(planificador_config_ruta);
	iniciarConsola();
	crearServidor();

	return EXIT_SUCCESS;
}
