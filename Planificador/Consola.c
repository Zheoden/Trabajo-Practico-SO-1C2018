#include "Consola.h"

void consola(){
  char * linea;
  while(1) {
    linea = readline(">");

    if(linea){
      add_history(linea);
    }

    if(!strncmp(linea, "pausar", 6)) {
    	if(planificacion_activa){
    		printf("Pausar\n");
    		pausarOContinuar();
    		log_info(logger,"//Hay que ejecutar pausar() o continuar()");
    	}

    }else	if(!strncmp(linea, "continuar", 9) ) {
    	if(!planificacion_activa){
    		printf("continuar\n");
    		pausarOContinuar();
    		log_info(logger,"//Hay que ejecutar pausar() o continuar()");
    	}

    }else	if(!strncmp(linea, "bloquear", 8)) {

    	printf("//Hay que ejecutar bloquear()\n");
    	char **parametros = string_split(linea, " ");

    	printf("Se bloqueó el proceso ESI de id %s, en la cola del recurso %s.\n", parametros[2], parametros[1]);
    	log_info(logger,"Se bloqueó el proceso ESI de id %s, en la cola del recurso %s.", parametros[2], parametros[1]);

    	string_iterate_lines(parametros,free);
		free(parametros);

    }else	if(!strncmp(linea, "desbloquear", 11)) {

    	printf("//Hay que ejecutar desbloquear()\n");

		char **parametros = string_split(linea, " ");

		printf("Se desbloqueó el primer proceso ESI en la cola del recurso %s.\n", parametros[1]);
		log_info(logger,"Se desbloqueó el primer proceso ESI en la cola del recurso %s.", parametros[1]);

		string_iterate_lines(parametros,free);
		free(parametros);


    }else	if(!strncmp(linea, "listar", 6)) {

    	printf("//Hay que ejecutar listar()\n");

		char **parametros = string_split(linea, " ");

		printf("Se listan los procesos bloqueados esperando el recurso %s.\n", parametros[1]);
		log_info(logger,"Se listan los procesos bloqueados esperando el recurso %s.", parametros[1]);

		string_iterate_lines(parametros,free);
		free(parametros);

    }else	if(!strncmp(linea, "kill", 4)) {

    	printf("//Hay que ejecutar kill()\n");

		char **parametros = string_split(linea, " ");

		printf("Se finaliza el proceso de id %s.\n", parametros[1]);
		log_info(logger,"Se finaliza el proceso de id %s.", parametros[1]);

		string_iterate_lines(parametros,free);
		free(parametros);


    }else	if(!strncmp(linea, "status", 6)) {

    	printf("//Hay que ejecutar status()\n");

		char **parametros = string_split(linea, " ");

		printf("Se muestra el estado de la clave %s.\n", parametros[1]);
		log_info(logger,"Se muestra el estado de la clave %s.", parametros[1]);

		string_iterate_lines(parametros,free);
		free(parametros);


    }else	if(!strncmp(linea, "deadlock", 8)) {
    	printf("//Hay que ejecutar deadlock()\n");
    	log_info(logger,"//Hay que ejecutar deadlock()");

    }else	if(!strncmp(linea, "exit", 4)) {
    	printf("Se salio de la consola.\n");
    	log_info(logger,"Se salio de la consola.");
        free(linea);
        break;
    }else{
    	printf("No se reconoce el comando ingresado: %s\n",linea);
    	log_info(logger,"No se reconoce el comando ingresado: %s", linea);

    }
    free(linea);
  }
}


void pausarOContinuar(){
	planificacion_activa = !planificacion_activa;
}
