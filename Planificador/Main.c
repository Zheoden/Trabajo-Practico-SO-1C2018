#include "Planificador.h"


int main(int argc, char ** argv){
	char* planificador_log_ruta = strdup("/home/utnso/Proyectos/tp-2018-1c-PC-citos/Planificador/Planificador.log");
	crearLogger(planificador_log_ruta, "Planificador", 0);
	char* planificador_config_ruta = strdup("/home/utnso/Proyectos/tp-2018-1c-PC-citos/Planificador/Planificador.cfg");
	leerConfig(planificador_config_ruta,logger);
	inicializar();
	iniciarConsola();
	atenderESI();
//	atenderCoordinador();
//	planificar();
	iniciarPlanificacion();
	crearCliente();
	/*Pruebas de SJF*/
/*
	t_ESIPlanificador* aux = inicializarESI("001",1);
	t_ESIPlanificador* aux1 = inicializarESI("002",1);
	t_ESIPlanificador* aux2 = inicializarESI("003",1);
	t_ESIPlanificador* aux3 = inicializarESI("004",1);
	t_ESIPlanificador* aux4 = inicializarESI("005",1);
	t_ESIPlanificador* aux5 = inicializarESI("006",1);
	t_ESIPlanificador* aux6 = inicializarESI("007",1);

	list_add(ESI_listos,aux);
	list_add(ESI_listos,aux1);
	list_add(ESI_listos,aux2);
	list_add(ESI_listos,aux3);
	list_add(ESI_listos,aux4);
	list_add(ESI_listos,aux5);
	list_add(ESI_listos,aux6);
/*
	aplicarSJF();
	aplicarSJFConDesalojo();
	aplicarSJFConDesalojo();
	aplicarSJFConDesalojo();
	aplicarSJFConDesalojo();
	aplicarSJFConDesalojo();

	usleep(5 * 1000000); //intervalo_de_dump segundos :D!
	printf("LISTOS--------------------------------\n");
	imprimir(ESI_listos);


	printf("EJECUCION--------------------------------\n");
	imprimir(ESI_ejecucion);*/

	/*Fin de pruebas de SJF*/
/*
	t_ESIPlanificador* aux15 = malloc(sizeof(t_ESIPlanificador)+25);
	aux15->ID = malloc(4);
	strcpy(aux15->ID, "015");
	aux15->bloqueado=true;
	aux15->razon_bloqueo = malloc(10);
	strcpy(aux15->razon_bloqueo,"claveTest");
	aux15->rafagas_ejecutadas = 0;

	list_add(ESI_bloqueados,aux15);
	liberarClave("claveTestTest");

	printf("LISTOS--------------------------------\n");
	imprimir(ESI_listos);

	printf("BLOQUEADOS--------------------------------\n");
	imprimir(ESI_bloqueados);*/

	for(;;);

	return EXIT_SUCCESS;
}
