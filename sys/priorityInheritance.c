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
        lptr->lprio = maxprio;
        /*updateMaxPrio(ldes1, maxprio);*/
        return maxprio;
}

int updateMaxPrio(int ldes1, int maxprio, int pid)
{
        kprintf("TEST-----------------------------\n");
        struct  lockentry *lptr;
        int prev;
        struct pentry *pptr = &proctab[pid];
        int i = 0, mprio = pptr->pprio, flag = 0;
                        
        kprintf("\nlocktab[i].lproc[currpid] = %d", locktab[ldes1].lproc[currpid]);
        kprintf("\npptr->locksState[i] = %d", pptr->locksState[ldes1]);
        
        if((pptr->locksState[ldes1] == READ || pptr->locksState[ldes1] == WRITE) && locktab[ldes1].lstate == LUSED)
        {
                kprintf("INSIDE UPDATE - MAX PRIO = %d\n", findMaxPriority(ldes1));
                
                if(findMaxPriority(ldes1))
                {
                        if (findMaxPriority(ldes1) > pptr->pprio)
                                mprio = findMaxPriority(ldes1);
                }
                kprintf("\nmprio = %d", mprio);
        }

        
        if(mprio > pptr->pprio)
 {
                kprintf("\npptr->pprio= %d",pptr->pprio );
                pptr->pinh = mprio;

                for (i =0; i< NLOCKS; i++)
                {
                        lptr = &locktab[i];
                        if (nonempty(lptr->lhead))
                        {
                                prev = q[lptr->ltail].qprev;
                                  while(prev<NPROC)
                                {
                                        if (pid == prev){
                                                flag = 1;
                                                break;
                                        }
                                        prev = q[prev].qprev;
                                }
                                if (flag)
                                        break;

                        }
                }
                kprintf("\npinh = %d",pptr->pinh);
                if(flag)
                        updateMaxPrio(i, maxprio, prev);
        }
}

