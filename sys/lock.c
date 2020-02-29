#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

extern struct  lockentry locktab[NLOCKS];
extern unsigned long ctr1000;
int findLockOwner(ldes1);
int findMaxPriority(ldes1);
int updateMaxPrio(int ldes1, int maxprio);

int lock (int ldes1, int type, int priority){
	
	STATWORD ps;
	struct  lockentry *lptr = &locktab[ldes1];
	struct pentry *pptr = &proctab[currpid];
	disable(ps);

	 if (isbadlock(ldes1) || pptr->locksState[ldes1] == DELETED) {
                restore(ps);
                return(SYSERR);
        }

	int ownerofLock = findLockOwner(ldes1);
	kprintf("\nMax prio: %d\n", findMaxPriority(ldes1));
	if (ownerofLock == -1)
		ownerofLock = currpid; 
	
	if (locktab[ldes1].lstate == LFREE)
	{
		pptr->lockid = -1;
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
			pptr->pstate=PRWAIT;
			pptr->lockid = ldes1;
                        pptr->locksState[ldes1] = type;
                        pptr->procwaittime = ctr1000;
			updateMaxPrio(ldes1, pptr->pprio);
                        insert(currpid, lptr->lhead, priority);
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
				pptr->pstate=PRWAIT;
				pptr->lockid = ldes1;
                       		pptr->locksState[ldes1] = type;
                       		pptr->procwaittime = ctr1000;
				updateMaxPrio(ldes1, pptr->pprio);
                        	insert(currpid, lptr->lhead, priority);
                        	resched();
                       		restore(ps);
                        	return(OK);
			}
			else
			{
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
			pptr->pstate=PRWAIT;
			pptr->lockid = ldes1;
                        pptr->locksState[ldes1] = type;
                        pptr->procwaittime = ctr1000;
			updateMaxPrio(ldes1, pptr->pprio);
                        insert(currpid, lptr->lhead, priority);
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

int findMaxPriority(int ldes1)
{
	struct  lockentry *lptr = &locktab[ldes1];
	struct pentry *pptr = &proctab[currpid];
	int maxprio = 0;
	int prev;

	if (nonempty(lptr->lhead))
	{
		prev = q[lptr->ltail].qprev;	
		 while(prev<NPROC)
        	{
			pptr = &proctab[prev];
			if (pptr->pprio > maxprio)
				maxprio = pptr->pprio;
			prev = q[prev].qprev;
		}
	}
	lptr->lprio = maxprio;
	updateMaxPrio(ldes1, maxprio);
}

int updateMaxPrio(int ldes1, int maxprio)
{
	struct pentry *pptr = &proctab[currpid];
	int i = 0;
	for( i =0; i< NPROC; i++)
	{
		pptr = &proctab[i];
		if(pptr->locksState[ldes1] != DELETED && locktab[ldes1].lproc[i] == LOCKACQ)
		{
			if(pptr->pinh < maxprio){
				pptr->pprio = maxprio;
				findMaxPriority(i);
			}
			else
				pptr->pprio = pptr->pinh;
		}
	}
}



