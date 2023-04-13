/*
 * Name:        exceptions.c
 * Description: Function exceptions.
 * Author:      K.C.Wang; cosh.cage#hotmail.com
 * File ID:     1208220252G0413231619L00036
 * License:     GPLv3
 *
 */

#include "kernel.h"

void __attribute__((interrupt)) UndefHandler(void)
{
	for (;;);
}

void __attribute__((interrupt)) SwiHandler(void)
{
	for (;;);
}

void __attribute__((interrupt)) PrefetchAbortHandler(void)
{
	for (;;);
}

void __attribute__((interrupt)) DataAbortHandler(void)
{
	for (;;);
}

void __attribute__((interrupt)) FiqHandler(void)
{
	for (;;);
}
