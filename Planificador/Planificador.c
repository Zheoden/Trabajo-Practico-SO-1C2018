#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

void main() {
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
