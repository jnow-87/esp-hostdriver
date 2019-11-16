#ifndef NETDEV_H
#define NETDEV_H


#include <stddef.h>
#include <types.h>
#include <net.h>
#include <sys/ringbuf.h>


/* incomplete types */
struct netdev_t;


/* types */
typedef struct datagram_t{
	struct datagram_t *prev,
					  *next;

	size_t len,
		   idx;
	uint8_t *data;

	sock_addr_t addr;		// NOTE addr has to be the last member of this
							// struct since it has a flexible array member
} datagram_t;

typedef struct socket_t{
	struct socket_t *prev,
					*next;

	struct netdev_t *dev;

	ringbuf_t stream;
	datagram_t *dgrams;

	// TODO add mutex
	struct socket_t *clients;

	sock_type_t type;
	sock_addr_t addr;		// NOTE addr has to be the last member of this
							// struct since it has a flexible array member
} socket_t;

typedef struct{
	int (*configure)(struct netdev_t *dev, void *cfg);

	int (*connect)(socket_t *sock);
	int (*listen)(socket_t *sock, int backlog);
	int (*close)(socket_t *sock);

	ssize_t (*send)(socket_t *sock, void *data, size_t data_len);
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

socket_t *netdev_sock_alloc(sock_type_t type, size_t addr_len);
void netdev_sock_free(socket_t *sock);
void netdev_sock_disconnect(socket_t *sock);

int bos_net_configure(void *cfg, size_t cfg_size);

int bos_socket(net_family_t domain, sock_type_t type);
int bos_connect(int fd, sock_addr_t *addr, size_t addr_len);
int bos_bind(int fd, sock_addr_t *addr, size_t addr_len);
int bos_listen(int fd, int backlog);
int bos_accept(int fd, sock_addr_t *addr, size_t *addr_len);
ssize_t bos_send(int fd, void *data, size_t data_len);
ssize_t bos_sendto(int fd, void *data, size_t data_len, sock_addr_t *addr, size_t addr_len);
ssize_t bos_recv(int fd, void *data, size_t data_len);
ssize_t bos_recvfrom(int fd, void *data, size_t data_len, sock_addr_t *addr, size_t *addr_len);
int bos_ioctl(int fd, int cmd, void *data, size_t data_len);

int bos_close(int fd);


#endif // NETDEV_H
