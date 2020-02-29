#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

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
	struct  lockentry *lptr = &locktab[lockdes];
//	kprintf("\nLockdes = %d\n", lockdes);


	//lptr->lstate = LFREE;
	lptr->lproc[currpid] = LOCKNOTACQ;
	
	lptr->lprio = 0;
	pptr->locksState[lockdes] = NOSTATE;

	int readerCount = 0, i =0;

	if(lptr->ltype==READ)
	{
		for (i=0;i<NLOCKS; i++)
		{
			if(lptr->lproc[i]==LOCKACQ)
				readerCount++;
		}
	}

	int prev,pid;
	kprintf("\nreaderCount = %d", readerCount);
	if(lptr->ltype==WRITE || !readerCount )
	{
		/* add processes to ready Queue*/
		
		int highestWriter = -1;
		int highestReader = -2;
		int highestWriterPriority = 0;
		int highestReaderPriority = 0;
		prev = q[lptr->ltail].qprev;
		//kprintf("\nlptr tail - %d\n", prev);

		 prev = q[lptr->ltail].qprev;

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

			/* if reader and writer have the same priorities, based on ctr1000 values schedule reader or writer (if difference id less than 0.4s)*/
			if(highestReaderPriority == highestWriterPriority)
                        {
                                if( &proctab[highestWriter].procwaittime  -  &proctab[highestReader].procwaittime  <= 400)
                                {
                                        pid = dequeue(highestReader);
                                        lptr->ltype = READ;
                                        pptr->locksState[lockdes] = READ;

                                }
                                else
                                {
                                        pid = dequeue(highestWriter);
                                        lptr->ltype = WRITE;
                                        pptr->locksState[lockdes] = WRITE;

                                }
				proctab[pid].procwaittime = 0;
                                lptr->lstate = LUSED;
				lptr->lproc[pid] = LOCKACQ;
				ready(pid, RESCHNO);
                        }

			/* writer has the highest priority, dequeue that process */
			else if (highestWriterPriority != 0 && highestWriterPriority > highestReaderPriority)
			{
				pid = dequeue(highestWriter);	
				lptr->lstate = LUSED;
               		 	lptr->ltype = WRITE;
                		lptr->lproc[pid] = LOCKACQ;
                		pptr->locksState[lockdes] = WRITE;
				ready(pid, RESCHNO);	
			}

			/* reader has the highest priority, dequeue all the processes that have priority gerater than the writer process */
			else if (highestReaderPriority != 0 && highestWriterPriority < highestReaderPriority) 
			{
				while(highestReaderPriority > highestWriterPriority && prev < NPROC)
				{
					//kprintf("\nlptr q - %d\n", prev);
					pid = dequeue(highestReader);
                        		lptr->lstate = LUSED;
                        		lptr->ltype = READ;
                        		lptr->lproc[pid] = LOCKACQ;
                        		pptr->locksState[lockdes] = READ;
        	               		ready(pid, RESCHNO);

					prev =  q[prev].qprev;
					if(proctab[prev].locksState[lockdes] == READ){
						highestReader = prev;
                                        	highestReaderPriority = q[prev].qkey;
					}
				}
		
			}
			resched();
		}
		else
		{
		 	lptr->lstate = LFREE;
		}
	}
}
