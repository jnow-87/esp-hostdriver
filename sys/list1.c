/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/list.h>
#include <sys/list1.h>


/* global functions */
void _list1_init(list1_t *head, list1_t **tail){
	__list1_init(head, *tail, next);
}

void _list1_add_head(list1_t **head, list1_t **tail, list1_t *el){
	__list1_add_head(*head, *tail, el, next);
}

void _list1_add_tail(list1_t **head, list1_t **tail, list1_t *el){
	__list1_add_tail(*head, *tail, el, next);
}

void _list1_rm_head(list1_t **head, list1_t **tail){
	__list1_rm_head(*head, *tail, next);
}
