#include "ESI.h"

int crearCliente(void) {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(IP);
	direccionServidor.sin_port = htons(PUERTO);

	int cliente = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(cliente,(void*) &direccionServidor,sizeof(direccionServidor))!=0){
		perror("No se pudo conectar");
		return 1;
	}

//	while(1){
		char mensaje [1000];
		scanf("%s", mensaje);

		send(cliente, mensaje, strlen(mensaje), 0);
//	}

	return 0;
}
