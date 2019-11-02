/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_RINGBUF_H
#define SYS_RINGBUF_H


#include <sys/types.h>


/* macros */
#define RINGBUF_INITIALISER(_data, _size){ \
	.rd = _size - 1, \
	.wr = 0, \
	.size = _size, \
	.data = _data, \
}


/* types */
typedef struct{
	size_t rd,
		   wr,
		   size;

	void *data;
} ringbuf_t;


/* prototypes */
void ringbuf_init(ringbuf_t *buf, void *data, size_t n);
size_t ringbuf_read(ringbuf_t *buf, void *data, size_t n);
size_t ringbuf_write(ringbuf_t *buf, void *data, size_t n);
size_t ringbuf_contains(ringbuf_t *buf);
size_t ringbuf_left(ringbuf_t *buf);
int ringbuf_empty(ringbuf_t *buf);
int ringbuf_full(ringbuf_t *buf);


#endif // SYS_RINGBUF_H
