#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

int ldelete (int lockdescriptor)
{
	STATWORD ps;    
	int	pid;
	struct	lockentry *lptr;
	struct  pentry *pptr;
	
	disable(ps);
	if (isbadlock(lockdescriptor) || locktab[lockdescriptor].lstate==LFREE) {
		restore(ps);
		return(SYSERR);
	}

	lptr = &locktab[lockdescriptor];
	lptr->lstate = LFREE;
	lptr->ltype = EMPTY;
	lptr->lprio = -1;
	lptr->lproc[currpid] = LOCKNOTACQ;

	
	if (nonempty(lptr->lhead)) {
		while( (pid=getfirst(lptr->lhead)) != EMPTY)
		  {
			proctab[pid].locksState[lockdescriptor] = DELETED;
			proctab[pid].lockid = -1;
		    	proctab[pid].pwaitret = DELETED;
		    	ready(pid,RESCHNO);
		  }
		resched();
	}
	restore(ps);
	return(OK);

}
