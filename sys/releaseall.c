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

	 int i = 0;
	//kprintf("\nIn releaseAll\n");

	int *a;	
	a = &args;

	a = &args;
	for ( i = 0 ; i < numlocks ; i++){	/* machine dependent; copy args	*/
		kprintf("\nAAAAAAAAAA = %d\n", *a);
		release(a);
		kprintf("\nAAAAAAAAAA - after releasing = %d\n", *a);
		a = a + 1;
	}

               

//	updateMaxPrio(*a,0, currpid);
	restore(ps);
	return OK;
}

void release(int *ldes)
{
	 STATWORD ps;
        disable(ps);
	struct pentry *pptr = &proctab[currpid];
	int lockdes = *ldes;
	int prev,pid;
	struct  lockentry *lptr = &locktab[lockdes];

	 if (isbadlock(lockdes) || pptr->locksState[lockdes] == DELETED || pptr->locksState[lockdes] == EMPTY || lptr->lstate == LFREE) {
		kprintf("\nRELEASE ERROR = %d\n", lockdes);
                restore(ps);
                return(SYSERR);
        
	}
	
kprintf("\na = %d\n", lockdes);
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
	pptr->locksState[lockdes] = EMPTY;
//	kprintf("\nreaderCount = %d", readerCount);

	/* If last reader or writer releases the lock, it can be acquired by reader/writer */
	if(lptr->ltype==WRITE || lastReaderCheck )
	{

		/* Print the Wait Queue*/
		while(prev<NPROC)
                {
                	if(proctab[prev].locksState[lockdes] == READ){
                        	kprintf("\nREAD=%d",q[prev].qkey);
                        }
                       	prev =  q[prev].qprev;
                }
		
		kprintf("\nWriter realeased lock or Last Reader released lock\n");
		prev = q[lptr->ltail].qprev;

		int writerId, writerPriority, readerId, readerPriority;
		 int writerId1, writerPriority1, readerId1, readerPriority1;	
		/* Check if Wait Queue has some processes*/
		if (prev < NPROC)
		{
			/* if First process is a writer
 			* 1. Find the reader with highest priority, if not found - writer can acquire lock
 			* 2. Check if reader and writer have equal priority - If yes, give lock to all readers who have wait time difference less than 400 ms
 			* 3. If differnce is greater than 400, writer gets the lock
 			*/ 
			if (proctab[prev].locksState[lockdes] == WRITE)
			{
				writerId = prev;
				writerPriority = q[prev].qkey;

				readerId = -1;
				readerPriority = 0;
			
				/* Find reader priority*/	
				prev = q[prev].qprev;
	                        while(prev<NPROC)
        	                {
                	                if(proctab[prev].locksState[lockdes] == READ){
                        	                readerId = prev;
                                	        readerPriority = q[prev].qkey;
                                        	break;
                                	}
                               		 prev =  q[prev].qprev;
                        	}
				/* Check case 2  */
				if (readerId != -1)
				{
					if((readerPriority == writerPriority) &&((&proctab[writerId].procwaittime  -  &proctab[readerId].procwaittime)  <= 400))
					{
						while((readerPriority == writerPriority) && (&proctab[writerId].procwaittime  -  &proctab[readerId].procwaittime  <= 400) && (prev < NPROC))
                                		{
                                        		lockAcquired(readerId, lockdes, READ);

							prev = q[lptr->ltail].qprev;
                                        		prev =  q[prev].qprev;
                                        		if(proctab[prev].locksState[lockdes] == READ){
                                      	        		readerId = prev;
                                                		readerPriority = q[prev].qkey;
                                        		}
                                		}
					}
					/* case 3*/
					else
					{
						lockAcquired(writerId, lockdes, WRITE);
					}

				}
				/* if no reader*/
				else
				{
				/* writer gets it */
					lockAcquired(writerId, lockdes, WRITE);
				}

			}

			/* check if first process is reader, all readers who have riority greater than the highest priority writer will get the lock */
			else if(proctab[prev].locksState[lockdes] == READ)
			{
				kprintf("\nHIT this case............\n");
				/*
 				* while(prev<NPROC)
				* {
 				* if(proctab[prev].locksState[lockdes] == READ){
				*kprintf("\nREAD=%d",q[prev].qkey);
				*} 						                                		                						                                		                                        	                        		                        		prev =  q[prev].qprev;
 				* 						                                		                                        	                        		                        		               			 }*/

				readerId1 = prev;
                                readerPriority1 = q[prev].qkey;

                                writerId1 = -1;
                                writerPriority1 = 0;

                                /* Find writer priority*/
                                prev = q[prev].qprev;
                                while(prev<NPROC)
                                {
                                        if(proctab[prev].locksState[lockdes] == WRITE){
                                                writerId1 = prev;
                                                writerPriority1 = q[prev].qkey;
                                                break;
                                        }
                                         prev =  q[prev].qprev;
                                }
                                /* Check case 2  */
                                if (writerId1 != -1)
                                {
					kprintf("\nWRITER ID - CHECK 1\n");
                                        if((readerPriority1 == writerPriority1) &&((&proctab[writerId1].procwaittime  -  &proctab[readerId1].procwaittime)  <= 400))
                                        {
                                                while((readerPriority1 == writerPriority1) && (&proctab[writerId1].procwaittime  -  &proctab[readerId1].procwaittime  <= 400) && (prev < NPROC))
                                                {
                                                        lockAcquired(readerId1, lockdes, READ);

                                                        prev = q[lptr->ltail].qprev;
                                                        prev =  q[prev].qprev;
                                                        if(proctab[prev].locksState[lockdes] == READ){
                                                                readerId1 = prev;
                                                                readerPriority1 = q[prev].qkey;
                                                        }
                                                }
                                        }
                                        /* case 3*/
                                        else
                                        {
                                                while(proctab[prev].locksState[lockdes] == READ  && prev < NPROC)
		                                {
                		                        kprintf("\nHIT THIS CASE----------------\n");
	
        		                                kprintf("\nQueue Item =%d\n",q[prev].qkey);
                        		                lockAcquired(prev, lockdes, READ);
	
        		                                prev =  q[lptr->ltail].qprev;
                        		                kprintf("\nQueue Item2 =%d\n",q[prev].qkey);
                                  		}

                                        }

                                }
                                /* if no writer*/
                                else
                                {
				kprintf("\nWRITER ID - CHECK 2\n");
                                /* reader gets it */
					prev =  q[lptr->ltail].qprev;
					while(proctab[prev].locksState[lockdes] == READ  && prev < NPROC)
                                        	{
                                                        kprintf("\nHIT THIS CASE----------------\n");

                                                        kprintf("\nQueue Item =%d\n",q[prev].qkey);
                                                        lockAcquired(prev, lockdes, READ);

                                                        prev =  q[lptr->ltail].qprev;
                                                        kprintf("\nQueue Item2 =%d\n",q[prev].qkey);
                                                }
                                }
                          }
	
			resched();
		}
		/* if there are no more items in the wait queue */
		else
		{
			kprintf("\nFREE STATE\n");
			lptr->lstate = LFREE;
                        pptr->locksState[lockdes] = EMPTY;
		}
	}
	/* if there are more readeers using this lock */
 	else
        {
                proctab[currpid].locksState[lockdes] = EMPTY;
                lptr->lstate = LUSED;
                lptr->ltype = READ;
        }
 restore(ps);
  return OK;

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
	proctab[pid].lockid = NOTINWQ;
        proctab[pid].locksState[lockdes] = type;
        proctab[pid].procwaittime = 0;
        ready(pid, RESCHNO);

}
