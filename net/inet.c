#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inet.h>


/* global functions */
inet_addr_t inet_addr(char *_addr){
	int8_t i;
	char c;
	char *tk;
	char addr[strlen(_addr)];
	inet_addr_t iaddr;


	strcpy(addr, _addr);

	iaddr = 0;
	tk = strtok(addr, ".");

	while(tk != 0x0){
		iaddr = (iaddr << 8) | atoi(tk);
		tk = strtok(0x0, ".");
	}

	return iaddr;
}

char *inet_ntoa(inet_addr_t addr, char *s, size_t len){
	#define BITS8(v, shift)	((v & (0xff << shift)) >> shift)

	snprintf(s, len, "%d.%d.%d.%d",
		BITS8(addr, 24), BITS8(addr, 16), BITS8(addr, 8), BITS8(addr, 0));

	return s;
}

int inet_match_addr(netdev_t *dev, sock_addr_t *addr, size_t addr_len){
	inet_dev_cfg_t *cfg;
	sock_addr_inet_t *inet_addr;


	cfg = (inet_dev_cfg_t*)dev->cfg;
	inet_addr = (sock_addr_inet_t*)addr;

	if(addr_len != sizeof(sock_addr_inet_t))
		return 0;

	if((cfg->ip & cfg->netmask) == (inet_addr->data.addr & cfg->netmask))
		return 1;

	return 0;
}
