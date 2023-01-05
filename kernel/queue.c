/*
 * Name:        queue.c
 * Description: Queue.
 * Author:      K.C.Wang; cosh.cage#hotmail.com
 * File ID:     1208220252D1229221931L00065
 * License:     GPLv3
 *
 */

#include "kernel.h"

P_TASK GetTask(P_TASK * list)
{
	size_t sr = DisableInterrupt();
	P_TASK p = *list;
	if (p)
	{
		*list = p->pnext;
	}
	EnableInterrupt(sr);
	return p;
}

void PutTask(P_TASK * list, P_TASK p)
{
	size_t sr = DisableInterrupt();
	p->pnext = *list;
	*list = p;
	EnableInterrupt(sr);
}

void Enqueue(P_TASK * queue, P_TASK p)
{
	size_t sr = DisableInterrupt();
	P_TASK q = *queue;
	if (!q)
	{
		*queue = p;
		p->pnext = NULL;
		return;
	}
	if ((*queue)->priority < p->priority)
	{
		p->pnext = *queue;
		*queue = p;
		return;
	}
	while (q->pnext && p->priority <= q->pnext->priority)
		q = q->pnext;
	p->pnext = q->pnext;
	q->pnext = p;
	EnableInterrupt(sr);
}

P_TASK Dequeue(P_TASK * queue)
{
	size_t sr = DisableInterrupt();
	P_TASK p = *queue;

	if (p)
		*queue = p->pnext;
	EnableInterrupt(sr);
	return p;
}
