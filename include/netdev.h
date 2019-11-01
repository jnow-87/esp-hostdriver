#ifndef NETDEV_H
#define NETDEV_H


#include <stddef.h>
#include <types.h>
#include <net.h>


/* incomplete types */
struct netdev_t;


/* types */
typedef struct{
	int (*configure)(struct netdev_t *dev, void *cfg);

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

	net_family_t domain;

	netdev_ops_t ops;
	void *data;
	uint8_t cfg[];
} netdev_t;


/* prototypes */
int netdev_init(void);

netdev_t *netdev_register(netdev_ops_t *ops, net_family_t domain, void *data);
int netdev_release(netdev_t *dev);

int bos_net_configure(void *cfg, size_t cfg_size);

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
