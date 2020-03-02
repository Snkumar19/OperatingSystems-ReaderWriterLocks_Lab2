#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

void lockAcquired(int highestWriterOrReader, int lockdes, int type);
int releaseall (int numlocks, int args)
{
	 STATWORD ps;
        disable(ps);
	//kprintf("\nIn releaseAll\n");
	int *a;	
	a = &args;
	int i = 0;
	for ( i = 0 ; i < numlocks ; i++){	/* machine dependent; copy args	*/
		//kprintf("\na = %d\n", *a);
		release(a);
		a--;
	}
	restore(ps);
	return OK;
}

void release(int *ldes)
{
	struct pentry *pptr = &proctab[currpid];
	int lockdes = *ldes;
	int prev,pid;
	struct  lockentry *lptr = &locktab[lockdes];

	int readerCount = 0, i =0, lastReaderCheck = 0;

	if(lptr->ltype==READ)
	{
		for (i=0;i<NLOCKS; i++)
		{
			if(lptr->lproc[i]==LOCKACQ)
				readerCount++;
		}
	}

	if(readerCount == 1)
		lastReaderCheck = 1;

	lptr->lproc[currpid] = LOCKNOTACQ;
	kprintf("\nreaderCount = %d", readerCount);

	/* If last reader or writer releases the lock, it can be acquired by reader/writer */
	if(lptr->ltype==WRITE || lastReaderCheck )
	{
		/* add processes to ready Queue*/
		
		int highestWriter = -1;
		int highestReader = -2;
		int highestWriterPriority = 0;
		int highestReaderPriority = 0;
		prev = q[lptr->ltail].qprev;
		//kprintf("\nlptr tail - %d\n", prev);

		if (prev < NPROC)
		{
			/* Find the writer with highest priority from lock queue */
			while(prev<NPROC)
			{
				//kprintf("type: %d", proctab[prev].locksState[lockdes]);
				if(proctab[prev].locksState[lockdes] == WRITE){
					highestWriter = prev;
					highestWriterPriority = q[prev].qkey;
					break;
				}
				prev =  q[prev].qprev;
			}
			/* Find the reader with highest priority from lock queue */
			prev = q[lptr->ltail].qprev;
		 	while(prev<NPROC)
                	{
				if(proctab[prev].locksState[lockdes] == READ){
                                	highestReader = prev;
					highestReaderPriority = q[prev].qkey;
					break;
				}
				prev =  q[prev].qprev;
			}

			prev = q[lptr->ltail].qprev;

			
			/* WRITER can acquire lock under 2 conditions:
 			* 1. When reader and writer have equal priority and wait time is greater than 400
 			* 2. When writer priority is greater than readre priority */
			if(((highestReaderPriority == highestWriterPriority) && (&proctab[highestWriter].procwaittime  -  &proctab[highestReader].procwaittime >  400)) || (highestWriterPriority != 0 && highestWriterPriority > highestReaderPriority))
                        {
				lockAcquired(highestWriter, lockdes, WRITE);
			}
			/* READER can acuire can lock when both reader have same priority and the wait time differnce is less than 400ms */
                        else if((highestReaderPriority == highestWriterPriority) && (&proctab[highestWriter].procwaittime  -  &proctab[highestReader].procwaittime  <= 400))
                        {
				while((highestReaderPriority == highestWriterPriority) && (&proctab[highestWriter].procwaittime  -  &proctab[highestReader].procwaittime  <= 400) && (prev < NPROC))
                                {
					lockAcquired(highestReader, lockdes, READ);
					
					prev =  q[prev].qprev;
                                        if(proctab[prev].locksState[lockdes] == READ){
                                                highestReader = prev;
                                                highestReaderPriority = q[prev].qkey;
                                        }
                        	}
			}

			/* READER can acquire lock when reader has higher priority than writer */
			else if (highestReaderPriority != 0 && highestWriterPriority < highestReaderPriority) 
			{
				while(highestReaderPriority > highestWriterPriority && prev < NPROC)
				{
					//kprintf("\nlptr q - %d\n", prev);
					lockAcquired(highestReader, lockdes, READ);

					prev =  q[prev].qprev;
					if(proctab[prev].locksState[lockdes] == READ){
						highestReader = prev;
                                        	highestReaderPriority = q[prev].qkey;
					}
				}
		
			}
			resched();
		}
		/* If Wait Queue is empty, change state to free */
		else
		{
		 	lptr->lstate = LFREE;
			pptr->locksState[lockdes] = EMPTY;
		}
	}
	else
	{
		proctab[currpid].locksState[lockdes] = EMPTY;
		lptr->lstate = LUSED;
		lptr->ltype = READ;
	}
}

void lockAcquired(int highestWriterOrReader, int lockdes, int type)
{
        struct  lockentry *lptr = &locktab[lockdes];
	int pid;
	pid = dequeue(highestWriterOrReader);
        lptr->lprio = findMaxPriority(lockdes);
        lptr->lstate = LUSED;
        lptr->ltype = type;
        lptr->lproc[pid] = LOCKACQ;
        proctab[pid].locksState[lockdes] = type;
        proctab[pid].procwaittime = 0;
        ready(pid, RESCHNO);

}
