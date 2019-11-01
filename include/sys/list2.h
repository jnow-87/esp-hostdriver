/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 *
 * \brief	Macro implementation for double-linked lists. The names for
 * 			the next and prev link elements have to be specified as
 * 			parameter.
 *
 * This version of the list macros uses the head element as functional
 * part of the list, i.e. it also stores data. Therefor, the list head
 * is required to be a pointer. head->prev points to the last element
 * of the list, while the list is delimited by 0x0 with
 * head->prev->next = 0x0
 */



#ifndef SYS_LIST2_H
#define SYS_LIST2_H


#include <string.h>


/**
 * \brief	initialize the head of a list
 *
 * \param	head		pointer to list head
 * \param	prev_name	name of the list prev pointer
 * \param	next_name	name of the list next pointer
 *
 * \return	none
 */
#define __list2_init(head, prev_name, next_name){ \
	(head)->prev_name = (head); \
	(head)->next_name = 0x0; \
}

/**
 * \brief	add an element at head of the list
 *
 * \param	head		pointer to list head
 * \param	el			pointer to element to insert
 * \param	prev_name	name of the list prev pointer
 * \param	next_name	name of the list next pointer
 *
 * \return	none
 */
#define __list2_add_head(head, el, prev_name, next_name){ \
	typeof(el) _el = (el); \
	\
	\
	_el->next_name = (head); \
	_el->prev_name = ((head) == 0x0 ? _el : (head)->prev_name); \
	\
	if((head) != 0x0) \
		(head)->prev_name = _el; \
	\
	(head) = _el; \
}

/**
 * \brief	add an element at the end of the list
 *
 * \param	head		pointer to list head
 * \param	el			pointer to element to insert
 * \param	prev_name	name of the list prev pointer
 * \param	next_name	name of the list next pointer
 *
 * \return	none
 */
#define __list2_add_tail(head, el, prev_name, next_name){ \
	typeof(el) _el = (el); \
	\
	\
	_el->next_name = 0x0; \
	\
	if((head) == 0x0){ \
		(head) = _el; \
		_el->prev_name = _el; \
	} \
	else{ \
		_el->prev_name = (head)->prev_name; \
		(head)->prev_name->next_name = _el; \
		(head)->prev_name = _el; \
	} \
}

/**
 * \brief	add an element between the two given elements
 *
 * \param	el			pointer to element to insert
 * \param	front		pointer to element that is in front of el
 * \param	back		pointer to element that is after el
 * \param	prev_name	name of the list prev pointer
 * \param	next_name	name of the list next pointer
 *
 * \return	none
 */
#define __list2_add_in(el, front, back, prev_name, next_name){ \
	typeof(el) _el = el; \
	typeof(el) _front = front; \
	typeof(el) _back = back; \
	\
	\
	_el->prev_name = front; \
	_el->next_name = back; \
	_front->next_name = _el; \
	_back->prev_name = _el; \
}

/**
 * \brief	remove an element and insert a new one at its location
 *
 * \param	head		pointer to list head
 * \param	old			element to replace
 * \param	new			element to insert
 * \param	prev_name	name of the list prev pointer
 * \param	next_name	name of the list next pointer
 *
 * \return	none
 */
#define __list2_replace(head, old, new, prev_name, next_name){ \
	typeof(new) _new = (new); \
	\
	\
	if((head) == 0x0){ \
		(head) = _new; \
		_new->prev_name = _new; \
		_new->next_name = 0x0; \
	} \
	else{ \
		_new->prev_name = ((head)->next_name == 0x0) ? _new : (old)->prev_name; \
		_new->next_name = (old)->next_name; \
		\
		if((old)->next_name)	(old)->next_name->prev_name = _new; \
		else					(head)->prev_name = _new; \
		\
		if((old) != (head))		(old)->prev_name->next_name = _new; \
		else					(head) = _new; \
	} \
}

/**
 * \brief	remove element from list
 *
 * \param	head		pointer to list head
 * \param	el			pointer to element that shall be removed
 * \param	prev_name	name of the list prev pointer
 * \param	next_name	name of the list next pointer
 *
 * \return	none
 */
#define __list2_rm(head, el, prev_name, next_name){ \
	if((el) != (head))			(el)->prev_name->next_name = (el)->next_name; \
	\
	if((el)->next_name != 0x0)	(el)->next_name->prev_name = (el)->prev_name; \
	else						(head)->prev_name = (el)->prev_name; \
	\
	if((el) == (head))			(head) = (el)->next_name; \
}

/**
 * \brief	check if the given element is part of the list
 *
 * \param	head		pointer to list head
 * \param	el			the element to be checked
 * \param	prev_name	name of the list prev pointer
 * \param	next_name	name of the list next pointer
 *
 * \return	true if the element is part of the list
 * 			false otherwise
 */
#define __list2_contains(head, el, prev_name, next_name)({ \
	typeof(head) _head = (head); \
\
	for(;_head!=0x0; _head=_head->next_name){ \
		if(_head == (el)) \
			break; \
	} \
\
	(_head == (el)); \
})


#endif // SYS_LIST2_H
