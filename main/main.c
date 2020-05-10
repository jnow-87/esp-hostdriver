#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/readline.h>
#include <pthread.h>
#include <netdev.h>
#include <inet.h>
#include <esp01.h>
#include "opt.h"

/* types */
typedef struct{
	int echo;
	int sock;
} recv_thread_arg_t;


/* local/static prototypes */
static void server(int sock);
static void client(int sock);
static void *recv_thread(void *arg);


/* global functions */
int main(int argc, char **argv){
	int r;
	int sock;
	inet_dev_cfg_t cfg;


	parse_opt(argc, argv);

	/* init devices */
	r = 0;
	r |= netdev_init();
	r |= esp01_init();

	if(r){
		printf("init failed\n");
		return 1;
	}

	/* configure network connection */
	cfg.mode = opts.dev_mode;
	cfg.ssid = opts.ssid;
	cfg.password = opts.password;

	cfg.hostname = "not-supported";
	cfg.dhcp = true;
	cfg.enc = ENC_WPA2_PSK;

	cfg.ip = inet_addr("192.168.0.1");
	cfg.gw = inet_addr("192.168.0.1");
	cfg.netmask = inet_addr("255.255.255.0");

	printf("configure esp01 as %s with ssid \"%s\", password \"%s\"\n", (opts.dev_mode == INET_AP ? "access point" : "client"), opts.ssid, opts.password);

	if(bos_net_configure(&cfg, sizeof(inet_dev_cfg_t)) != 0){
		printf("device configuration failed\n");
		return 1;
	}

	/* init socket */
	sock = bos_socket(AF_INET, opts.type);

	printf("socket: %d\n", sock);

	/* server socket test */
	if(opts.ip == 0)	server(sock);
	else				client(sock);

	return 0;
}


/* local functions */
static void server(int sock){
	int r;
	int volatile client;
	size_t addr_len;
	char ip[16];
	sock_addr_inet_t addr;
	pthread_t tid;
	recv_thread_arg_t arg;


	r = 0;

	addr.domain = AF_INET;
	addr.data.addr = INET_ADDR_ANY;
	addr.data.port = opts.port;

	printf("start server on port %d\n", opts.port);
	printf("bind: %d\n", (r |= bos_bind(sock, (sock_addr_t*)&addr, sizeof(sock_addr_inet_t))));
	printf("listen: %d\n", (r |= bos_listen(sock, 0)));

	if(r)
		return;

	while(1){
		addr_len = sizeof(sock_addr_inet_t);
		memset(&addr, 0x0, addr_len);

		client = bos_accept(sock, (sock_addr_t*)&addr, &addr_len);

		if(client <= 0){
			usleep(500);
			continue;
		}

		printf("client socket %d %s on port %d\n", client, inet_ntoa(addr.data.addr, ip, 16), addr.data.port);

		arg.echo = 1;
		arg.sock = client;

		pthread_create(&tid, 0, recv_thread, &arg);
	}
}

static void client(int sock){
	int r;
	size_t sock_len;
	char *line;
	char ip[16];
	sock_addr_inet_t addr;
	pthread_t tid;
	recv_thread_arg_t arg;


	r = 0;

	addr.domain = AF_INET;
	addr.data.addr = opts.ip;
	addr.data.port = opts.port;

	printf("connect to %s on port %d\n", inet_ntoa(addr.data.addr, ip, 16), addr.data.port);
	printf("connect: %d\n", (r |= bos_connect(sock, (sock_addr_t*)&addr, sizeof(sock_addr_inet_t))));

	if(r)
		return;

	arg.echo = 0;
	arg.sock = sock;

	pthread_create(&tid, 0, recv_thread, &arg);

	while(1){
		line = readline("$ ");

		if(strcmp(line, "q") == 0)
			break;

		printf("send: %d\n", bos_send(sock, line, strlen(line)));

		free(line);
	}

	printf("close: %d\n", bos_close(sock));
	pthread_cancel(tid);
}

static void *recv_thread(void *_arg){
	size_t n;
	int sock;
	char s[16],
		 ip[16];
	size_t addr_len;
	sock_addr_inet_t addr;
	recv_thread_arg_t *arg;


	arg = (recv_thread_arg_t*)_arg;
	sock = arg->sock;

	addr_len = sizeof(sock_addr_inet_t);

	printf("start recv thread for socket %d\n", sock);

	while(1){
		n = bos_recvfrom(sock, s, 16, (sock_addr_t*)&addr, &addr_len);

		if((ssize_t)n < 0)
			break;

		if(n == 0){
			usleep(500000);
			continue;
		}

		if(s[n - 1] == '\n')
			n--;

		s[n] = 0;

		printf("recv: \"%s\" from %s:%d\n", s, inet_ntoa(addr.data.addr, ip, 15), addr.data.port);
		snprintf(s, 16, "ack %d", strlen(s));

		if(arg->echo){
			if(opts.type == SOCK_DGRAM)		bos_sendto(sock, s, strlen(s), (sock_addr_t*)&addr, addr_len);
			else							bos_send(sock, s, strlen(s));
		}
	}

	printf("target socket %d closed, exit recv thread\n", sock);
	bos_close(sock);
}
