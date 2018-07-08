#include "lista.h"

int list_get_index(t_list* self,void* elemento,bool (*comparator)(void *,void *)){
	int longitud_de_lista = list_size(self);
	int i;
	int cont = 0;
	for(i = 0; i < longitud_de_lista; i++ ){
		if(!comparator(list_get(self,i),elemento)){
			cont++;
		}else{
			break;
		}
	}
	return cont;
}


bool list_element_repeats(t_list* self, bool (*comparator)(void *,void *)){
	int longitud_de_lista = list_size(self);
	int i,j;
	if(longitud_de_lista > 1){
		for(i = 0; i < longitud_de_lista; i++ ){
			for(j = 0; j < longitud_de_lista; j++ ){
				if( (comparator(list_get(self,i),list_get(self,j))) && i != j ){
					return true;
				}
			}
		}
	}
	return false;
}
