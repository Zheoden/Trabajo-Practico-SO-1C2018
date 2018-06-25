#ifndef CONEXIONES_H_
#define CONEXIONES_H_
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/sendfile.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <commons/collections/list.h>
#include <readline/readline.h>
#include <stdbool.h>



//////////////////////////////////////////
//       Funciones De Servidores        //
//////////////////////////////////////////

int ConectarAServidor(int puerto, char* ip);


#endif /* CONEXIONES_H_ */
