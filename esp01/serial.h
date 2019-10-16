#ifndef SERIAL_H
#define SERIAL_H


#include <pthread.h>
#include <termios.h>


/* types */
typedef struct{
	int fd;
	struct termios term_attr;

	pthread_t read_thread_tid;
} serial_t;


/* global functions */
serial_t *serial_init(char const *path);
void serial_close(serial_t *dev);

int serial_send_cmd(serial_t *dev, char const *cmd, bool resp);


#endif // SERIAL_H
