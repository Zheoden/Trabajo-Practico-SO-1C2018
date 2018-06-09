#include "conexiones.h"

int ConectarAServidor(int puerto, char* ip) {
	int socketFD = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in direccion;

	direccion.sin_family = AF_INET;
	direccion.sin_port = htons(puerto);
	direccion.sin_addr.s_addr = inet_addr(ip);
	memset(&(direccion.sin_zero), '\0', 8);

	while (connect(socketFD, (struct sockaddr *) &direccion, sizeof(struct sockaddr))<0)
		sleep(1); //Espera un segundo y se vuelve a tratar de conectar.
	return socketFD;

}
