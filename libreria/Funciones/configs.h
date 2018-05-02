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
/*
 * Demostracion de uso de la funcion setearValores
 *
 * void setearValores(t_config * archivoConfig){
 * 		Copiar y Pegar el codigo correspondiente a cada tipo de dato
 * 		case int:
 * 			int variable = config_get_int_value(archivoConfig, "OBJETIVO");
 * 		case string:
 * 			char* variable = strdup(config_get_string_value(archivoConfig, "OBJETIVO"));
 * 		case long:
 * 			long variable = config_get_long_value(archivoConfig,"OBJETIVO");
 * 		case double
 * 			double variable = config_get_double_value(archivoConfig,"OBJETIVO");
 * 		case array:
 * 			char ** array = config_get_array_value(archivoConfig,"OBJETIVO");
 * }
 */

#endif /* CONFIG_H_ */
