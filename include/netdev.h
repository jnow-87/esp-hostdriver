#ifndef NETDEV_H
#define NETDEV_H


#include <stddef.h>
#include <types.h>
#include <net.h>
#include <inet.h>


/* incomplete types */
struct netdev_t;


/* types */
typedef enum{
	NETDEV_AP = 0x1,
	NETDEV_CLIENT = 0x2,
} netdev_mode_t;

typedef enum{
	ENC_OPEN = 0,
	ENC_WPA_PSK,
	ENC_WPA2_PSK,
	ENC_WPA_WPA2_PSK,
} netdev_enc_mode_t;

typedef struct{
	char *hostname;

	bool dhcp;
	inet_addr_t ip,
				gw,
				netmask;

	netdev_enc_mode_t enc;
	char *password;
} netdev_cfg_t;

typedef struct{
	int (*configure)(struct netdev_t *dev, netdev_cfg_t *cfg);

	int (*connect)(struct netdev_t *dev, sock_addr_t *addr);
	int (*listen)(struct netdev_t *dev, int backlog);
	int (*accept)(struct netdev_t *dev, sock_addr_t *addr);
	int (*close)(struct netdev_t *dev);

	int (*send)(struct netdev_t *dev, void *data, size_t data_len, sock_addr_t *addr);
	int (*recv)(struct netdev_t *dev, void *data, size_t data_len, sock_addr_t *addr);
} netdev_ops_t;

typedef struct netdev_t{
	struct netdev_t *prev,
					*next;

	netdev_ops_t ops;
	netdev_cfg_t cfg;

	void *data;
} netdev_t;


/* prototypes */
int netdev_init(void);

netdev_t *netdev_register(netdev_ops_t *ops, void *data);
int netdev_release(netdev_t *dev);

int bos_socket(net_family_t domain, sock_type_t type);
int bos_connect(int fd, sock_addr_t *addr, size_t addr_len);
int bos_bind(int fd, sock_addr_t *addr, size_t addr_len);
int bos_listen(int fd, int backlog);
int bos_accept(int fd, sock_addr_t *addr, size_t *addr_len);
int bos_send(int fd, void *data, size_t data_len);
int bos_sendto(int fd, void *data, size_t data_len, sock_addr_t *addr, size_t addr_len);
int bos_recv(int fd, void *data, size_t data_len);
int bos_recvfrom(int fd, void *data, size_t data_len, sock_addr_t *addr, size_t *addr_len);
int bos_ioctl(int fd, int cmd, void *data, size_t data_len);


#endif // NETDEV_H
