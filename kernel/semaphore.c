/*
 * Name:        semaphore.c
 * Description: Semaphore.
 * Author:      K.C.Wang; cosh.cage#hotmail.com
 * File ID:     1208220252E1229221931L00057
 * License:     GPLv3
 *
 */

#include "kernel.h"

extern P_TASK gpRunning;
extern P_TASK gpReadyQueue;

void InitSemaphore(P_SEMAPHORE s, ptrdiff_t value)
{
	s->value = value;
	s->queue = NULL;
}

void SemaphoreP(P_SEMAPHORE s)
{
	size_t sr = DisableInterrupt();
	// printf("%d in P s=%d\n", running->pid, s->value);
	--s->value;
	if (s->value < 0)
	{
		//printf("proc%d BLOCKED on %x\n", running->pid, s);
		gpRunning->status = TS_BLOCK;
		Enqueue(&s->queue, gpRunning);
		EnableInterrupt(sr);
		TaskSwitch();
		return;
	}
	EnableInterrupt(sr);
}

void SemaphoreV(P_SEMAPHORE s)
{
	P_TASK p;
	size_t sr = DisableInterrupt();

	++s->value;
	//printf("%d in V sValue=%d\n", running->pid, s->value);
	if (s->value <= 0)
	{
		p = Dequeue(&s->queue);
		p->status = TS_READY;
		Enqueue(&gpReadyQueue, p);

		//printf("V up task%d pri=%d; running pri=%d\n", p->pid, p->priority, running->priority);

		Reschedule();
	}
	EnableInterrupt(sr);
}
