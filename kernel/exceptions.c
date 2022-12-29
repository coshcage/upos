/*
 * Name:        exceptions.c
 * Description: Function exceptions.
 * Author:      K.C.Wang; cosh.cage#hotmail.com
 * File ID:     1208220252G1229221931L00017
 * License:     GPLv3
 *
 */

#include "kernel.h"

void __attribute__((interrupt)) undef_handler(void) { for (;;); }
void __attribute__((interrupt)) swi_handler(void) { for (;;); }
void __attribute__((interrupt)) prefetch_abort_handler(void) { for (;;); }
void __attribute__((interrupt)) data_abort_handler(void) { for (;;); }
void __attribute__((interrupt)) fiq_handler(void) { for (;;); }
