#include <stdio.h>
#include <netdev.h>
#include <esp01.h>


/* global functions */
int main(int argc, char **argv){
	int r;


	printf("hello\n");

	r = 0;
	r |= netdev_init();
	r |= esp01_init();

	if(r){
		printf("init failed\n");
		return 1;
	}

	return 0;
}
