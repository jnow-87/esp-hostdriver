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
#include <opt.h>
#include "serial.h"


/* types */
typedef enum{
	RES_INVAL = -1,
	RES_OK = 0,
	RES_ERR,
	RES_BUSY,
} result_t;


/* local/static prototypes */
static void *read_thread(void *arg);
static void parse_line(char const *line);
static void recv(serial_t *dev, size_t n);


/* static variables */
static pthread_cond_t sig = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

static result_t result;
static char const *response = 0x0;


/* global functions */
serial_t *serial_init(char const *path){
	serial_t *dev;
	struct termios attr;
	void *rx_data;


	dev = malloc(sizeof(serial_t));

	if(dev == 0x0)
		goto err_0;

	dev->fd = open(path, O_RDWR);

	if(dev->fd == -1)
		goto err_0;

	rx_data = malloc(32);

	if(rx_data == 0x0)
		goto err_1;

	ringbuf_init(&dev->rx_buf, rx_data, 32);

	if(tcgetattr(dev->fd, &dev->term_attr) != 0)
		goto err_2;

	attr = dev->term_attr;

	attr.c_iflag = IXON | ICRNL | IGNCR;
	attr.c_oflag = ONLCR | OPOST;
	attr.c_lflag = IEXTEN | ECHOE | ECHOCTL | ECHOK | ECHOE | ISIG;
	attr.c_cflag = CBAUDEX | CLOCAL | HUPCL | CREAD;

	attr.c_cflag |= CS8;
	attr.c_cflag &= ~(PARENB | CSTOPB);
	cfsetspeed(&attr, B115200);

	if(tcsetattr(dev->fd, TCSANOW, &attr) != 0)
		goto err_3;

	if(pthread_create(&dev->read_thread_tid, 0, read_thread, dev) != 0)
		goto err_3;

	return dev;


err_3:
	tcsetattr(dev->fd, TCSANOW, &dev->term_attr);

err_2:
	free(rx_data);

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

size_t serial_read(serial_t *dev, void *data, size_t n){
	return ringbuf_read(&dev->rx_buf, data, n);
}

void serial_cmd_start(serial_t *dev, char const *resp){
	pthread_mutex_lock(&mtx);
	result = RES_INVAL;
	response = resp;
}

void serial_cmd_send(serial_t *dev, char const *s){
	if(s && *s)
		write(dev->fd, s, strlen(s));
}

void serial_cmd_send_char(serial_t *dev, char c){
	write(dev->fd, &c, 1);
}

int serial_cmd_wait(serial_t *dev){
	result_t r;


	write(dev->fd, "\n", 1);

	pthread_cond_wait(&sig, &mtx);
	r = result;
	pthread_mutex_unlock(&mtx);

	return (r == RES_OK ? 0 : -r);
}

int serial_cmd(serial_t *dev, char const *cmd, char const *resp){
	serial_cmd_start(dev, resp);
	serial_cmd_send(dev, cmd);

	return serial_cmd_wait(dev);
}


/* local functions */
static void *read_thread(void *arg){
	size_t i;
	char c;
	char line[64];
	serial_t *dev;


	dev = (serial_t*)arg;
	i = 0;

	while(1){
		if(read(dev->fd, &c, 1) != 1)
			break;

		if(opts.esp_debug)
			printf("%c", c);

		if(i + 1 < 64)
			line[i++] = c;

		if(c == ':' && strncmp(line, "+IPD", 4) == 0){
			line[i] = 0;
			while(line[i] != ',' && i > 0)	i--;

			i = atoi(line + i + 1);

			recv(dev, i);
		}
		else if(c == '\n'){
			line[i] = 0;
			i = 0;

			parse_line(line);
		}
	}

	return 0x0;
}

static void parse_line(char const *line){
	result_t r;


	r = RES_INVAL;

	pthread_mutex_lock(&mtx);

	if(response == 0x0 && strncmp(line, "OK", 2) == 0)					r = RES_OK;
	else if(response && strncmp(line, response, strlen(response)) == 0)	r = RES_OK;
	else if(strncmp(line, "ERROR", 5) == 0)								r = RES_ERR;
//			else if(strncmp(line, "busy", 4) == 0)								r = RES_BUSY;

	if(r != RES_INVAL){
		result = r;
		pthread_cond_signal(&sig);
	}

	pthread_mutex_unlock(&mtx);
}

static void recv(serial_t *dev, size_t n){
	char c;


	for(;n>0; n--){
		if(read(dev->fd, &c, 1) != 1)
			break;

		ringbuf_write(&dev->rx_buf, &c, 1);
	}
}
