#include "Planificador.h"


void consola(){
  char * linea;
  while(1) {
    linea = readline(">");

    if(linea)
      add_history(linea);
    if(!strncmp(linea, "exit", 4)) {
       free(linea);
       break;
    }
    printf("%s\n", linea);
    free(linea);
  }
}

