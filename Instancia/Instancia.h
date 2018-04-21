#ifndef CLIENTE_H_
#define CLIENTE_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h> // Para malloc
#include <openssl/md5.h> // Para calcular el MD5
#include <netdb.h> // Para getaddrinfo
#include <unistd.h> // Para close
#include <sys/socket.h> // Para crear sockets, enviar, recibir, etc
#include <readline/readline.h> // Para usar readline
#include <commons/log.h>
#include <commons/collections/list.h>
#define IP "127.0.0.1"
#define PUERTO 8080

int crearCliente(void);

#endif /* CLIENTE_H_ */
