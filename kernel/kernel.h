/*
 * Name:        kernel.h
 * Description: Kernel definitions.
 * Author:      K.C.Wang; cosh.cage#hotmail.com
 * File ID:     1208220252A1229221931L00097
 * License:     GPLv3
 *
 */

#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "osdef.h"

#define STACK_SIZE (4096)
#define NTASK (5)

typedef enum en_TASK_STATUS {
	TS_FREE   = 'F',
	TS_READY  = 'R',
	TS_SLEEP  = 'S',
	TS_BLOCK  = 'B',
	TS_ZOMBIE = 'Z',
	TS_PAUSE  = 'P'
} TASK_STATUS;

typedef struct st_MUTEX {
	BOOL             lock;     /* 0: unlocked, 1: locked. */
	ptrdiff_t        priority; /* Current priority. */
	struct st_TASK * owner;    /* Pointer to current owner. */
	struct st_TASK * queue;    /* Blocked tasks. */
} MUTEX, * P_MUTEX;

typedef struct st_SEMAPHORE {
	ptrdiff_t        value;
	struct st_TASK * queue;
} SEMAPHORE, * P_SEMAPHORE;

typedef struct st_TASK {
	struct st_TASK * pnext;
	struct st_TASK * pparent;
	size_t *         ksp;
	ptrdiff_t        pid;
	ptrdiff_t        ppid;
	ptrdiff_t        priority;     /* Created priority. */
	ptrdiff_t        realPriority; /* Current priority; Same as created when start. */
	ptrdiff_t        runTime;
	TASK_STATUS      status;
	ptrdiff_t        event;
	ptrdiff_t        exitCode;

	struct st_MUTEX *     mp;       /* Point to mutex this task is blocked on. */
	struct st_SEMAPHORE * s;        /* Point to semaphore this task is blocked on. */
	size_t pstack[STACK_SIZE];
} TASK, * P_TASK;

#define VIC_BASE_ADDR (0x10140000)

extern size_t DisableInterrupt(void);
extern void   EnableInterrupt (size_t sr);
extern void   TaskSwitch(void);
extern size_t GetCPSR(void);

P_TASK GetTask(P_TASK * list);
void   PutTask(P_TASK * list, P_TASK p);
void   Enqueue(P_TASK * queue, P_TASK p);
P_TASK Dequeue(P_TASK * queue);

void InitSemaphore(P_SEMAPHORE s, ptrdiff_t value);
void SemaphoreP(P_SEMAPHORE s);
void SemaphoreV(P_SEMAPHORE s);

void InitMutex(P_MUTEX m);
BOOL MutexLock(P_MUTEX s);
BOOL MutexUnlock(P_MUTEX s);

void InitKernel(void);
void Scheduler(void);
void Reschedule(void);
void Schedule(void);
ptrdiff_t KFork(void (*func)(void), ptrdiff_t priority);
void      KSleep(ptrdiff_t event);
void      KWakeup(ptrdiff_t event);
ptrdiff_t KExit(ptrdiff_t value);
ptrdiff_t KWait(size_t * status);

void undef_handler(void);
void swi_handler(void);
void prefetch_abort_handler(void); 
void data_abort_handler(void);
void fiq_handler(void);

void irq_chandler(void);
void copy_vectors(void);

#endif
