#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

extern struct  lockentry locktab[NLOCKS];
extern unsigned long ctr1000;
int findLockOwner(ldes1);

int lock (int ldes1, int type, int priority){
	
	STATWORD ps;
	struct  lockentry *lptr = &locktab[ldes1];
	struct pentry *pptr = &proctab[currpid];
	disable(ps);
	int ownerofLock = findLockOwner(ldes1);
	if (ownerofLock == -1)
		ownerofLock = currpid; 
	
	if (locktab[ldes1].lstate == LFREE)
	{
		lptr->lstate = LUSED;
		lptr->ltype = type;
		lptr->lprio = priority;
		lptr->lproc[ownerofLock] = LOCKACQ;

		pptr->locksState[ldes1] = type;
		//kprintf ("\n LOCK LSTATE IS FREE : %d, %d, %d, %d %d\n",ldes1, ownerofLock, lptr->lproc[ownerofLock], pptr->locksState[ldes1],lptr->lprio );
		restore(ps);
		return(OK);
	}
	/* Control Comes here only if the State is not Free */
	if (type == READ)
	{
		//kprintf ("\n LOCK LSTATE IS USED - TYPE READ : %d, %d, %d, %d %d\n",ldes1, ownerofLock, lptr->lproc[ownerofLock], pptr->locksState[ldes1],lptr->lprio );
		
		/* if writer does not have the lock */
		int writerLock = 0;
		int i = 0;

		if (lptr->ltype == WRITE)
		{
			for (i = 0; i < NLOCKS; i++)
        		{
               			 if(lptr->lproc[i] == LOCKACQ)
                       			writerLock = 1;
        		}
	
		}
		
		if (!writerLock){
			lptr->lprio = priority;
			lptr->lproc[currpid] = LOCKACQ;

			pptr->locksState[ldes1] = type;

			//kprintf ("\n LOCK LSTATE IS USED - TYPE READ : %d, %d, %d, %d %d\n",ldes1, currpid, lptr->lproc[currpid], pptr->locksState[ldes1],lptr->lprio );
			restore(ps);
               		 return(OK);

		}
		if(writerLock)
		{
			pptr->pstate=PRWAIT;
			pptr->locksState[ldes1] = WAITONLOCK;
			pptr->procwaittime = ctr1000;
			linsert(currpid, lptr->lhead, priority,type,ldes1);
			resched();
			restore(ps);
                        return(OK);
		}
	}
	 if (type == WRITE)
	{
		//kprintf("\n WRITER.. \n");
		pptr->pstate=PRWAIT;
                pptr->locksState[ldes1] = WAITONLOCK;
		 pptr->procwaittime = ctr1000;
               	linsert(currpid, lptr->lhead, priority,type,ldes1);
                resched();
		 restore(ps);
                 return(OK);
	}

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
