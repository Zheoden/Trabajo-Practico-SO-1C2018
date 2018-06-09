#include "Instancia.h"

int main() {

	char* instancia_log_ruta = strdup("/home/utnso/Proyectos/tp-2018-1c-PC-citos/Instancia/Instancia.log");
	crearLogger(instancia_log_ruta, "Instancia", 0);
	char* instancia_config_ruta = strdup("/home/utnso/Proyectos/tp-2018-1c-PC-citos/Instancia/Instancia.cfg");
	leerConfig(instancia_config_ruta,logger);
	inicializar();
	//crearCliente();
	//////////////////////////////////////////
	//Llamado a funciones de Prueba para Dump/
	//////////////////////////////////////////
/*
	tamanio_entrada = 10;
	cantidad_de_entradas= 500;
	inicializarTabla();
	verificarPuntoMontaje();
	cargarDatos("ElFede","Hola Como Andas?");
	cargarDatos("ElSuperIvo","Todo Bien, y vos?");
	cargarDatos("LaCaro","Io khe ze, no soi 100tifiko");
	cargarDatos("LaMikis","Holiiiiiiisssss");
	cargarDatos("ElYoff","Veamos que tan bien funciona esto");
	cargarDatos("Comodin","Comodin Comodin?");
	dump();
*/

	//////////////////////////////////////////
	//            Fin de Pruebas            //
	//////////////////////////////////////////





	/////////////////////////////////////////////
	//Llamado a funciones de Prueba para De-Dump/
	/////////////////////////////////////////////
	tamanio_entrada = 10;
	cantidad_de_entradas= 500;
	inicializarTabla();
	verificarPuntoMontaje();
	iniciarDump();
	usleep(intervalo_de_dump * 2 * 1000000); //intervalo_de_dump segundos
	cargarDatos("ElRicky","Ricky And Morty");
	usleep(intervalo_de_dump * 1000000); //intervalo_de_dump segundos
	cargarDatos("ElRicky","Ricky And Morty!!!!!!!!!!!!!!!!");
	cargarDatos("ElMorty","Soy un Morty? Soy un Morty? Soy un Morty? Soy un Morty? Soy un Morty?");
	cargarDatos("ElFede","Hola Como Andas?");
	cargarDatos("ElSuperIvo","Todo Bien, y vos?");
	cargarDatos("LaCaro","Io khe ze, no soi 100tifiko");
	cargarDatos("LaMikis","Holiiiiiiisssss");
	cargarDatos("ElYoff","Veamos que tan bien funciona esto");
	cargarDatos("Comodin","Comodin Comodin?");

	//////////////////////////////////////////
	//            Fin de Pruebas            //
	//////////////////////////////////////////

	for(;;);
	return EXIT_SUCCESS;
}
