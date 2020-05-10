#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <net.h>
#include <inet.h>
#include <netdev.h>
#include <sys/list.h>
#include "serial.h"


/* macros */
#define NLINKS	5


/* types */
typedef struct{
	serial_t *itf;
	socket_t *links[NLINKS];
	socket_t *tcp_server;
} esp_t;


/* local/static prototypes */
static int configure(netdev_t *dev, void *cfg);

static int connect(socket_t *sock);
static int listen(socket_t *sock, int backlog);
static int close(socket_t *sock);
static ssize_t send(socket_t *sock, void *data, size_t data_len);

static int send_cmd(serial_t *serial, char const *resp, char const *fmt, ...);
static void recv(netdev_t *dev, int link_id, void *data, size_t len, sock_addr_inet_t *remote);
static void accept(netdev_t *dev, int link_id, sock_addr_inet_t *remote);
static void closed(netdev_t *dev, int link_id);
static int get_link_id(esp_t *esp, socket_t *sock);


/* global functions */
int esp01_init(void){
	int r;
	serial_t *serial;
	netdev_t *dev;
	esp_t *esp;
	netdev_ops_t ops;


	serial = serial_init("/dev/ttyUSB0");

	if(serial == 0x0)
		goto err_0;

	esp = calloc(sizeof(esp_t), 1);

	if(esp == 0x0)
		goto err_1;

	esp->itf = serial;

	ops.configure = configure;
	ops.connect = connect;
	ops.listen = listen;
	ops.close = close;
	ops.send = send;

	dev = netdev_register(&ops, AF_INET, esp);

	if(dev == 0x0)
		goto err_2;

	r = 0;
	r |= serial_cmd(serial, "AT+RST", "ready");
	r |= serial_cmd(serial, "AT+CIPMUX=1", 0x0);

	if(r != 0)
		goto err_3;

	serial->recv = recv;
	serial->accept = accept;
	serial->closed = closed;
	serial->dev = dev;

	return 0;


err_3:
	netdev_release(dev);

err_2:
	free(esp);

err_1:
	serial_close(serial);

err_0:
	return -1;
}


/* local functions */
static int configure(netdev_t *dev, void *_cfg){
	int r;
	char addr[16];
	char *enc[] ={"0", "2", "3", "4"};
	inet_dev_cfg_t *cfg;
	serial_t *serial;


	cfg = (inet_dev_cfg_t*)_cfg;
	serial = ((esp_t*)dev->data)->itf;

	r = 0;

	r |= send_cmd(serial, 0x0, "AT+CWMODE_CUR=%s", (cfg->mode == INET_AP ? "2" : "1"));								// device mode
	r |= send_cmd(serial, 0x0, "AT+CIPDINFO=1");																	// source ip and port for incoming messages
	r |= send_cmd(serial, 0x0, "AT+CWDHCP_CUR=%s,%s", (cfg->mode == INET_AP ? "0" : "1"), (cfg->dhcp ? "1" : "0"));	// dhcp

	if(cfg->mode == INET_AP){
		// configure access point
		r |= send_cmd(serial, 0x0, "AT+CIPAP_CUR=\"%a\",\"%a\",\"%a\"", &cfg->ip, &cfg->gw, &cfg->netmask);
		r |= send_cmd(serial, 0x0, "AT+CWSAP_CUR=\"%s\",\"%s\",1,%s,4,0", cfg->ssid, cfg->password, enc[cfg->enc]);
	}
	else if(cfg->mode == INET_CLIENT){
		// connect to access point
		if(!cfg->dhcp)
			r |= send_cmd(serial, 0x0, "AT+CIPSTA_CUR=\"%a\",\"%a\",\"%s\"", &cfg->ip, &cfg->netmask, &cfg->gw);

		r |= send_cmd(serial, 0x0, "AT+CWJAP_CUR=\"%s\",\"%s\"", cfg->ssid, cfg->password);
	}
	else
		r = -1;

	return r;
}

static int connect(socket_t *sock){
	esp_t *esp;
	inet_data_t *remote;
	int link_id;


	esp = (esp_t*)sock->dev->data;
	remote = &((sock_addr_inet_t*)(&sock->addr))->data;

	link_id = get_link_id(esp, 0x0);

	if(link_id < 0)
		return -1;

	if(send_cmd(esp->itf, 0x0, "AT+CIPSTART=%d,\"%s\",\"%a\",%d", link_id, (sock->type == SOCK_DGRAM ? "UDP" : "TCP"), &remote->addr, remote->port) != 0)
		return -1;

	esp->links[link_id] = sock;

	return 0;
}

