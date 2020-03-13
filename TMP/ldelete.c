#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

SYSCALL ldelete (int lockdescriptor)
{
	STATWORD ps;    
	int	pid, i;
	struct	lockentry *lptr;
	struct  pentry *pptr;
	
	disable(ps);
	if (isbadlock(lockdescriptor) || locktab[lockdescriptor].lstate==LFREE) {
		restore(ps);
		return(SYSERR);
	}

	//kprintf("\n In ldelete\n");

	lptr = &locktab[lockdescriptor];
	lptr->lstate = LFREE;
	lptr->ltype = EMPTY;
	lptr->lprio = -1;
	lptr->lproc[currpid] = LOCKNOTACQ;

	for(i=0;i<NPROC;i++)
	{
		pptr = &proctab[i];
		if(pptr->pstate != PRFREE && lptr->deleteTracker[i] != BEFORELCREATE)
		{	
			lptr->deleteTracker[i] = LNOTAVAILABLE;
			//kprintf("\nDelete check = %d, %d\n",i, lptr->deleteTracker[i] );
		}
	}	

	if (nonempty(lptr->lhead)) {
		while( (pid=getfirst(lptr->lhead)) != EMPTY)
		  {
			//kprintf("\n Delete - wq\n");
			proctab[pid].locksState[lockdescriptor] = DELETED;
			proctab[pid].lockid = NOTINWQ;
		    	proctab[pid].pwaitret = DELETED;
		    	ready(pid,RESCHNO);
		  }
		resched();
	}
	restore(ps);
	return(OK);

}
