#include "ESI.h"


int main(int argc, char ** argv) {

	char* instancia_log_ruta = strdup("/home/utnso/Proyectos/tp-2018-1c-PC-citos/ESI/ESI.log");
	crearLogger(instancia_log_ruta, "ESI", 0);
	char* instancia_config_ruta = strdup("/home/utnso/Proyectos/tp-2018-1c-PC-citos/ESI/ESI.cfg");
	leerConfig(instancia_config_ruta);
	atenderCoordinador();
	atenderPlanificador();

	for(;;);
	return EXIT_SUCCESS;
}


/*
int main(int argc, char **argv){
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
*/

