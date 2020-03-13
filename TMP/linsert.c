/* linsert.c  -  linsert */

#include <conf.h>
#include <kernel.h>
#include <lock.h>
#include <q.h>

/*------------------------------------------------------------------------
 *  linsert.c  --  insert an process into a lock's wait list according to wait priority
 *------------------------------------------------------------------------
 */
extern unsigned long ctr1000;
int linsert(int proc, int head, int key, int type, int ldesc){
	int	next;			/* runs through list		*/
	int	prev;
	
	struct  lockentry *lptr = &locktab[ldesc];
	//lptr->lwaittime[proc] = ctr1000;
		

	next = q[head].qnext;
	while (q[next].qkey < key)	/* tail has maxint as key	*/
		next = q[next].qnext;
	q[proc].qnext = next;
	q[proc].qprev = prev = q[next].qprev;
	q[proc].qkey  = key;
	//q[proc].qltype  = type;
	q[prev].qnext = proc;
	q[next].qprev = proc;
	return(OK);
}
