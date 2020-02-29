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

	int i = 0;
	for( i = 0; i < NPROC; i++)
	{
		
		pptr = &proctab[i];
		pptr->locksState[lockdescriptor] = DELETED;
	}
	
	if (nonempty(lptr->lhead)) {
		while( (pid=getfirst(lptr->lhead)) != EMPTY)
		  {
		    	proctab[pid].pwaitret = DELETED;
		    ready(pid,RESCHNO);
		  }
		resched();
	}
	restore(ps);
	return(OK);

}
