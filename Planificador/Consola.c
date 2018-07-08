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

		if( parametros[1] == NULL && parametros[2] == NULL){
			printf("El Comando listar debe recibir dos parametros.\n");
		}else{
			bloquear(parametros[1],parametros[2]);
    		log_info(logger,"Se bloqueó el proceso ESI de id %s, en la cola del recurso %s.", parametros[2], parametros[1]);
		}
    	string_iterate_lines(parametros,free);
		free(parametros);

    }else	if(!strncmp(linea, "desbloquear", 11)) {

		char **parametros = string_split(linea, " ");

		if(parametros[1] == NULL){
			printf("El Comando desbloquear debe recibir un parametro.\n");
		}else{
			desbloquear(parametros[1]);
			log_info(logger,"Se desbloqueó el primer proceso ESI en la cola del recurso %s.", parametros[1]);
		}
		string_iterate_lines(parametros,free);
		free(parametros);


    }else	if(!strncmp(linea, "listar", 6)) {


		char **parametros = string_split(linea, " ");
		if(parametros[1] == NULL){
			printf("El Comando listar debe recibir un parametro.\n");
		}else{
			listar(parametros[1]);
	//		printf("Se listan los procesos bloqueados esperando el recurso %s.\n", parametros[1]);
			log_info(logger,"Se listan los procesos bloqueados esperando el recurso %s.", parametros[1]);
		}

		string_iterate_lines(parametros,free);
		free(parametros);

    }else	if(!strncmp(linea, "kill", 4)) {


		char **parametros = string_split(linea, " ");
		if(parametros[1] == NULL){
			printf("El Comando kill debe recibir un parametro.\n");
		}else{
			killProceso(parametros[1]);
			printf("Se finaliza el proceso de id %s.\n", parametros[1]);
			log_info(logger,"Se finaliza el proceso de id %s.", parametros[1]);
		}
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

    	deadlock();

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

void desbloquear(char* clave){

	bool comparadorClaves(t_ESIPlanificador* unESI){
		return !strcmp(unESI->razon_bloqueo, clave);
	}

	t_ESIPlanificador* aux = (t_ESIPlanificador*) list_remove_by_condition(ESI_bloqueados,(void*)comparadorClaves);
	if(aux != NULL){//Encontramos el ESI en ejecucion, lo modifico y lo paso a bloqueados
		aux->bloqueado = false;
		strcpy(aux->razon_bloqueo,"");
		list_add(ESI_listos,aux);
		printf("Se desbloqueo el ESI con el ID: %s.\n",aux->ID);
	}else{ // No existe el esi en listos ni en ejecucion. notifico al usuario
		printf("No se Encontro ningun ESI bloqueado por esa clave, por favor intente con otro.\n");
	}

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

void killProceso(char* id){

	bool comparadorID(t_ESIPlanificador* unESI){
		return !strcmp(unESI->ID, id);
	}

	//Busco el ESI en todas las listas (Listos, Bloqueados, Ejecucion)
	//Lo Busco en Listos
	t_ESIPlanificador* ESIenListos = (t_ESIPlanificador*) list_remove_by_condition(ESI_listos,(void*)comparadorID);
	if(ESIenListos != NULL){//Encontramos el ESI en listos
		printf("El ESI con el ID ingresado (%s), se encontro en la cola de Listos, se paso a finalizado.\n",id);
		list_add(ESI_finalizados,ESIenListos);
		EnviarDatosTipo(ESIenListos->socket,PLANIFICADOR, NULL, 0, t_ABORTARESI);
	}else{//No esta en la lista de listos, libero aux para poder reutilizarlo
		free(ESIenListos);
		//Lo Busco en Ejecucion
		t_ESIPlanificador* ESIenEjecucion = (t_ESIPlanificador*) list_remove_by_condition(ESI_ejecucion,(void*)comparadorID);
		if(ESIenEjecucion != NULL){//Encontramos el ESI en ejecucion
			printf("El ESI con el ID ingresado (%s), se encontro en la cola de Ejecucion, se paso a finalizado.\n",id);
			list_add(ESI_finalizados,ESIenEjecucion);
			EnviarDatosTipo(ESIenEjecucion->socket,PLANIFICADOR, NULL, 0, t_ABORTARESI);
		}else{ // No existe el esi en listos ni en ejecucion.
			free(ESIenEjecucion);
			//Lo Busco en Bloqueados
			t_ESIPlanificador* ESIenBloqueados = (t_ESIPlanificador*) list_remove_by_condition(ESI_bloqueados,(void*)comparadorID);
			if(ESIenBloqueados != NULL){//Encontramos el ESI en bloqueados
				printf("El ESI con el ID ingresado (%s), se encontro en la cola de Bloqueados, se paso a finalizado.\n",id);
				list_add(ESI_finalizados,ESIenBloqueados);
				EnviarDatosTipo(ESIenBloqueados->socket,PLANIFICADOR, NULL, 0, t_ABORTARESI);
			}else{ // No existe el esi en listos ni en ejecucion ni en bloqueados, se le notifica al usuario que no se encontro el ID solicitado.
				printf("No se Encontro el ID especificado, por favor intente con otro.\n");
				free(ESIenBloqueados);//libero aux para que no ocupe memoria un NULL
			}
		}
	}
}

void status(){

}

void deadlock(){
	int i;
	int cantidad_de_bloqueados = list_size(ESI_bloqueados);
	esis_en_deadlock = list_create();

	for (i = 0; i < cantidad_de_bloqueados ; i++) {
		t_ESIPlanificador* esi_actual = (t_ESIPlanificador*) list_get(ESI_bloqueados,i);
		char* clave_que_necesita = malloc (strlen(esi_actual->razon_bloqueo) + 1);
		strcpy(clave_que_necesita,esi_actual->razon_bloqueo);

		if(!verificar_si_hay_circulo()){
			verificar_si_alguien_tiene_el_recurso(clave_que_necesita);
		}

		free(clave_que_necesita);
		//elimino el elemento repetido de la lista
		list_remove(esis_en_deadlock,list_size(esis_en_deadlock)-1);

	}// imprimo el deadlock encontrado
	printf("Los siguientes esis estan en deadlock: ");
	int j;
	for (j = 0; j < list_size(esis_en_deadlock) ; j++) {
		t_ESIPlanificador* aux = (t_ESIPlanificador*)list_get(esis_en_deadlock,j);
		if(j == list_size(esis_en_deadlock)-1){
			printf("%s.\n ",aux->ID);
		}else{
			printf("%s, ",aux->ID);
		}
	}
	list_destroy(esis_en_deadlock);
}


bool tiene_clave_tomada(t_ESIPlanificador* esi, char* clave ) {

	bool esta_la_clave(char* laClave ) {
		return !strcmp(laClave, clave);
	}

	return list_any_satisfy(esi->clave, (void*)esta_la_clave);
}

bool comparador_de_esis(t_ESIPlanificador* unESI, t_ESIPlanificador* otroESI){
	return !strcmp(unESI->ID, otroESI->ID);
}

void verificar_si_alguien_tiene_el_recurso(char* clave){

	int k;
	int cantidad_de_bloqueados = list_size(ESI_bloqueados);
	for (k = 0; k < cantidad_de_bloqueados ; k++) {
		t_ESIPlanificador* esi_actual = (t_ESIPlanificador*) list_get(ESI_bloqueados,k);

		if(tiene_clave_tomada(esi_actual,clave)){//Encontre el esi que tiene tomada la clave
			//tengo que verificar si este esi esta bloqueado esperando otro
			list_add(esis_en_deadlock,esi_actual);
			if(!verificar_si_hay_circulo()){
				verificar_si_alguien_tiene_el_recurso(esi_actual->razon_bloqueo);
			}
		}
	}
}


bool verificar_si_hay_circulo(){
	return list_element_repeats(esis_en_deadlock,(void*)comparador_de_esis);
}
