#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(void) {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons(8080);

	int servidor = socket(AF_INET, SOCK_STREAM, 0);

	int activado = 1;
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	if (bind(servidor, (void*) &direccionServidor, sizeof(direccionServidor)) !=0){
		perror("falló el bind");
		return 1;
	}
	printf("Estoy Escuchando\n");
	listen(servidor,100);


	struct sockaddr_in direccionCliente;
	unsigned int tamanoDireccion;
	int cliente = accept(servidor, (void*)&direccionCliente, &tamanoDireccion);

	printf("Recibi una conexion en %d!! \n", cliente);
	send(cliente, "Hola NetCat!\n",18,0);
	send(cliente, ":)\n",4,0);
/*
	char* buffer = malloc(5);

	int bytesRecibidos = recv(cliente,buffer, 4, MSG_WAITALL);
	if (bytesRecibidos <= 0) {
		perror("El chabon se desconecto o bla bla bla");
		return 1;
	}

	buffer[bytesRecibidos] = "\0";
	printf("me llegaron %d bytes con %s", bytesRecibidos, buffer);

	free(buffer);
*/
	return 0;
}
