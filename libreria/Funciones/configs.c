#include "configs.h"

void leerConfig(char * configPath,t_log* logger) {
	leerArchivoDeConfiguracion(configPath,logger);
	log_info(logger, "Archivo de configuracion leido correctamente");
}

void leerArchivoDeConfiguracion(char * configPath,t_log* logger) {
 	t_config * archivoConfig;

 	archivoConfig = config_create(configPath);

 	if (archivoConfig == NULL){
 		perror("[ERROR] Archivo de configurarchcion no encontrado");
 		log_error(logger,"Archivo de configurarchcion no encontrado");
 	}

 	setearValores(archivoConfig);
 	config_destroy(archivoConfig);
 }