static int listen(socket_t *sock, int backlog){
	esp_t *esp;
	inet_data_t *remote;


	esp = (esp_t*)sock->dev->data;
	remote = &((sock_addr_inet_t*)(&sock->addr))->data;

	if(backlog != 0)
		return -1;

	if(esp->tcp_server != 0x0)
		return -1;

	if(send_cmd(esp->itf, 0x0, "AT+CIPSERVER=1,%d", remote->port) != 0)
		return -1;

	esp->tcp_server = sock;

	return 0;
}

static int close(socket_t *sock){
	int link_id;
	esp_t *esp;


	esp = (esp_t*)sock->dev->data;

	if(sock == esp->tcp_server){
		if(send_cmd(esp->itf, 0x0, "AT+CIPSERVER=0") != 0)
			return -1;

		esp->tcp_server = 0x0;

		return 0;
	}

	link_id = get_link_id(esp, sock);

	if(link_id < 0)
		return -1;

	if(send_cmd(esp->itf, 0x0, "AT+CIPCLOSE=%d", link_id) != 0)
		return -1;

	esp->links[link_id] = 0x0;

	return 0;
}

static ssize_t send(socket_t *sock, void *data, size_t data_len){
	int r;
	int link_id;
	esp_t *esp;
	serial_t *serial;
	inet_data_t *remote;


	if(data_len == 0)
		return 0;

	esp = (esp_t*)sock->dev->data;
	serial = esp->itf;
	remote = &((sock_addr_inet_t*)(&sock->addr))->data;

	r = 0;
	link_id = get_link_id(esp, sock);

	if(link_id < 0)
		return -1;

	if(sock->type == SOCK_DGRAM)	r |= send_cmd(serial, 0x0, "AT+CIPSEND=%d,%d,\"%a\",%d", link_id, data_len, &remote->addr, remote->port);
	else							r |= send_cmd(serial, 0x0, "AT+CIPSEND=%d,%d", link_id, data_len);

	if(r)
		return r;

	serial_cmd_start(serial, "SEND OK");
	serial_cmd_send(serial, data);
	r |= serial_cmd_wait(serial);

	return (r == 0 ? data_len : -1);
}

static int send_cmd(serial_t *serial, char const *resp, char const *fmt, ...){
	char c;
	char s[16];
	va_list lst;


	va_start(lst, fmt);

	serial_cmd_start(serial, resp);

	for(c=*fmt; c!=0; c=*(++fmt)){
		if(c == '%'){
			c = *(++fmt);

			if(c == 0)
				break;

			switch(c){
			case 'a':
				serial_cmd_send(serial, inet_ntoa(*va_arg(lst, inet_addr_t*), s, 15));
				break;

			case 's':
				serial_cmd_send(serial, va_arg(lst, char*));
				break;

			case 'd':
				snprintf(s, 16, "%d", va_arg(lst, int));
				serial_cmd_send(serial, s);
				break;

			default:
				serial_cmd_send_char(serial, c);
			}
		}
		else
			serial_cmd_send_char(serial, c);
	}

	va_end(lst);

	return serial_cmd_wait(serial);

}

static void recv(netdev_t *dev, int link_id, void *data, size_t len, sock_addr_inet_t *remote){
	esp_t *esp;
	socket_t *sock;
	datagram_t *dgram;


	esp = (esp_t*)dev->data;
	sock = esp->links[link_id];

	if(sock->type == SOCK_DGRAM){
		dgram = malloc(sizeof(datagram_t) + len + sizeof(sock_addr_inet_t) - sizeof(sock_addr_t));

		dgram->len = len;
		dgram->data = (uint8_t*)(&dgram->addr + sizeof(sock_addr_inet_t));
		memcpy(dgram->data, data, len);
		memcpy(&dgram->addr, remote, sizeof(sock_addr_inet_t));

		list_add_tail(sock->dgrams, dgram);
	}
	else
		ringbuf_write(&sock->stream, data, len);
}

static void accept(netdev_t *dev, int link_id, sock_addr_inet_t *remote){
	char ip[16];
	socket_t *sock;
	esp_t *esp;


	esp = (esp_t*)dev->data;

	if(esp->tcp_server == 0x0)
		return;

	sock = netdev_sock_alloc(SOCK_STREAM, sizeof(sock_addr_inet_t));

	if(sock == 0x0)
		return;

	memcpy(&sock->addr, remote, sizeof(sock_addr_inet_t));

	esp->links[link_id] = sock;
	list_add_tail(esp->tcp_server->clients, sock);
}

static void closed(netdev_t *dev, int link_id){
	esp_t *esp;
	socket_t *sock;


	esp = (esp_t*)dev->data;
	sock = esp->links[link_id];

	esp->links[link_id] = 0x0;
	netdev_sock_disconnect(sock);
}

static int get_link_id(esp_t *esp, socket_t *sock){
	unsigned int i;


	for(i=0; i<NLINKS; i++){
		if(esp->links[i] == sock)
			return i;
	}

	return -1;
}
