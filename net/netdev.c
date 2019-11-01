#include <stdlib.h>
#include <stddef.h>
#include <inet.h>
#include <netdev.h>
#include <sys/list.h>
#include <bos/fs.h>


/**
 * TODO
 * 	handle changing target for sendto, recvfrom
 * 	handle esp commands that return something different than OK, ERROR, busy, such as AT+RST
 */

/* types */
typedef struct{
	size_t cfg_size,
		   addr_len;

	int (*match_addr)(netdev_t *dev, sock_addr_t *addr, size_t addr_len);
} domain_cfg_t;

typedef struct{
	net_family_t domain;
	sock_type_t type;

	netdev_t *dev;
} socket_t;


/* local/static prototypes */
static int open(struct fs_node_t *start, char const *path, f_mode_t mode, struct process_t *this_p);
static int close(struct fs_filed_t *fd, struct process_t *this_p);
static size_t read(struct fs_filed_t *fd, void *buf, size_t n);
static size_t write(struct fs_filed_t *fd, void *buf, size_t n);

static int assign_netdev(socket_t *sock, sock_addr_t *addr, size_t addr_len);

static char *itoa(int v, unsigned int base, char *s, size_t len);


/* static variables */
static netdev_t *dev_lst = 0x0;
static int netfs_id = -1;
static fs_node_t *net_root = 0x0;

static domain_cfg_t const domain_cfg[] = {
	sizeof(inet_dev_cfg_t), sizeof(sock_addr_inet_t), inet_match_addr,	// AF_INET
};


/* global functions */
int netdev_init(void){
	fs_ops_t ops;


	ops.open = open;
	ops.close = close;
	ops.read = read;
	ops.write = write;
	ops.ioctl = 0x0;
	ops.fcntl = 0x0;
	ops.node_rm = 0x0;

	netfs_id = fs_register(&ops);

	if(netfs_id == -1)
		return -1;

	net_root = fs_node_create(0x0, "socket", 6, FT_DIR, 0x0, netfs_id);

	if(net_root == 0x0)
		return -1;

	return 0;
}

netdev_t *netdev_register(netdev_ops_t *ops, net_family_t domain, void *data){
	netdev_t *dev;
	size_t cfg_size;


	cfg_size = domain_cfg[domain].cfg_size;

	dev = malloc(sizeof(netdev_t) + cfg_size);

	if(dev == 0x0)
		goto err;

	dev->domain = domain;
	dev->ops = *ops;
	dev->data = data;
	memset(&dev->cfg, 0x0, cfg_size);

	list_add_tail(dev_lst, dev);

	return dev;


err:
	return 0x0;
}

int netdev_release(netdev_t *dev){
	list_rm(dev_lst, dev);
	free(dev);

	return 0;
}

int bos_net_configure(void *cfg, size_t cfg_size){
	if(cfg_size != domain_cfg[dev_lst->domain].cfg_size)
		return -1;

	return dev_lst->ops.configure(dev_lst, cfg);
}

int bos_socket(net_family_t domain, sock_type_t type){
	static int id = 0;
	char name[8];
	fs_filed_t *fd;
	fs_node_t *node;
	socket_t *sock;


	id++;

	itoa(id, 10, name, 8);

	sock = malloc(sizeof(socket_t));

	sock->domain = domain;
	sock->type = type;
	sock->dev = 0x0;

	node = fs_node_create(net_root, name, strlen(name), FT_REG, sock, netfs_id);
	fd = fs_fd_alloc(node, O_RDWR, 0x0);

	return fd->id;
}

int bos_connect(int fd_id, sock_addr_t *addr, size_t addr_len){
	int r;
	fs_filed_t *fd;
	socket_t *sock;
	netdev_t *dev;


	r = -1;

	fd = fs_fd_acquire(fd_id);
	sock = fd->node->data;
	dev = sock->dev;

	if(sock->domain != addr->domain)
		goto end;

	if(dev == 0x0){
		if(assign_netdev(sock, addr, addr_len) != 0)
			goto end;
	}

	if(dev->ops.connect)
		r = dev->ops.connect(dev, addr);

end:
	fs_fd_release(fd);

	return r;
}

int bos_bind(int fd_id, sock_addr_t *addr, size_t addr_len){
	int r;
	fs_filed_t *fd;
	socket_t *sock;


	r = -1;

	fd = fs_fd_acquire(fd_id);
	sock = fd->node->data;

	if(sock->domain != addr->domain)
		goto end;

	r = assign_netdev(sock, addr, addr_len);

end:
	fs_fd_release(fd);

	return r;
}

