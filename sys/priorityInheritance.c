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
        kprintf("MAX PRIO = %d\n", maxprio);
       // lptr->lprio = maxprio;
        /*updateMaxPrio(ldes1, maxprio);*/
        return maxprio;
}

int updateMaxPrio(int ldes1, int maxprio, int pid)
{
        kprintf("TEST-----------------------------\n");
        struct  lockentry *lptr;
        int prev;
        struct pentry *pptr = &proctab[pid];
        int i = 0, mprio = 0, flag = 0;
	int lockWithMaxPrio, lockToBeUpdated;


	for(i = 0; i < NLOCKS; i++)
	{
		lptr = &locktab[i];
		lptr->lprio = findMaxPriority(i);
		kprintf("\n lptr->lprio = %d", lptr->lprio);
	}

	for(i = 0; i < NLOCKS; i++)
        {
		lptr = &locktab[i];
		if((pptr->locksState[i] == READ || pptr->locksState[i] == WRITE) && (locktab[i].lstate == LUSED))
		{
			if (lptr->lprio > mprio)
			{	
				mprio = lptr->lprio;
				lockWithMaxPrio = i;
			}
		}
	}

	if(mprio > 0)
	{
		if (mprio > pptr->pprio)
			pptr->pinh = mprio;
		else
			pptr->pinh = 0;
	}
	else
		pptr->pinh = 0;
                        

	kprintf("\npptr->pinh=%d",pptr->pinh);
	if (pptr->procwaittime > 0)
	{	
		lptr = &locktab[pptr->lockid];
		lptr->lprio = findMaxPriority(pptr->lockid);


		for (i =0;i< NPROC; i++)
		{
			if(lptr->lproc[i] == LOCKACQ)
				updateMaxPrio(ldes1, maxprio, i);
		}
        }
}

