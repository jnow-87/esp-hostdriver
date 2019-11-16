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
static void parse_line(serial_t *dev, char *line, size_t len);
static void recv(serial_t *dev, char *line, size_t len);
static void accept(serial_t *dev, char *line, size_t len);
static void closed(serial_t *dev, char *line, size_t len);


/* static variables */
static pthread_cond_t sig = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

static result_t result;
static char const *response = 0x0;


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
		goto err_2;

	if(pthread_create(&dev->read_thread_tid, 0, read_thread, dev) != 0)
		goto err_2;

	return dev;


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
			recv(dev, line, i);
			i = 0;
		}
		else if(c == '\n'){
			line[i] = 0;
			parse_line(dev, line, i);
			i = 0;
		}
	}

	return 0x0;
}

static void parse_line(serial_t *dev, char *line, size_t len){
	result_t r;


	/* check for incoming connection */
	if(strncmp(line + len - 9, ",CONNECT", 8) == 0){
		accept(dev, line, len);
		return;
	}

	if(strncmp(line + len - 8, ",CLOSED", 7) == 0){
		closed(dev, line, len);
		return;
	}
	r = RES_INVAL;

	/* check for command response */
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

static void recv(serial_t *dev, char *line, size_t len){
	size_t n,
		   i;
	int link_id;
	sock_addr_inet_t remote;


	remote.domain = AF_INET;

	while(line[len] != ',' && len > 0)	len--;
	remote.data.port = atoi(line + len + 1);
	line[len] = 0;

	while(line[len] != ',' && len > 0)	len--;
	remote.data.addr = inet_addr(line + len + 1);
	line[len] = 0;

	while(line[len] != ',' && len > 0)	len--;
	n = atoi(line + len + 1);
	line[len] = 0;

	while(line[len] != ',' && len > 0)	len--;
	link_id = atoi(line + len + 1);

	char s[n + 1];

	for(i=0; i<n; i++){
		if(read(dev->fd, s + i, 1) != 1)
			break;
	}

	s[i] = 0;

	dev->recv(dev->dev, link_id, s, i, &remote);
}

static void accept(serial_t *dev, char *line, size_t len){
	size_t i;
	char link_id[5];
	sock_addr_inet_t remote;


	i = 0;

	while(line[i] != ',' && i < len)	i++;
	line[i] = 0;
	strncpy(link_id, line, 5);

	write(dev->fd, "AT+CIPSTATUS\n", 13);

	i = 0;

	while(1){
		if(read(dev->fd, line + i, 1) != 1)
			break;

		if(opts.esp_debug){
			printf("%c", line[i]);
			fflush(stdout);
		}

		if(line[i] == '\n'){
			line[i] = 0;

			if(strcmp(line, "OK") == 0 || strcmp(line, "ERROR") == 0)
				return;

			if(strncmp(line, "+CIPSTATUS:", 11) == 0 && strncmp(line + 11, link_id, strlen(link_id)) == 0){
				i = 20;
				while(line[i] != '"')	i++;
				line[i] = 0;

				remote.domain = AF_INET;
				remote.data.addr = inet_addr(line + 20);

				line += i + 2;
				i = 0;
				while(line[i] != ',')	i++;
				line[i] = 0;

				remote.data.port = atoi(line);
				dev->accept(dev->dev, atoi(link_id), &remote);
			}

			i = 0;
		}
		else
			i++;
	}
}

static void closed(serial_t *dev, char *line, size_t len){
	size_t i;
	int link_id;


	i = 0;
	while(line[i] != ',' && i < len)	i++;
	line[i] = 0;
	link_id = atoi(line);

	dev->closed(dev->dev, link_id);
}
