#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/readline.h>
#include <pthread.h>
#include <netdev.h>
#include <inet.h>
#include <esp01.h>
#include "opt.h"


/* local/static prototypes */
static void *recv_thread(void *arg);


/* global functions */
int main(int argc, char **argv){
	int r;
	char *line;
	int sock;
	inet_dev_cfg_t cfg;
	sock_addr_inet_t addr;
	pthread_t tid;


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
	cfg.mode = opts.mode;
	cfg.ssid = opts.ssid;
	cfg.password = opts.password;

	cfg.hostname = "not-supported";
	cfg.dhcp = true;
	cfg.enc = ENC_WPA2_PSK;

	cfg.ip = inet_addr("192.168.0.1");
	cfg.gw = inet_addr("192.168.0.1");
	cfg.netmask = inet_addr("255.255.255.0");

	printf("configure esp01 as %s with ssid \"%s\", password \"%s\"\n", (opts.mode == INET_AP ? "access point" : "client"), opts.ssid, opts.password);

	if(bos_net_configure(&cfg, sizeof(inet_dev_cfg_t)) != 0){
		printf("device configuration failed\n");
		return 1;
	}

	/* socket test */
	addr.domain = AF_INET;
	addr.data.addr = inet_addr("192.168.0.24");
	addr.data.port = 1234;

	sock = bos_socket(AF_INET, opts.type);

	printf("socket: %d\n", sock);
	printf("connect: %d\n", bos_connect(sock, (sock_addr_t*)&addr, sizeof(sock_addr_inet_t)));

	pthread_create(&tid, 0, recv_thread, &sock);

	while(1){
		line = readline("$ ");

		if(strcmp(line, "q") == 0)
			break;

		printf("send: %d\n", bos_send(sock, line, strlen(line)));

		free(line);
	}

	printf("close: %d\n", bos_close(sock));

	pthread_cancel(tid);

	return 0;
}


/* local functions */
static void *recv_thread(void *arg){
	int *sock;
	int c;


	sock = (int*)arg;

	while(1){
		if(bos_recv(*sock, &c, 1) != 1){
			usleep(500000);
			continue;
		}

		write(1, &c, 1);
	}
}
