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

    	char **parametros = string_split(linea, " ");

    	bloquear(parametros[1],parametros[2]);

//    	printf("Se bloqueó el proceso ESI de id %s, en la cola del recurso %s.\n", parametros[2], parametros[1]);
    	log_info(logger,"Se bloqueó el proceso ESI de id %s, en la cola del recurso %s.", parametros[2], parametros[1]);

    	string_iterate_lines(parametros,free);
		free(parametros);

    }else	if(!strncmp(linea, "desbloquear", 11)) {

		char **parametros = string_split(linea, " ");

//		printf("Se desbloqueó el primer proceso ESI en la cola del recurso %s.\n", parametros[1]);
		log_info(logger,"Se desbloqueó el primer proceso ESI en la cola del recurso %s.", parametros[1]);

		string_iterate_lines(parametros,free);
		free(parametros);


    }else	if(!strncmp(linea, "listar", 6)) {

    	printf("//Hay que ejecutar listar()\n");

		char **parametros = string_split(linea, " ");

		listar(parametros[1]);

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

void bloquear(char* clave, char* id){

	bool comparadorID(t_ESIPlanificador* unESI){
		return !strcmp(unESI->ID, id);
	}

	t_ESIPlanificador* aux = (t_ESIPlanificador*) list_remove_by_condition(ESI_listos,(void*)comparadorID);

	if(aux != NULL){//Encontramos el ESI en listos, lo modifico y lo paso a bloqueados
		aux->bloqueado = true;
		aux->razon_bloqueo = malloc (strlen(clave)+1);
		strcpy(aux->razon_bloqueo,clave);
		list_add(ESI_bloqueados,aux);
	}else{//No esta en la lista de listos, lo busco en Ejecucion
		free(aux);

		t_ESIPlanificador* aux = (t_ESIPlanificador*) list_remove_by_condition(ESI_ejecucion,(void*)comparadorID);
		if(aux != NULL){//Encontramos el ESI en ejecucion, lo modifico y lo paso a bloqueados
			aux->bloqueado = true;
			aux->razon_bloqueo = malloc (strlen(clave)+1);
			strcpy(aux->razon_bloqueo,clave);
			list_add(ESI_bloqueados,aux);
		}else{ // No existe el esi en listos ni en ejecucion. notifico al usuario
			printf("No se Encontro el ID especificado, por favor intente con otro.\n");
		}
	}



}

void desbloquear(){

}

void listar(char* recurso){

	int i;
	for (i = 0; i < list_size(ESI_bloqueados); i++) {
		t_ESIPlanificador* aux = (t_ESIPlanificador*) list_get(ESI_bloqueados,i);
		if( !strcmp(recurso, aux->razon_bloqueo) ){

		printf("ID: %s\n",aux->ID);
		printf("Bloqueado: %d\n",aux->bloqueado);
		printf("Rafagas Ejecutadas: %d\n",aux->rafagas_ejecutadas);
		printf("Razon De Bloqueo: %s\n",aux->razon_bloqueo);
		printf("Socket: %d\n",aux->socket);
		printf("%s\n","------------------------------------");

		}
	}

}








