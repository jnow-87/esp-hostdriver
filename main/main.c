#include <stdio.h>
#include <netdev.h>
#include <inet.h>
#include <esp01.h>


/* global functions */
int main(int argc, char **argv){
	int r;
	inet_dev_cfg_t cfg;


	r = 0;
	r |= netdev_init();
	r |= esp01_init();

	if(r){
		printf("init failed\n");
		return 1;
	}

	/* init device */
	cfg.mode = INET_AP;
	cfg.hostname = "mainart";
	cfg.dhcp = true;
	cfg.password = "Wesd2xc3X";
	cfg.ip = inet_addr("192.168.0.1");
	cfg.gw = inet_addr("192.168.0.1");
	cfg.netmask = inet_addr("255.255.255.0");
	cfg.enc = ENC_WPA2_PSK;

	if(bos_net_configure(&cfg, sizeof(inet_dev_cfg_t)) != 0){
		printf("device configuration failed\n");
		return 1;
	}

	return 0;
}
