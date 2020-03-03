#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

extern struct  lockentry locktab[NLOCKS];
extern unsigned long ctr1000;

void waitForLock(int currpid, int ldes1, int type, int priority);

int lock (int ldes1, int type, int priority){
	
	STATWORD ps;
	int i = 0;
	struct  lockentry *lptr = &locktab[ldes1];
	struct pentry *pptr = &proctab[currpid];
	disable(ps);

	 if (isbadlock(ldes1) || pptr->locksState[ldes1] == DELETED) {
                restore(ps);
                return(SYSERR);
        }

//	int ownerofLock = findLockOwner(ldes1);
//	kprintf("\nMax prio: %d\n", findMaxPriority(ldes1));
//	updateMaxPrio(ldes1,priority, currpid);
//	if (ownerofLock == -1)
//		ownerofLock = currpid; 
	
	if (locktab[ldes1].lstate == LCREATE || locktab[ldes1].lstate == LFREE)
	{
		pptr->lockid = NOTINWQ;
		lptr->lstate = LUSED;
		lptr->ltype = type;
		lptr->lprio = priority;
		lptr->lproc[currpid] = LOCKACQ;

		pptr->locksState[ldes1] = type;
		//kprintf ("\n LOCK LSTATE IS FREE : %d, %d, %d, %d %d\n",ldes1, ownerofLock, lptr->lproc[ownerofLock], pptr->locksState[ldes1],lptr->lprio );

		restore(ps);
		return(OK);
	}
	/* Control Comes here only if the State is not Free */
	if (locktab[ldes1].lstate == LUSED &&  lptr->ltype == WRITE)
	{
		// 	SINCE Writers need exclusive locks, put them to WAIT
			waitForLock(currpid,ldes1,type, priority);
		/*	pptr->pstate=PRWAIT;
			pptr->lockid = ldes1;
                        pptr->locksState[ldes1] = type;
                        pptr->procwaittime = ctr1000;
		//	updateMaxPrio(ldes1, pptr->pprio);
                        insert(currpid, lptr->lhead, priority);
			lptr->lprio = findMaxPriority(ldes1);
			findProcessWithLock(ldes1);		
			//updateMaxPrio(ldes1,priority, currpid); */
                        resched();
                        restore(ps);
                        return(OK);
	}

	if (locktab[ldes1].lstate == LUSED &&  lptr->ltype == READ){
	/* This means a Reader is using the lock, action is based on the type */
	/* Check if the type is Read 						
	 * If yes, Check if a higher prio writer is waiting 
	 * If yes, move to wait state, insert to lock queue resched and restore and return 
	 * If not, Give the lock to the reader process and free all read process above the highest prio of the reader
	 *
	 */

		if(type==READ)
		{
			if(findHigherPriorityWriter(priority,ldes1))
			{	
				waitForLock(currpid,ldes1,type,priority);
				/*pptr->pstate=PRWAIT;
				pptr->lockid = ldes1;
                       		pptr->locksState[ldes1] = type;
                       		pptr->procwaittime = ctr1000;
                        	insert(currpid, lptr->lhead, priority);
				findProcessWithLock(ldes1);
				lptr->lprio = findMaxPriority(ldes1);*/
                        	resched();
                       		restore(ps);
                        	return(OK);
			}
			else
			{
				kprintf("\nThis case\n");
				lptr->lprio = priority;
        	                lptr->lproc[currpid] = LOCKACQ;

	                        pptr->locksState[ldes1] = type;

                                restore(ps);
                                return(OK);
                        
			}
		}
		 /* If the type is Write
		 *Put into wait mode and call resched
               	  */
		else if(type==WRITE)
		{
			waitForLock(currpid,ldes1,type,priority);
			/*pptr->pstate=PRWAIT;
			pptr->lockid = ldes1;
                        pptr->locksState[ldes1] = type;
                        pptr->procwaittime = ctr1000;
			//updateMaxPrio(ldes1, pptr->pprio);
                        insert(currpid, lptr->lhead, priority);
			findProcessWithLock(ldes1);
			//updateMaxPrio(ldes1, pptr->pprio, currpid);
			lptr->lprio = findMaxPriority(ldes1);*/
                        resched();
                        restore(ps);
                        return(OK);

		}
	}
}

int findHigherPriorityWriter(int priority, int ldes1)
{
	struct lockentry *lptr = &locktab[ldes1];
	 struct pentry *nptr;
	int prev;
	prev = q[lptr->ltail].qprev;

        if (prev < NPROC)
        {
            while(prev<NPROC)
            {
		nptr = &proctab[prev];
    	    	if(nptr->locksState[ldes1] == WRITE && q[prev].qkey > priority)
                	return 1;
                prev =  q[prev].qprev;
            }
	}
	return 0;
}

void waitForLock(int currpid, int ldes1, int type, int priority)
{
         struct  lockentry *lptr = &locktab[ldes1];
         struct pentry *pptr = &proctab[currpid];
	 pptr->pstate=PRWAIT;
         pptr->lockid = ldes1;
         pptr->locksState[ldes1] = type;
         pptr->procwaittime = ctr1000;
         insert(currpid, lptr->lhead, priority);
         findProcessWithLock(ldes1);
         lptr->lprio = findMaxPriority(ldes1);

}
/*
int findLockOwner(ldes1)
{
	struct  lockentry *lptr = &locktab[ldes1];
	if (ldes1 < 0 )
		return SYSERR;
	int i = 0 ;	
	for (i = 0; i < NPROC; i++)
	{
		if(lptr->lproc[i] == LOCKACQ)
			return i;
	}
	locktab[ldes1].lstate = LFREE;
	return -1;
}


*/
