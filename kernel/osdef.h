/*
 * Name:        osdef.h
 * Description: Common definitions.
 * Author:      K.C.Wang; cosh.cage#hotmail.com
 * File ID:     1208220252B1229221931L00032
 * License:     LGPLv2
 *
 */

#ifndef _OSDEF_H_
#define _OSDEF_H_

typedef unsigned int  size_t;
typedef int           ptrdiff_t;

#define NULL          ((void *)0)
// #define offsetof(s,m) ((size_t)&(((s*)0)->m))

/* The above codes are from stddef.h. */

typedef int           BOOL;
#define TRUE          (1)
#define FALSE         (0)

#define ON            TRUE
#define OFF           FALSE

typedef unsigned char BYTE;
typedef unsigned char * P_BYTE;

#endif
