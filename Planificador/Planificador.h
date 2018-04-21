#ifndef CLIENTE_H_
#define CLIENTE_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h> // Para malloc
#include <netdb.h> // Para getaddrinfo
#include <unistd.h> // Para close
#include <signal.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/md5.h> // Para calcular el MD5
#include <sys/socket.h> // Para crear sockets, enviar, recibir, etc
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h> // Para usar readline
#include <readline/history.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#define IP "127.0.0.1"
#define PUERTO 8080

void consola();

int crearServidor(void);
void sigchld_handler(int s);

#endif /* CLIENTE_H_ */
