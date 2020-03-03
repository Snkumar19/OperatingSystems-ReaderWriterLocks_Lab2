#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

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

	for (j=0; j<NPROC; j++)
        {
		pptr = &proctab[j];
        	if(lptr->lproc[j]==LOCKACQ)
		{
			kprintf("\n JJJJJJJJJJJJJJJJJJJJJJJ PID = %d\n", j);
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
			
			kprintf("\n MPRIO = %d", mprio);
        		if(mprio > 0 && mprio > pptr->pprio)
               			pptr->pinh = mprio;
        		else
               			pptr->pinh = 0;

			kprintf("\n pid = %d", j);
		        kprintf("\npptr->pinh=%d",pptr->pinh);
        		kprintf("\nCURRPID = %d", currpid);
        		if (pptr->lockid == NOTINWQ)
                		return;
        		else
        		{
                		lptr = &locktab[pptr->lockid];
                		lptr->lprio = findMaxPriority(pptr->lockid);
				kprintf("\n LAST PART - lptr->lprio = %d", lptr->lprio);
                             	findProcessWithLock(pptr->lockid);
        		}	
        	}
	}

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
                        if(pptr->pinh > pptr->pprio && pptr->pinh > maxprio)
                                maxprio = pptr->pinh;
                        else if (pptr->pprio > maxprio)
                                maxprio = pptr->pprio;
                        prev = q[prev].qprev;
                }
        }
        //kprintf("MAX PRIO = %d\n", maxprio);
       // lptr->lprio = maxprio;
        /*updateMaxPrio(ldes1, maxprio);*/
        return maxprio;
}


