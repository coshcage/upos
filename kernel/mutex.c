/*
 * Name:	    mutex.c
 * Description: Mutexes.
 * Author:      K.C.Wang; cosh.cage#hotmail.com
 * File ID:     1208220252F1229221931L00108
 * License:     GPLv3
 *
 */

#include "kernel.h"

extern P_TASK gpRunning;
extern P_TASK gpReadyQueue;

BOOL MutexLock(P_MUTEX s)
{
	P_TASK q, temp;  /* Current owner if mutex is locked. */

	size_t sr = DisableInterrupt();
	// printf("TASK%d locking mutex: ", running->pid);

	if (OFF == s->lock)
	{
		s->lock = ON;
		s->owner = gpRunning;
		gpRunning->priority = gpRunning->realPriority;
	}
	else
	{
		if (s->owner == gpRunning)
		{
			/* Already locked by this task. */
			// printf("mutex already locked by you!\n");
			return TRUE;
		}
		// printf("TASK%d BLOCK ON MUTEX: ", running->pid);
		gpRunning->status = TS_BLOCK;
		gpRunning->mp = s; /* This task is blocked on this mutex. */
		Enqueue(&s->queue, gpRunning);

		/* Boost owner priority to running's priority. */
		if (gpRunning->priority > s->owner->priority)
		{
			s->owner->priority = gpRunning->priority;
			//printf("RAISE PROC%d PRIORITY to %d\n", s->owner->pid, s->owner->priority);
			/* Re-order gpReadyQueue. */
			temp = NULL;
			while (q = Dequeue(&gpReadyQueue))
				Enqueue(&temp, q);
			gpReadyQueue = temp;
		}
		TaskSwitch();
	}
	EnableInterrupt(sr);
	return TRUE;
}

BOOL MutexUnlock(P_MUTEX s)
{
	P_TASK p;
	size_t sr = DisableInterrupt();
	// printf("task%d unlocking mutex: ", running->pid);
	// printf("priority=%d realpriority=%d\n",
	//	   running->priority, running->realPriority);

	if (OFF == s->lock)
	{
		// printf("mutex_unlock error: mutex %x is NOT locked\n", s);
		EnableInterrupt(sr);
		return FALSE;
	}
	if (s->owner != gpRunning)
	{
		//printf("mutex_unlock error: not owner\n");
		EnableInterrupt(sr);
		return FALSE;
	}
	/* Owner and mutex was locked. */
	if (NULL == s->queue) { /* Mutex has waiters. */
		/* Mutex has no waiter. */
		s->lock = OFF; /* Clear lock. */
		s->owner = NULL;  /* Clear owner. */
		/* Revert back to original priority. */
	}
	else
	{
		/* Mutex has waiters. Unblock one as new owner. */
		p = Dequeue(&s->queue);
		p->status = TS_READY;
		s->owner = p;
		//printf("%d mutex_unlock: new owner=task%d ", running->pid, p->pid);

		Enqueue(&gpReadyQueue, p);

		if (gpRunning->priority > gpRunning->realPriority) {
			//printf("restore task%d priorit to %d\n", running->pid, running->realPriority);
			gpRunning->priority = gpRunning->realPriority;
		}

		Reschedule();

	}

	EnableInterrupt(sr);

	return TRUE;
}
