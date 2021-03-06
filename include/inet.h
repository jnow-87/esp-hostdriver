#ifndef INET_H
#define INET_H


#include <net.h>
#include <netdev.h>


/* macros */
#define INET_ADDR_ANY	0


/* types */
typedef uint32_t inet_addr_t;

typedef enum{
	INET_AP = 1,
	INET_CLIENT,
} inet_dev_mode_t;

typedef enum{
	ENC_OPEN = 0,
	ENC_WPA_PSK,
	ENC_WPA2_PSK,
	ENC_WPA_WPA2_PSK,
} inet_enc_t;

typedef struct{
	inet_dev_mode_t mode;

	char *ssid,
		 *hostname;

	bool dhcp;
	inet_addr_t ip,
				gw,
				netmask;

	inet_enc_t enc;
	char *password;
} inet_dev_cfg_t;

typedef struct{
	inet_addr_t addr;
	uint16_t port;
} inet_data_t;

typedef struct{
	net_family_t domain;
	inet_data_t data;
} sock_addr_inet_t;


/* prototypes */
inet_addr_t inet_addr(char *addr);
char *inet_ntoa(inet_addr_t addr, char *s, size_t len);
int inet_match_addr(netdev_t *dev, sock_addr_t *addr, size_t addr_len);


#endif // INET_H
