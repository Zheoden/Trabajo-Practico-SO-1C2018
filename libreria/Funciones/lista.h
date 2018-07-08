#ifndef LISTA_H_
#define LISTA_H_
#include <stdlib.h>
#include <commons/collections/list.h>
#include <stdbool.h>

int list_get_index(t_list* self,void* elemento,bool (*comparator)(void *,void *));
bool list_element_repeats(t_list* self, bool (*comparator)(void *,void *));

#endif /* LISTA_H_ */
