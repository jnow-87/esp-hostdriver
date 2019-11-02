#include <stdarg.h>
#include <stdio.h>
#include <net.h>
#include <inet.h>
#include <netdev.h>
#include "serial.h"


/* local/static prototypes */
static int configure(struct netdev_t *dev, void *cfg);

static int connect(struct netdev_t *dev, socket_t *sock);
static int listen(struct netdev_t *dev, int backlog);
static int accept(struct netdev_t *dev, socket_t *sock);
static int close(struct netdev_t *dev);

static ssize_t send(struct netdev_t *dev, void *data, size_t data_len, socket_t *sock);
static ssize_t recv(struct netdev_t *dev, void *data, size_t data_len, socket_t *sock);

static int send_cmd(serial_t *esp, char const *resp, char const *fmt, ...);


/* global functions */
int esp01_init(void){
	int r;
	serial_t *esp;
	netdev_t *dev;
	netdev_ops_t ops;


	esp = serial_init("/dev/ttyUSB0");

	if(esp == 0x0)
		goto err_0;

	ops.configure = configure;
	ops.connect = connect;
	ops.listen = listen;
	ops.accept = accept;
	ops.close = close;
	ops.send = send;
	ops.recv = recv;

	dev = netdev_register(&ops, AF_INET, esp);

	if(dev == 0x0)
		goto err_1;

	r = 0;
	r |= serial_cmd(esp, "AT+RST", "ready");
	r |= serial_cmd(esp, "AT+CIPMUX=1", 0x0);

	if(r != 0)
		goto err_1;

	return 0;


err_1:
	serial_close(esp);

err_0:
	return -1;
}


/* local functions */
/**
 * TODO
 * 	consider if mixed client and ap mode shall be supported
 * 	handle link-ids
 */
static int configure(struct netdev_t *dev, void *_cfg){
	int r;
	char addr[16];
	char *enc[] ={"0", "2", "3", "4"};
	inet_dev_cfg_t *cfg;
	serial_t *esp;


	cfg = (inet_dev_cfg_t*)_cfg;
	esp = (serial_t*)dev->data;

	r = 0;

	r |= send_cmd(esp, 0x0, "AT+CWMODE_CUR=%s", (cfg->mode == INET_AP ? "2" : "1"));								// device mode
	r |= send_cmd(esp, 0x0, "AT+CWDHCP_CUR=%s,%s", (cfg->mode == INET_AP ? "0" : "1"), (cfg->dhcp ? "1" : "0"));	// dhcp

	if(cfg->mode == INET_AP){
		// configure access point
		r |= send_cmd(esp, 0x0, "AT+CIPAP_CUR=\"%a\",\"%a\",\"%a\"", &cfg->ip, &cfg->gw, &cfg->netmask);
		r |= send_cmd(esp, 0x0, "AT+CWSAP_CUR=\"%s\",\"%s\",1,%s,4,0", cfg->ssid, cfg->password, enc[cfg->enc]);
	}

	if(cfg->mode == INET_CLIENT){
		// connect to access point
		if(!cfg->dhcp)
			r |= send_cmd(esp, 0x0, "AT+CIPSTA_CUR=\"%a\",\"%a\",\"%s\"", &cfg->ip, &cfg->netmask, &cfg->gw);

		r |= send_cmd(esp, 0x0, "AT+CWJAP_CUR=\"%s\",\"%s\"", cfg->ssid, cfg->password);
	}

	return r;
}

static int connect(struct netdev_t *dev, socket_t *sock){
	serial_t *esp;
	inet_data_t *remote;
	char port[16];


	esp = (serial_t*)dev->data;
	remote = &((sock_addr_inet_t*)(&sock->addr))->data;

	snprintf(port, 16, "%d", remote->port);
	return send_cmd(esp, 0x0, "AT+CIPSTART=0,\"%s\",\"%a\",%s", (sock->type == SOCK_DGRAM ? "UDP" : "TCP"), &remote->addr, port);
}

static int listen(struct netdev_t *dev, int backlog){
	// TODO
	return 0;
}

static int accept(struct netdev_t *dev, socket_t *sock){
	// TODO
	return 0;
}

static int close(struct netdev_t *dev){
	serial_t *esp;


	esp = (serial_t*)dev->data;
	return send_cmd(esp, 0x0, "AT+CIPCLOSE=0");
}

static ssize_t send(struct netdev_t *dev, void *data, size_t data_len, socket_t *sock){
	int r;
	char len[6],
		 port[16];
	serial_t *esp;
	inet_data_t *remote;


	if(data_len == 0)
		return 0;

	esp = (serial_t*)dev->data;
	remote = &((sock_addr_inet_t*)(&sock->addr))->data;

	snprintf(len, 6, "%d", data_len);

	r = 0;

	if(sock->type == SOCK_DGRAM){
		snprintf(port, 16, "%d", remote->port);
		r |= send_cmd(esp, 0x0, "AT+CIPSEND=0,%s,\"%a\",%s", len, &remote->addr, port);
	}
	else
		r |= send_cmd(esp, 0x0, "AT+CIPSEND=0,%s", len);

	if(r)
		return r;

	serial_cmd_start(esp, "SEND OK");
	serial_cmd_send(esp, data);
	r |= serial_cmd_wait(esp);

	return (r == 0 ? data_len : -1);
}

static ssize_t recv(struct netdev_t *dev, void *data, size_t data_len, socket_t *sock){
	return serial_read((serial_t*)dev->data, data, data_len);
}

static int send_cmd(serial_t *esp, char const *resp, char const *fmt, ...){
	char c;
	char *s;
	char addr_s[16];
	inet_addr_t *addr;
	va_list lst;


	va_start(lst, fmt);

	serial_cmd_start(esp, resp);

	for(c=*fmt; c!=0; c=*(++fmt)){
		if(c == '%'){
			c = *(++fmt);

			if(c == 0)
				break;

			switch(c){
			case 'a':
				addr = va_arg(lst, void*);
				serial_cmd_send(esp, inet_ntoa(*addr, addr_s, 15));
				break;

			case 's':
				s = va_arg(lst, char*);
				serial_cmd_send(esp, s);
				break;

			default:
				serial_cmd_send_char(esp, c);
			}
		}
		else
			serial_cmd_send_char(esp, c);
	}

	va_end(lst);

	return serial_cmd_wait(esp);

}
