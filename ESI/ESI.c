#include "ESI.h"

int crearClienteCoor() {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(coordinador_ip);
	direccionServidor.sin_port = htons(coordinador_puerto);

	int cliente = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(cliente,(void*) &direccionServidor,sizeof(direccionServidor))!=0){
		perror("No se pudo conectar");
//		return 1;
	}

	while(1){
		char mensaje [1000];
		scanf("%s", mensaje);
		send(cliente, mensaje, strlen(mensaje), 0);


     	char* buffer = malloc(30);
     	int bytesRecibidos = recv(cliente,buffer, 25, 0);
     	if (bytesRecibidos <= 0) {
     		perror("El chabon se desconecto o bla bla bla");
//     		return 1;
     	}
     	buffer[bytesRecibidos] = '\0';
     	printf("me llegaron %d bytes con %s\n", bytesRecibidos, buffer);
     	free(buffer);
	}

//	return 0;
}

int crearClientePlanif() {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(planificador_ip);
	direccionServidor.sin_port = htons(planificador_puerto);

	int cliente = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(cliente,(void*) &direccionServidor,sizeof(direccionServidor))!=0){
		perror("No se pudo conectar");
		return 1;
	}

	EnviarHandshake(cliente,ESI);

	while(1){
		char mensaje [1000];
		scanf("%s", mensaje);
		send(cliente, mensaje, strlen(mensaje), 0);


     	char* buffer = malloc(30);
     	int bytesRecibidos = recv(cliente,buffer, 25, 0);
     	if (bytesRecibidos <= 0) {
     		perror("El chabon se desconecto o bla bla bla");
     		return 1;
     	}
     	buffer[bytesRecibidos] = '\0';
     	printf("me llegaron %d bytes con %s\n", bytesRecibidos, buffer);
     	free(buffer);
	}

	return 0;
}

void atenderPlanificador(){
	pthread_t hilo;
	pthread_create(&hilo, NULL, (void *) crearClientePlanif, NULL);
	pthread_detach(hilo);
}

void atenderCoordinador(){
	pthread_t unHilo;
	pthread_create(&unHilo, NULL, (void *) crearClienteCoor,NULL);
	pthread_detach(unHilo);
}

void setearValores(t_config * archivoConfig) {
 	planificador_puerto = config_get_int_value(archivoConfig, "PLANIFICADOR_PUERTO");
 	planificador_ip = strdup(config_get_string_value(archivoConfig, "PLANIFICADOR_IP"));
 	coordinador_puerto = config_get_int_value(archivoConfig, "COORDINADOR_PUERTO");
 	coordinador_ip = strdup(config_get_string_value(archivoConfig, "COORDINADOR_IP"));
 }

int cantidadDeApariciones(char * cadena, char separador){
	int i;
	int cont = 0;
	for(i=0;i < strlen(cadena);i++){
		if (cadena[i] == separador){
			cont++;
		}
	}
	return cont;
}


int parsear(int argc, char **argv){
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    t_esi_operacion parsed;
    char* operacion = string_new();
    esi unEsi;

    fp = fopen("/home/utnso/Proyectos/tp-2018-1c-PC-citos/ESI/script.esi", "r");
    if (fp == NULL){
        perror("Error al abrir el archivo: ");
        exit(EXIT_FAILURE);
    }

    while ((read = getline(&line, &len, fp)) != -1) {
        parsed = parse(line);
        if(parsed.valido){
            switch(parsed.keyword){
                case GET:
                    printf("%d\tclave: <%s>\n", parsed.keyword ,parsed.argumentos.GET.clave);
                    string_append(&operacion, "00");
                    string_append(&operacion," ");
                    string_append(&operacion,parsed.argumentos.GET.clave);
                    string_append(&operacion,"#");
                    break;
                case SET:
                    printf("SET\tclave: <%s>\tvalor: <%s>\n", parsed.argumentos.SET.clave, parsed.argumentos.SET.valor);
                    string_append(&operacion,"01");
                    string_append(&operacion," ");
                    string_append(&operacion,parsed.argumentos.SET.clave);
                    string_append(&operacion," ");
                    string_append(&operacion,parsed.argumentos.SET.valor);
                    string_append(&operacion,"#");
                    break;
                case STORE:
                    printf("STORE\tclave: <%s>\n", parsed.argumentos.STORE.clave);
                    string_append(&operacion,"02");
                    string_append(&operacion," ");
                    string_append(&operacion,parsed.argumentos.STORE.clave);
                    string_append(&operacion,"#");
                    break;
                default:
                    fprintf(stderr, "No pude interpretar <%s>\n", line);
                    exit(EXIT_FAILURE);
            }

            destruir_operacion(parsed);
        } else {
            fprintf(stderr, "La linea <%s> no es valida\n", line);
            exit(EXIT_FAILURE);
        }

    }

    fclose(fp);
    if (line)
        free(line);


    int i;
    printf("%s\n", operacion);
    for(i = 0;i < (int)cantidadDeApariciones(operacion,'#');i++){
    	printf("%s\n", string_split(operacion,"#")[i]);
    }

    return EXIT_SUCCESS;
}
