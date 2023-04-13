/*
 * Name:        main.c
 * Description: Startup.
 * Author:      K.C.Wang; cosh.cage#hotmail.com
 * File ID:     1208220252H1229221931L00037
 * License:     LGPLv2
 *
 */

#include "kernel.h"

#include "queue.c"
#include "mutex.c"
#include "exceptions.c"
#include "kernel.c"
#include "semaphore.c"

extern P_TASK gpReadyQueue;

void task1(void)
{
	for (;;);
}

int main()
{

	InitKernel();
	KFork(task1, 1);
	
	while (1)
	{
		if (gpReadyQueue)
			TaskSwitch();
	}
}
