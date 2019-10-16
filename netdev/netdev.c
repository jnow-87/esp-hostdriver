#include <inet.h>
#include <netdev.h>


/* global functions */
int netdev_init(void){
	return 0;
}

netdev_t *netdev_register(netdev_ops_t *ops, void *data){
	return 0x0;
}

int netdev_release(netdev_t *dev){
	return 0;
}

int bos_socket(net_family_t domain, sock_type_t type){
	return 0;
}

int bos_connect(int fd, sock_addr_t *addr, size_t addr_len){
	return 0;
}

int bos_bind(int fd, sock_addr_t *addr, size_t addr_len){
	return 0;
}

int bos_listen(int fd, int backlog){
	return 0;
}

int bos_accept(int fd, sock_addr_t *addr, size_t *addr_len){
	return 0;
}

int bos_send(int fd, void *data, size_t data_len){
	return 0;
}

int bos_sendto(int fd, void *data, size_t data_len, sock_addr_t *addr, size_t addr_len){
	return 0;
}

int bos_recv(int fd, void *data, size_t data_len){
	return 0;
}

int bos_recvfrom(int fd, void *data, size_t data_len, sock_addr_t *addr, size_t *addr_len){
	return 0;
}

int bos_ioctl(int fd, int cmd, void *data, size_t data_len){
	return 0;
}
