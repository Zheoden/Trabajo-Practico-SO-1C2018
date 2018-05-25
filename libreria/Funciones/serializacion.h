#ifndef SERIALIZACION_H_
#define SERIALIZACION_H_
#include <stdlib.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <commons/collections/list.h>
#include <readline/readline.h>
#include <stdbool.h>
#include <netinet/in.h>


//////////////////////////////////////////
//           Tipos de Mensajes          //
//////////////////////////////////////////
typedef enum t_protocolo {
	t_GET = 1,
	t_SET,
	t_STORE,
	t_HANDSHAKE,
	t_SOLICITUDNOMBRE,
	t_CONFIGURACIONINSTANCIA,
	t_IDENTIFICACIONINSTANCIA,
	t_RESPUESTASET,
	t_NUEVOESI,
	t_ABORTARESI
} t_protocolo;

//////////////////////////////////////////
//               Procesos               //
//////////////////////////////////////////

typedef enum proceso {
	ESI=1,
	COORDINADOR,
	PLANIFICADOR,
	INSTANCIA
} proceso;

typedef struct {
	char* nombre;
	int tamanio;
} nombreProceso;

//////////////////////////////////////////
//           Comunicacion Base          //
//////////////////////////////////////////

typedef struct {
	t_protocolo tipoMensaje;
	proceso quienEnvia;
	int tamanioMensaje;
}__attribute__((packed)) Header;

typedef struct {
	Header header;
	void* mensaje;
}__attribute__((packed)) Paquete;

//////////////////////////////////////////
//          Estructuras Utiles          //
//////////////////////////////////////////

typedef struct {
	char* ID;
	char* clave;
	char* valor;
	bool bloqueado;
	char* razon_bloqueo;
}__attribute__((packed)) t_ESIPlanificador;

typedef struct {
	char* ID;
	char* clave;
	char* valor;
}__attribute__((packed)) t_ESICoordinador;

typedef struct {
	char* ID;
	t_list* clave;
}__attribute__((packed)) t_listaDeESICoordinador;

typedef struct {
	int socket;
	char* nombre;
	bool activo;
	t_list* claves;
}__attribute__((packed)) t_Instancia;

typedef struct {
	char* clave;
	int index;
	int entradasOcupadas;
	int tamanio;
}__attribute__((packed)) t_AlmacenamientoEntradaAdministrativa;

//////////////////////////////////////////
//              Funciones               //
//////////////////////////////////////////

nombreProceso getNombreDelProceso(proceso proceso);

bool EnviarHandshake(int socketFD, proceso quienEnvia);

bool EnviarDatos(int socketFD, proceso quienEnvia, void* datos, int tamDatos);

bool EnviarDatosTipo(int socketFD, proceso quienEnvia, void* datos, int tamDatos, t_protocolo tipoMensaje);

bool EnviarMensaje(int socketFD, char* msg, proceso quienEnvia);

bool EnviarPaquete(int socketCliente, Paquete* paquete);

bool RecibirHandshake(int socketFD, proceso quienEnvia);

int RecibirDatos(void* paquete, int socketFD, uint32_t cantARecibir);

int RecibirPaqueteServidor(int socketFD, proceso quienEnvia, Paquete* paquete); //Responde al recibir un Handshake

int RecibirPaqueteCliente(int socketFD, proceso quienEnvia, Paquete* paquete); //No responde los Handshakes

#endif /* SOCKETS_H_ */
