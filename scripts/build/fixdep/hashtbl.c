/**
 * Copyright (C) 2015 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Most of source code is taken from the linux kernel's build helper fixdep.c
 * but has been restructured for the purpose of this project.
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "hashtbl.h"


/* macros */
#define HASHSZ 256


/* static prototypes */
static unsigned int strhash(const char *str, unsigned int sz);


/* global variables */
static struct item *hashtab[HASHSZ];


/* global functions */
/*
 * Lookup a value in the configuration string.
 */
int hashtbl_lookup(const char *name, int len, unsigned int hash){
	struct item *aux;

	for(aux = hashtab[hash % HASHSZ]; aux; aux = aux->next){
		if(aux->hash == hash && aux->len == len &&
		    memcmp(aux->name, name, len) == 0)
			return 1;
	}

	return 0;
}

/*
 * Add a new value to the configuration string.
 */
int hashtbl_add(const char *name, int len){
	struct item *aux;
	unsigned int hash = strhash(name, len);


	if(hashtbl_lookup(name, len, hash))
	    return 1;

	aux = malloc(sizeof(*aux) + len);

	if(!aux){
		perror("fixdep:malloc");
		exit(1);
	}

	memcpy(aux->name, name, len);
	aux->len = len;
	aux->hash = hash;
	aux->next = hashtab[hash % HASHSZ];
	hashtab[hash % HASHSZ] = aux;

	return 0;
}

/*
 * Clear the set of configuration strings.
 */
void hashtbl_clear(void){
	struct item *aux, *next;
	unsigned int i;

	for(i = 0; i < HASHSZ; i++){
		for(aux = hashtab[i]; aux; aux = next){
			next = aux->next;
			free(aux);
		}
		hashtab[i] = NULL;
	}
}


/* local functions */
unsigned int strhash(const char *str, unsigned int sz){
	/* fnv32 hash */
	unsigned int i, hash = 2166136261U;

	for(i = 0; i < sz; i++)
		hash =(hash ^ str[i]) * 0x01000193;
	return hash;
}
