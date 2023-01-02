/*
 * Name:        kernel.c
 * Description: Kernel functions.
 * Author:      K.C.Wang; cosh.cage#hotmail.com
 * File ID:     1208220252C1229221931L00280
 * License:     GPLv3
 *
 */

#include "kernel.h"

TASK gTasks[NTASK];
P_TASK gpRunning = NULL, gpFreeList = NULL, gpReadyQueue = NULL, gpSleepList = NULL;

size_t gTaskSize = sizeof(TASK);
ptrdiff_t gSwitchFlag = 0;

MUTEX gReadyQMutex = { 0 }; // mutex to protect readyQueue
ptrdiff_t gIntNest = 0;   // interrupt nesting level

void InitKernel(void)
{
	P_TASK p;
	size_t i;
	// initialize rQmutex
	gReadyQMutex.lock = OFF;
	gReadyQMutex.owner = gReadyQMutex.queue = NULL;

	gpRunning = GetTask(&gpFreeList);
	gpRunning->pid = 0;
	gpRunning->priority = 0;

	
	// kprintf("kernel_init(): ");
	for (i = 0; i < NTASK; ++i)
	{
		p		   = &gTasks[i];
		p->pid	   = i;
		p->status  = TS_READY;
		p->runTime = 0;
		p->pnext   = p + 1;
	}
	gTasks[NTASK - 1].pnext = NULL;
	gpFreeList = &gTasks[0];
	gpSleepList = NULL;
	gpReadyQueue = NULL;

	// initialize rQmutex
	gReadyQMutex.lock = OFF;
	gReadyQMutex.owner = gReadyQMutex.queue = NULL;

	gpRunning = GetTask(&gpFreeList);
	gpRunning->pid = 0;
	gpRunning->priority = 0;
	//printf("task %d running\n", running->pid);
}

// readyQueue is protected by rQmutex
void Scheduler(void)
{
	//printf("task%d switch task: ", running->pid);
	if (TS_READY == gpRunning->status)
		Enqueue(&gpReadyQueue, gpRunning);
	//  printQ(readyQueue);

	gpRunning = Dequeue(&gpReadyQueue);
	//printf("next running = task%d pri=%d realpri=%d\n",
	//	  running->pid, running->priority, running->realPriority);
	/*
	color = RED + running->pid;
	if (running->pid == 0) color = YELLOW;
	if (running->pid == 5) color = CYAN;
	*/
	gSwitchFlag = 0;
}

// reschedule() called from inside V()/mutex_lock() with IRQ disabled
void Reschedule(void)
{
	/* After entering a task into gpReadyQueue, try to preempty running task. */
	if (gpReadyQueue && gpReadyQueue->priority > gpRunning->priority)
	{
		if (0 == gIntNest)
		{
			// printf("task%d PREEMPT task%d IMMEDIATELY\n", readyQueue->pid, running->pid);
			TaskSwitch();
		}
		else
		{
			//printf("task%d DEFER PREEMPT task%d ", readyQueue->pid, running->pid);
			gSwitchFlag = 1; /* IRQs are disabled, so no need to lock / unlock. */
		}
	}
}

void Schedule(void)
{
	size_t cpsr = GetCPSR();

	if (gpRunning->priority < gpReadyQueue->priority)
	{
		if ((cpsr & 0x1F) == 0x12)
			TaskSwitch();
		else
			TaskSwitch();
	}
}

ptrdiff_t KFork(void (*func)(void), ptrdiff_t priority)
{
	size_t i;
	P_TASK p = GetTask(&gpFreeList);
	if (0 == p)
	{
		// kprintf("kfork failed\n");
		return -1;
	}
	p->ppid = gpRunning->pid;
	p->pparent = gpRunning;
	p->status = TS_READY;

	p->realPriority = p->priority = priority;

	p->runTime = 0;
	p->mp = NULL;   // mutex blocked on
	p->s = NULL;	 // semaphore blocked on

	// set kstack to resume to body
	// stack = r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r14
	//		 1  2  3  4  5  6  7  8  9  10 11  12  13  14
	for (i = 1; i < 15; i++)
		p->pstack[STACK_SIZE - i] = 0;
	p->pstack[STACK_SIZE - 1] = (size_t)func;  // in dec reg=address ORDER !!!
	p->ksp = &(p->pstack[STACK_SIZE - 14]);

	MutexLock(&gReadyQMutex);

	Enqueue(&gpReadyQueue, p);

	MutexUnlock(&gReadyQMutex);

	//printQ(readyQueue);
	//printf("task%d create a child task%d\n", running->pid, p->pid); 
	Reschedule();
	return p->pid;
}

