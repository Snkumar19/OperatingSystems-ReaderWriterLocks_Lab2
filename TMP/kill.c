/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include<lock.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev;

	int i = 0;
	int ldes;
	struct  lockentry *lptr;

	disable(ps);

	//kprintf("\n IN KILL, state = %c\n", pptr->pstate);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;
			ldes = pptr->lockid;
			//kprintf("\n Kill check = %d \n",ldes);
			lptr = &locktab[ldes];

			 pid = dequeue(pid);
		        lptr->lprio = findMaxPriority(ldes);
        		proctab[pid].lockid = NOTINWQ;
       			 proctab[pid].locksState[ldes] = DELETED;
		        proctab[pid].procwaittime = 0;
			findProcessWithLock(ldes);
			/*for (i=0; i<NPROC; i++)
                        {
				//kprintf("\nProcess with this lock = %d- %d\n",i, lptr->lproc[i]);
                                if(lptr->lproc[i]==LOCKACQ){
					kprintf("\nProcess holding this lock = %d\n",i);
                                        updateMaxPrio(ldes,i);
				}
                        }*/


	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}
