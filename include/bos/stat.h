/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_STAT_H
#define SYS_STAT_H


#include <sys/types.h>


/* types */
typedef enum{
	FT_BLK = 1,
	FT_CHR,
	FT_DIR,
	FT_REG,
	FT_LNK,
} file_type_t;

typedef struct{
	file_type_t type;
	size_t size;
} stat_t;


#endif // SYS_STAT_H
