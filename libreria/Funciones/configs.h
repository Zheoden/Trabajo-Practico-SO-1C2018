#ifndef CONFIGS_H_
#define CONFIGS_H_
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <commons/config.h>
#include <commons/log.h>


// Prototipos
void leerConfig(char * configPath,t_log* logger);
void setearValores(t_config * archivoConfig); // Redefinir en cada proyecto
void leerArchivoDeConfiguracion(char * configPath,t_log* logger);


#endif /* CONFIG_H_ */
