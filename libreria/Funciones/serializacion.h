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
	t_ABORTARESI,
	t_SIGUIENTELINEA,
	t_RESPUESTALINEACORRECTA,
	t_RESPUESTALINEAINCORRECTA,
	t_RESPUESTASET,
	t_RESPUESTASTORE,
	t_CLAVEBORRADA,
	t_LEERCLAVE
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
	pthread_t hilo;
	int socket;
} t_prueba_hilo;


typedef struct {
	char* clave;
}__attribute__((packed)) t_PlanificadorCoordinador;

typedef struct {
	int socket;
	char* ID;
	bool bloqueado;
	char* razon_bloqueo;//clave que lo bloqueo o "consola"
	int rafagas_ejecutadas;
	float rafagas_estimadas;
	int response_ratio;
	int tiempo_espera;
}__attribute__((packed)) t_ESIPlanificador;

typedef struct {
	char* clave;
	char* valor;
}__attribute__((packed)) t_ESICoordinador;

typedef struct {
	char* ID;
	t_list* clave;
}__attribute__((packed)) t_ESI;

typedef struct {
	int socket;
	char* nombre;
	bool estado_de_conexion;
	bool flagEL;
	int tamanio_ocupado;
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

char* getNombreDelProceso(proceso proceso);

bool EnviarHandshake(int socketFD, proceso quienEnvia);

bool EnviarDatosTipo(int socketFD, proceso quienEnvia, void* datos, int tamDatos, t_protocolo tipoMensaje);

bool EnviarPaquete(int socketCliente, Paquete* paquete);

int RecibirDatos(void* paquete, int socketFD, uint32_t cantARecibir);

int RecibirPaqueteServidor(int socketFD, proceso quienEnvia, Paquete* paquete); //Responde al recibir un Handshake

int RecibirPaqueteCliente(int socketFD, proceso quienEnvia, Paquete* paquete); //No responde los Handshakes


#endif /* SERIALIZACION_H_ */
