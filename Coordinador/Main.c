#include "Coordinador.h"

int main(void) {
	char* coordinador_log_ruta = strdup("/home/utnso/Proyectos/tp-2018-1c-PC-citos/Coordinador/Coordinador.log");
	crearLogger(coordinador_log_ruta, "Coordinador", 0);
	int aux = servidorConSelect();
	return aux;
}
