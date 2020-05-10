/**
 * Copyright (C) 2015 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Most of source code is taken from the linux kernel's build helper fixdep.c
 * but has been restructured for the purpose of this project.
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef UTIL_H
#define UTIL_H


#include <arpa/inet.h>


/* macros */
#define INT_CONF ntohl(0x434f4e46)
#define INT_ONFI ntohl(0x4f4e4649)
#define INT_NFIG ntohl(0x4e464947)
#define INT_FIG_ ntohl(0x4649475f)


/* prototypes */
int file_map(const char *filename, int *fd, void **map, unsigned int *size);
void file_unmap(int fd, void *map, unsigned int size);

int strrcmp(char *s, unsigned int slen, char *sub, unsigned int sublen);

void traps(void);


#endif // UTIL_H
