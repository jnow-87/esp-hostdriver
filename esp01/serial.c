#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <types.h>
#include "serial.h"


/* local/static prototypes */
static void *read_thread(void *arg);


/* global functions */
serial_t *serial_init(char const *path){
	serial_t *dev;
	struct termios attr;


	dev = malloc(sizeof(serial_t));

	if(dev == 0x0)
		goto err_0;

	dev->fd = open(path, O_RDWR);

	if(dev->fd == -1)
		goto err_0;

	if(tcgetattr(dev->fd, &dev->term_attr) != 0)
		goto err_1;

	attr = dev->term_attr;

	attr.c_iflag = IXON | ICRNL | IGNCR;
	attr.c_oflag = ONLCR | OPOST;
	attr.c_lflag = IEXTEN | ECHOE | ECHOCTL | ECHOK | ECHOE | ISIG;
	attr.c_cflag = CBAUDEX | CLOCAL | HUPCL | CREAD;

	attr.c_cflag |= CS8;
	attr.c_cflag &= ~(PARENB | CSTOPB);
	cfsetspeed(&attr, B115200);

	if(tcsetattr(dev->fd, TCSANOW, &attr) != 0)
		goto err_2;

	if(pthread_create(&dev->read_thread_tid, 0, read_thread, dev) != 0)
		goto err_2;

	return 0;


err_2:
	tcsetattr(dev->fd, TCSANOW, &dev->term_attr);

err_1:
	close(dev->fd);

err_0:
	return 0x0;
}

void serial_close(serial_t *dev){
	tcsetattr(dev->fd, TCSANOW, &dev->term_attr);
	pthread_cancel(dev->read_thread_tid);

	free(dev);
}

int serial_send_cmd(serial_t *dev, char const *cmd, bool resp){
	if(cmd == 0x0 || *cmd == 0)
		return 0;

	printf("command \"%s\"\n", cmd);

	write(dev->fd, cmd, strlen(cmd));
	write(dev->fd, "\n", 1);

	// TODO
	usleep(2000000);

	return 0;
}


/* local functions */
static void *read_thread(void *arg){
	char c;
	serial_t *dev;


	dev = (serial_t*)arg;

	while(1){
		if(read(dev->fd, &c, 1) != 1)
			break;

		printf("%c", c);
	}

	return 0x0;
}
