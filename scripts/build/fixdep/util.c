/**
 * Copyright (C) 2015 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Most of source code is taken from the linux kernel's build helper fixdep.c
 * but has been restructured for the purpose of this project.
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "util.h"


int file_map(const char *filename, int *_fd, void **_map, unsigned int *_size){
	int fd;
	void *map;
	struct stat st;


	if(_fd == 0 || _map == 0 || _size == 0)
		return -1;

	fd = open(filename, O_RDWR);

	if(fd < 0){
		fprintf(stderr, "fixdep: error opening file \"%s\": %s\n", filename, strerror(errno));
		return -1;
	}

	if(fstat(fd, &st) < 0){
		fprintf(stderr, "fixdep: error fstat'ing file: %s\n", strerror(errno));
		goto err;
	}

	if(st.st_size == 0)
		goto err;

	map = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

	if((long)map == -1){
		perror("fixdep: mmap");
		goto err;
	}

	*_fd = fd;
	*_map = map;
	*_size = st.st_size;

	return 0;

err:
	close(fd);
	return -1;
}

void file_unmap(int fd, void *map, unsigned int size){
	munmap(map, size);
	close(fd);
}

/* test is s ends in sub */
int strrcmp(char *s, unsigned int slen, char *sub, unsigned int sublen){
	if(sublen > slen)
		return 1;

	return memcmp(s + slen - sublen, sub, sublen);
}

void traps(void){
	static char test[] __attribute__((aligned(sizeof(int)))) = "CONF";
	int *p =(int *)test;

	if(*p != INT_CONF){
		fprintf(stderr, "fixdep: sizeof(int) != 4 or wrong endianness? %#x\n",
			*p);
		exit(2);
	}
}
