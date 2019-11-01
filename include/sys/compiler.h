/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_COMPILER_H
#define SYS_COMPILER_H


/* macros */
// stringify
#define STRGIFY(s)		#s
#define STR(s)			STRGIFY(s)

// error and warnings
#define BUILD_ASSERT(cond)				((void)sizeof(char[1 - 2*!(cond)]))

#define GCC_ERROR(s)					_Pragma(STRGIFY(GCC error s))
#define GCC_WARNING(s)					_Pragma(STRGIFY(GCC warning s))

#define CPP_ERROR(s)					GCC_ERROR(STRGIFY(s))
#define CPP_WARNING(s)					GCC_WARNING(STRGIFY(s))

#define CALL_DISABLED(_call, _option)	({ CPP_ERROR(function STRGIFY(_call()) disabled - check kernel configuration _option); 0x0; })

// suppress 'unused' compiler warning
#define __used			__attribute__((used))

// variables are meant to appear possibly unused
#define __unused		__attribute__((unused))

// for enu, struct, union types specify that the minimum required memory
// be used to represent the type
#define __packed		__attribute__((packed))

// assign variable to specified section
#define __section(sec)	__attribute__((section(sec)))

// specify minimum alignment
#define __align(base)	__attribute__((aligned(base)))

// declaration is alias for another symbol
#define __alias(f)		__attribute__((alias (#f)))

// specify width of a type
// 	QI	integer that is as wide as the smallest addressable unit, usually 8 bits
// 	HI	integer, twice as wide as a QI mode integer, usually 16 bits
// 	SI	integer, four times as wide as a QI mode integer, usually 32 bits
// 	DI	integer, eight times as wide as a QI mode integer, usually 64 bits
// 	SF	float, as wide as a SI mode integer, usually 32 bits
// 	DF	float, as wide as a DI mode integer, usually 64 bits
#define __mode(m)		__attribute__((mode(m)))

// offsetof
#include <stddef.h>
#define offsetofvar(var, member)	((size_t)(&(((typeof((var))*)(0))->member)))


#endif // SYS_COMPILER_H
