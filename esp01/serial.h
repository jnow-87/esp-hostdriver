#ifndef SERIAL_H
#define SERIAL_H


#include <pthread.h>
#include <termios.h>
#include <sys/ringbuf.h>


/* types */
typedef struct{
	int fd;
	struct termios term_attr;

	pthread_t read_thread_tid;

	ringbuf_t rx_buf;
} serial_t;


/* global functions */
serial_t *serial_init(char const *path);
void serial_close(serial_t *dev);

size_t serial_read(serial_t *dev, void *data, size_t n);

void serial_cmd_start(serial_t *dev, char const *resp);
void serial_cmd_send(serial_t *dev, char const *s);
void serial_cmd_send_char(serial_t *dev, char c);
int serial_cmd_wait(serial_t *dev);
int serial_cmd(serial_t *dev, char const *cmd, char const *resp);


#endif // SERIAL_H
