#include "archivos.h"

const char* get_filename(const char* path){
	const char *file = strrchr(path,'/');
	if(!file || file == path) return "";
	return file +1;
}

const char* get_file_extension(const char* filename){
	const char *extension = strrchr(filename,'.');
	if(!extension || extension == filename) return "";
	return extension +1;
}
