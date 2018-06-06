#include "Planificador.h"


int main(int argc, char ** argv){
	char* planificador_log_ruta = strdup("/home/utnso/Proyectos/tp-2018-1c-PC-citos/Planificador/Planificador.log");
	crearLogger(planificador_log_ruta, "Planificador", 0);
	char* planificador_config_ruta = strdup("/home/utnso/Proyectos/tp-2018-1c-PC-citos/Planificador/Planificador.cfg");
	leerConfig(planificador_config_ruta,logger);
	inicializar();
	iniciarConsola();
	iniciarPlanificacion();
//	atenderCoordinador();
//	crearServidorSencillo();

	/*Pruebas de SJF*/

	t_ESIPlanificador* aux = inicializarESI("001",1);
	t_ESIPlanificador* aux1 = inicializarESI("002",1);
	t_ESIPlanificador* aux2 = inicializarESI("003",1);
	t_ESIPlanificador* aux3 = inicializarESI("004",1);
	t_ESIPlanificador* aux4 = inicializarESI("005",1);
	t_ESIPlanificador* aux5 = inicializarESI("006",2);
	t_ESIPlanificador* aux6 = inicializarESI("007",2);

	list_add(ESI_listos,aux);
	list_add(ESI_listos,aux1);
	list_add(ESI_listos,aux2);
	list_add(ESI_listos,aux3);
	list_add(ESI_listos,aux4);
	list_add(ESI_listos,aux5);
	list_add(ESI_listos,aux6);

	aplicarSJF();
	aplicarSJFConDesalojo();
	aplicarSJFConDesalojo();
	aplicarSJFConDesalojo();
	aplicarSJFConDesalojo();
	aplicarSJFConDesalojo();
	printf("LISTOS--------------------------------\n");
	imprimir(ESI_listos);
	printf("EJECUCION--------------------------------\n");
	imprimir(ESI_ejecucion);

	/*Fin de pruebas de SJF*/

	for(;;);

	return EXIT_SUCCESS;
}
