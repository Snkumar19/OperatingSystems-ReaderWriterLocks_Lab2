#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

/* Return pinh if set, otherwise returns prio*/
int findPrio(int pid)
{
	struct pentry *pptr = &proctab[pid];
	if(pptr->pinh != 0)
		return pptr->pinh;
	return pptr->pprio;
}

void findProcessWithLock(int ldes1)
{
	struct lockentry *lptr = &locktab[ldes1];
	struct pentry *pptr;

        int i = 0, j = 0 , mprio = 0, lockWithMaxPrio, lockToBeUpdated;

	/* Get all the process holding this lock*/
	for (j=0; j<NPROC; j++)
        {
		pptr = &proctab[j];
		/* check if holding*/
        	if(lptr->lproc[j]==LOCKACQ)
		{
			//kprintf("\n JJJJJJJJJJJJJJJJJJJJJJJ PID = %d\n", j);
			/*For every lock this process is associated with, get the max prio from all WQ
 			* mprio - hold the max value - 1a
 			* */
 			for(i = 0; i < NLOCKS; i++)
			{
                		lptr = &locktab[i];
				//kprintf("\n lptr->lprio = %d", lptr->lprio);

                		lptr->lprio = findMaxPriority(i);
                		if((pptr->locksState[i] == READ || pptr->locksState[i] == WRITE) && (locktab[i].lstate == LUSED))
                		{
                        		if (lptr->lprio > mprio)
					{
                                		mprio = lptr->lprio;
                                		lockWithMaxPrio = i;
                        		}
                		}	
        		}
			
			//kprintf("\n MPRIO = %d", mprio);
			/*Check if mprio is updated, if yes - update pinh, otherwise set it to 0*/
        		if(mprio > 0 && mprio > pptr->pprio)
               			pptr->pinh = mprio;
        		else
               			pptr->pinh = 0;

			//kprintf("\n pid = %d", j);
		        //kprintf("\npptr->pinh=%d",pptr->pinh);
        		//kprintf("\nCURRPID = %d", currpid);

			/* Update the pinh value - if this process is in WQ & call the same function for all the processes associated with this lock till WQ is empty*/
        		if (pptr->lockid == NOTINWQ)
                		return;
                   	findProcessWithLock(pptr->lockid);	
        	}
	}

}

/*return max prio from WQ*/
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
                        if(pptr->pinh > pptr->pprio && pptr->pinh > maxprio)
                                maxprio = pptr->pinh;
                        else if (pptr->pprio > maxprio)
                                maxprio = pptr->pprio;
                        prev = q[prev].qprev;
                }
        }
        //kprintf("MAX PRIO = %d\n", maxprio);
       // lptr->lprio = maxprio;
        return maxprio;
}


