#ifndef NET_H
#define NET_H


#include <types.h>


/* types */
typedef enum{
	AF_INET = 0,
} net_family_t;

typedef enum{
	SOCK_STREAM = 1,
	SOCK_DGRAM,
} sock_type_t;

typedef struct{
	net_family_t domain;
	uint8_t data[];
} sock_addr_t;


#endif // NET_H
