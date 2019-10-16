#ifndef INET_H
#define INET_H


#include <net.h>


/* types */
typedef uint32_t inet_addr_t;

typedef struct{
	sock_type_t type;

	inet_addr_t addr;
	uint16_t port;
} inet_data_t;

typedef struct{
	net_family_t familty;
	inet_data_t data;
} sock_addr_inet_t;


/* prototypes */
inet_addr_t inet_addr(char const *addr);


#endif // INET_H
