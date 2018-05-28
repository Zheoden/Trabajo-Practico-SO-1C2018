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
	tamanio_entrada = 10;
	cantidad_de_entradas= 500;
	verificarPuntoMontaje();
	foo();
	cargarDatosFicticios("ElFede","Hola Como Andas?");
	cargarDatosFicticios("ElSuperIvo","Todo Bien, y vos?");
	cargarDatosFicticios("LaCaro","Io khe ze, no soi 100tifiko");
	cargarDatosFicticios("LaMikis","Holiiiiiiisssss");
	cargarDatosFicticios("ElYoff","Veamos que tan bien funciona esto");
	cargarDatosFicticios("Comodin","Comodin Comodin?");
	dump();
	//////////////////////////////////////////
	//            Fin de Pruebas            //
	//////////////////////////////////////////


	return EXIT_SUCCESS;
}