int bos_listen(int fd_id, int backlog){
	int r;
	fs_filed_t *fd;
	socket_t *sock;
	netdev_t *dev;


	r = -1;

	fd = fs_fd_acquire(fd_id);
	sock = fd->node->data;
	dev = sock->dev;

	if(dev == 0x0)
		goto end;

	if(dev->ops.listen)
		r = dev->ops.listen(dev, backlog);

end:
	fs_fd_release(fd);

	return r;
}

int bos_accept(int fd_id, sock_addr_t *addr, size_t *addr_len){
	int r;
	fs_filed_t *fd;
	socket_t *sock;
	netdev_t *dev;


	r = -1;

	fd = fs_fd_acquire(fd_id);
	sock = fd->node->data;
	dev = sock->dev;

	if(dev == 0x0)
		goto end;

	if(*addr_len != domain_cfg[sock->domain].addr_len)
		goto end;

	if(dev->ops.accept)
		r = dev->ops.accept(dev, addr);

end:
	fs_fd_release(fd);

	return r;
}

int bos_send(int fd_id, void *data, size_t data_len){
	return bos_sendto(fd_id, data, data_len, 0x0, 0);
}

int bos_sendto(int fd_id, void *data, size_t data_len, sock_addr_t *addr, size_t addr_len){
	int r;
	fs_filed_t *fd;
	socket_t *sock;
	netdev_t *dev;


	r = -1;

	fd = fs_fd_acquire(fd_id);
	sock = fd->node->data;
	dev = sock->dev;

	if(dev == 0x0)
		goto end;

	if(addr && addr_len != domain_cfg[sock->domain].addr_len)
		goto end;

	if(dev->ops.send)
		r = dev->ops.send(dev, data, data_len, addr);

end:
	fs_fd_release(fd);

	return r;
}

int bos_recv(int fd_id, void *data, size_t data_len){
	return bos_recvfrom(fd_id, data, data_len, 0x0, 0);
}

int bos_recvfrom(int fd_id, void *data, size_t data_len, sock_addr_t *addr, size_t *addr_len){
	int r;
	fs_filed_t *fd;
	socket_t *sock;
	netdev_t *dev;


	r = -1;

	fd = fs_fd_acquire(fd_id);
	sock = fd->node->data;
	dev = sock->dev;

	if(dev == 0x0)
		goto end;

	if(addr && *addr_len != domain_cfg[sock->domain].addr_len)
		goto end;

	if(dev->ops.recv)
		r = dev->ops.recv(dev, data, data_len, addr);

end:
	fs_fd_release(fd);

	return r;
}


/* local functions */
static int open(struct fs_node_t *start, char const *path, f_mode_t mode, struct process_t *this_p){
	return 0;
}

static int close(struct fs_filed_t *fd, struct process_t *this_p){
	socket_t *sock;


	sock = fd->node->data;

	if(sock)
		sock->dev->ops.close(sock->dev);

	fs_fd_free(fd);
	return 0;
}

static size_t read(struct fs_filed_t *fd, void *buf, size_t n){
	return bos_recv(fd->id, buf, n);
}

static size_t write(struct fs_filed_t *fd, void *buf, size_t n){
	return bos_send(fd->id, buf, n);
}

static int assign_netdev(socket_t *sock, sock_addr_t *addr, size_t addr_len){
	netdev_t *dev;


	if(addr_len != domain_cfg[sock->domain].addr_len)
		return -1;

	list_for_each(dev_lst, dev){
		if(dev->domain != sock->domain)
			continue;

		if(domain_cfg[sock->domain].match_addr(dev, addr, addr_len)){
			sock->dev = dev;
			return 0;
		}
	}

	return -1;
}


/* to-be-removed */
static char *itoa(int v, unsigned int base, char *s, size_t len){
	char d;
	char inv_s[len];
	size_t i;


	/* convert int to (inverted) string */
	i = 0;

	do{
		d = (v % base) % 0xff;
		v /= base;

		if(d < 10)	inv_s[i] = '0' + d;
		else		inv_s[i] = 'a' + d - 10;

		i++;
	}while(v != 0 && i < len);

	/* check if the entire number could be converted */
	if(v != 0)
		return 0x0;

	/* reverse string */
	len = i;

	for(i=0; i<len; i++)
		s[i] = inv_s[len - 1 - i];
	s[i] = 0;

	return s;
}
