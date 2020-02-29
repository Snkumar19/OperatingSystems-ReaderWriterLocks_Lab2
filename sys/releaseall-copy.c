#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

int releaseall (int numlocks, int args)
{
	//kprintf("\nIn releaseAll\n");
	int *a;	
	a = &args;
	int i = 0;
	for ( i = 0 ; i < numlocks ; i++){	/* machine dependent; copy args	*/
		//kprintf("\na = %d\n", *a);
		release(a);
		a--;
	}
	return OK;
}

void release(int *ldes)
{
	struct  pentry  proctab[NPROC];
	//kprintf("\nldesc = %d\n", *ldes);
	struct pentry *pptr = &proctab[currpid];
	struct  lockentry *lptr = &locktab[*ldes];

	//lptr->lstate = LFREE;
	lptr->lproc[currpid] = LOCKNOTACQ;
	
	lptr->lprio = 0;
	pptr->locksState[*ldes] = NOSTATE;

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
//	kprintf("\nreaderCount = %d", readerCount);
	if(lptr->ltype==WRITE || readerCount )
	{
		/* add processes to ready Queue*/
		
		int highestWriter = -1;
		int highestReader = -2;
		int highestWriterPriority = 0;
		int highestReaderPriority = 0;
		prev = q[lptr->ltail].qprev;
		//kprintf("\nlptr tail - %d\n", prev);

		while(prev<NPROC)
                        {
                        //        if(q[prev].qltype == WRITE){
                              //  kprintf("\nQueue: %d\n", q[prev].qkey);
                                prev =  q[prev].qprev;
                        }
		 prev = q[lptr->ltail].qprev;

		if (prev < NPROC)
		{
			while(prev<NPROC)
			{
				if(&proctab[prev].locksState[*ldes] == WRITE){
					highestWriter = prev;
					highestWriterPriority = q[prev].qkey;
					break;
				}
				prev =  q[prev].qprev;
			}
			prev = q[lptr->ltail].qprev;
		 	while(prev<NPROC)
                	{
				if(&proctab[prev].locksState[*ldes] == READ){
                                	highestReader = prev;
					highestReaderPriority = q[prev].qkey;
					break;
				}
				prev =  q[prev].qprev;
			}

			prev = q[lptr->ltail].qprev;

			if (highestWriterPriority != 0 && highestWriterPriority > highestReaderPriority)
			{
				pid = dequeue(highestWriter);	
				lptr->lstate = LUSED;
               		 	lptr->ltype = WRITE;
                		lptr->lproc[pid] = LOCKACQ;

                		pptr->locksState[*ldes] = WRITE;

				ready(pid, RESCHNO);	
			}

			else if (highestReaderPriority != 0 && highestWriterPriority < highestReaderPriority) 
			{
				while(highestReaderPriority > highestWriterPriority && prev < NPROC)
				{
					//kprintf("\nlptr q - %d\n", prev);
					pid = dequeue(highestReader);
                        		lptr->lstate = LUSED;
                        		lptr->ltype = READ;
                        		lptr->lproc[pid] = LOCKACQ;
                        
                        		pptr->locksState[*ldes] = READ;
	
        	               		ready(pid, RESCHNO);

					prev =  q[prev].qprev;
					if(&proctab[prev].locksState[*ldes] == READ){
						highestReader = prev;
                                        	highestReaderPriority = q[prev].qkey;
					}
				}
		
			}
			else if(highestReaderPriority == highestWriterPriority)
			{
				//kprintf("\nequal prio\n");
				if( &proctab[highestWriter].procwaittime  -  &proctab[highestReader].procwaittime  <= 400)
				{
					pid = dequeue(highestReader);
					proctab[highestReader].procwaittime = 0;
                                        lptr->lstate = LUSED;
                                        lptr->ltype = READ;
                                        lptr->lproc[pid] = LOCKACQ;

                                        pptr->locksState[*ldes] = READ;

                                        ready(pid, RESCHNO);

				}
				else
				{
					 pid = dequeue(highestWriter);
					proctab[highestWriter].procwaittime =0;
	                                lptr->lstate = LUSED;
        	                        lptr->ltype = WRITE;
                	                lptr->lproc[pid] = LOCKACQ;

                        	        pptr->locksState[*ldes] = WRITE;

                                	ready(pid, RESCHNO);

				}	
			}
			 
		resched();
		}
	}
}
