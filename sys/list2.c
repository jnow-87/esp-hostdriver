/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/list.h>
#include <sys/list2.h>


/* global functions */
void _list2_init(list2_t *head){
	__list2_init(head, prev, next);
}

void _list2_add_head(list2_t **head, list2_t *el){
	__list2_add_head(*head, el, prev, next);
}

void _list2_add_tail(list2_t **head, list2_t *el){
	__list2_add_tail(*head, el, prev, next);
}

void _list2_add_in(list2_t *el, list2_t *front, list2_t *back){
	__list2_add_in(el, front, back, prev, next);
}

void _list2_replace(list2_t **head, list2_t *old, list2_t *new){
	__list2_replace(*head, old, new, prev, next);
}

void _list2_rm(list2_t **head, list2_t *el){
	__list2_rm(*head, el, prev, next);
}

bool _list2_contains(list2_t *head, list2_t *el){
	return __list2_contains(head, el, prev, next);
}

list2_t *_list2_find(list2_t *head, size_t mem_offset, void const *ref, size_t size){
	list2_t *el;


	list_for_each(head, el){
		if(memcmp(((char*)el) + mem_offset, ref, size) == 0)
			return el;
	}

	return 0x0;
}

list2_t *_list2_find_str(list2_t *head, size_t mem_offset, char const *ref, size_t len, bool is_array){
	list2_t *el;
	char *s;


	list_for_each(head, el){
		if(is_array)	s = ((char*)el) + mem_offset;
		else			s = *((char**)(((char*)el) + mem_offset));

		if(len == 0){
			if(strcmp(s, ref) == 0)
				return el;
		}
		else if(strncmp(s, ref, len) == 0)
			return el;
	}

	return 0x0;
}
