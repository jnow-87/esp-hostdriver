/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_FCNTL_H
#define SYS_FCNTL_H


#include <stddef.h>


/* types */
typedef enum{
	F_SEEK = 1,		// implementation required on lowest callback level
	F_TELL,			// implementation required on lowest callback level
	F_MODE_GET,		// implemented at highest callback level
	F_MODE_SET,		// implemented at highest callback level
	F_STAT,			// implementation required on lowest callback level
} fcntl_cmd_t;

typedef enum{
	SEEK_SET = 1,
	SEEK_CUR,
	SEEK_END,
} whence_t;

typedef struct{
	whence_t whence;
	size_t pos;
	int offset;
} seek_t;

typedef enum{
	O_CREAT = 0x1,
	O_APPEND = 0x2,
	O_RDONLY = 0x4,
	O_WRONLY = 0x8,
	O_RDWR = 0xc,
	O_NONBLOCK = 0x10,
} f_mode_t;


#endif // SYS_FCNTL_H
