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

void sigchld_handler(int s)
 {
     while(wait(NULL) > 0);
 }

 int crearServidor2(void)
 {
     int sockfd, cliente;  // Escuchar sobre sock_fd, nuevas conexiones sobre new_fd
     struct sockaddr_in my_addr;    // información sobre mi dirección
     struct sockaddr_in their_addr; // información sobre la dirección del cliente
     int sin_size;
     struct sigaction sa;
     int yes=1;

     if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
         perror("socket");
         exit(1);
     }

     if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
         perror("setsockopt");
         exit(1);
     }

     my_addr.sin_family = AF_INET;         // Ordenación de bytes de la máquina
     my_addr.sin_port = htons(PUERTO);     // short, Ordenación de bytes de la red
     my_addr.sin_addr.s_addr = INADDR_ANY; // Rellenar con mi dirección IP
     memset(&(my_addr.sin_zero), '\0', 8); // Poner a cero el resto de la estructura

     if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr))
                                                                    == -1) {
         perror("bind");
         exit(1);
     }

     printf("Estoy Escuchando\n");

     if (listen(sockfd, SOMAXCONN) == -1) {
         perror("listen");
         exit(1);
     }

     sa.sa_handler = sigchld_handler; // Eliminar procesos muertos
     sigemptyset(&sa.sa_mask);
     sa.sa_flags = SA_RESTART;
     if (sigaction(SIGCHLD, &sa, NULL) == -1) {
         perror("sigaction");
         exit(1);
     }

     while(1) {  // main accept() loop
         sin_size = sizeof(struct sockaddr_in);
         if ((cliente = accept(sockfd, (struct sockaddr *)&their_addr,
                                                        &sin_size)) == -1) {
             perror("accept");
             continue;
         }
         printf("server: Llego una conexion de: %s\n",
                                            inet_ntoa(their_addr.sin_addr));

         while(1){
        	 char* buffer = malloc(30);
        	 int bytesRecibidos = recv(cliente,buffer, 25, 0);
        	 if (bytesRecibidos <= 0 || buffer[0] == 't') {
        		 break;
        	 }
        	 buffer[bytesRecibidos] = (char)'\0';
        	 printf("me llegaron %d bytes con %s\n", bytesRecibidos, buffer);
        	 free(buffer);

        	 send(cliente, "Hello, world!", 14, 0);

         }

         if (!fork()) { // Este es el proceso hijo
             close(sockfd); // El hijo no necesita este descriptor
             if (send(cliente, "Hello, world!", 14, 0) == -1)
            	 perror("send");
             	 perror("Voy a cerrar la conexion!!!");
             close(cliente);
             exit(0);
         }
         printf("Voy a cerrar la conexion con %d!!\n", cliente);
         close(cliente);  // El proceso padre no lo necesita
     }

     return 0;
 }