void KSleep(ptrdiff_t event)
{
	//printf("proc %d ksleep on %x\n", running->pid, event);  
	size_t sr = DisableInterrupt();
	gpRunning->event = event;
	gpRunning->status = TS_SLEEP;
	Enqueue(&gpSleepList, gpRunning);
	//printf("sleepList = "); printQ(sleepList);
	TaskSwitch();
	EnableInterrupt(sr);
}

void KWakeup(ptrdiff_t event)
{
	P_TASK p, tmp = NULL;
	int yes = 0;
	size_t sr = DisableInterrupt();
	while (NULL != (p = Dequeue(&gpSleepList)))
	{
		if (p->event == event)
		{
			//printf("kwakeup %d ", p->pid);
			p->status = TS_READY;
			Enqueue(&gpReadyQueue, p);
			yes = 1;
		}
		else
		{
			Enqueue(&tmp, p);
		}
	}
	gpSleepList = tmp;
	if (yes)
		Reschedule();
	EnableInterrupt(sr);
}

ptrdiff_t KExit(ptrdiff_t value)
{
	size_t i;;
	size_t wk1 = 0;
	P_TASK p;

	//printf("%d in kexit, value=%d\n", running->pid, value);
	if (1 == gpRunning->pid)
	{
		//printf("P1 never dies\n");
		return -1;
	}
	for (i = 1; i < NTASK; ++i)
	{
		p = &gTasks[i];
		if ((TS_FREE != p->status) && (p->ppid == gpRunning->pid))
		{
			//printf("give orphaned child%d to P1\n", p->pid);
			p->ppid = 1;
			p->pparent = &gTasks[1];
			wk1++;
		}
	}
	gpRunning->exitCode = value;
	gpRunning->status = TS_ZOMBIE;

	KWakeup((ptrdiff_t)gpRunning->pparent);
	if (wk1)
		KWakeup((ptrdiff_t)&gTasks[1]);
	TaskSwitch();
	return 0;
}

ptrdiff_t KWait(size_t * status)
{
	size_t i; 
	P_TASK p;
	size_t child = 0;
	//printf("task%d in kwait() : ", running->pid);
	p = &gTasks[0];
	for (i = 1; i < NTASK; ++i)
	{
		p = p + 1;
		//	 printf("pid=%d%d| ", p->pid, p->ppid);
		if (TS_FREE != p->status && p->ppid == gpRunning->pid)
		{
			child++;
		}
	}
	if (child == 0)
	{
		//printf("no child\n");
		return -1;
	}

	while (1)
	{
		p = &gTasks[0];
		for (i = 1; i < NTASK; ++i)
		{
			p = p + 1;
			if ((TS_ZOMBIE == p->status) && (p->ppid == gpRunning->pid)) {
				//printf("task%d free ZOMBIE child %d\n", running->pid, p->pid);
				//strcpy(zfree, "release ZOMBIE ");
				//strcat(zfree, itos(p->pid));
				//klog(zfree);
				*status = p->exitCode;
				p->status = TS_FREE;
				PutTask(&gpFreeList, p);
				//printList(freeList);

				return p->pid;
			}
		}
		// printf("task%d sleep\n", running->pid);
		KSleep((ptrdiff_t)gpRunning);
	}
}

void copy_vectors(void) {
	extern size_t vectors_start;
	extern size_t vectors_end;
	size_t * vectors_src = &vectors_start;
	size_t * vectors_dst = NULL;

	while (vectors_src < &vectors_end)
		*vectors_dst++ = *vectors_src++;
}

void irq_chandler(void)
{
	int (*f)();						 // f is a function pointer
	f = (void *)*((int *)(VIC_BASE_ADDR + 0x30)); // read ISR address in vectorAddr
	f();								// call the ISR function
	*((int *)(VIC_BASE_ADDR + 0x30)) = 1; // write to VIC vectorAddr reg as EOI
}
