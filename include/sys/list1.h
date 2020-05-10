/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 *
 * \brief	Macro implementation for single-linked lists. The name for
 * 			the next link element has to be specified as parameter.
 *
 * This version of the list macros uses the head element as functional
 * part of the list, i.e. it also stores data. Therefor, the list head
 * is required to be a pointer. head->prev points to the last element
 * of the list, while the list is delimited by 0x0 with
 * head->prev->next = 0x0
 */



#ifndef SYS_LIST1_H
#define SYS_LIST1_H


#include <string.h>


/**
 * \brief	initialize the head of a list
 *
 * \param	head		pointer to list head
 * \param	tail		pointer to list tail
 * \param	next_name	name of the list next pointer
 *
 * \return	none
 */
#define __list1_init(head, tail, next_name){ \
	(head)->next_name = 0x0; \
	(tail) = (head); \
}

/**
 * \brief	add an element at head of the list
 *
 * \param	head		pointer to list head
 * \param	tail		pointer to list tail
 * \param	el			pointer to element to insert
 * \param	next_name	name of the list next pointer
 *
 * \return	none
 */
#define __list1_add_head(head, tail, el, next_name){ \
	typeof(el) _el = (el); \
	\
	\
	_el->next_name = (head); \
	(head) = _el; \
	\
	if((tail) == 0x0) \
		(tail) = (head); \
}

/**
 * \brief	add an element at the end of the list
 *
 * \param	head		pointer to list head
 * \param   tail		pointer to list tail
 * \param	el			pointer to element to insert
 * \param	next_name	name of the list next pointer
 *
 * \return	none
 */
#define __list1_add_tail(head, tail, el, next_name){ \
	typeof(el) _el = (el); \
	\
	\
	_el->next_name = 0x0; \
	\
	if((tail) != 0x0) \
		(tail)->next_name = _el; \
	\
	(tail) = _el; \
	\
	if((head) == 0x0) \
		(head) = _el; \
}

/**
 * \brief	remove element from list
 *
 * \param	head		pointer to list head
 * \param	tail		pointer to list tail
 * \param	next_name	name of the list next pointer
 *
 * \return	none
 */
#define __list1_rm_head(head, tail, next_name){ \
	if((tail) == (head)) \
		(tail) = 0x0; \
	\
	(head) = (head)->next_name; \
}


#endif // SYS_LIST1_H
