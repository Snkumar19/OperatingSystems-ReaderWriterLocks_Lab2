#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

LOCAL int newlock();

int lcreate()
{
	STATWORD ps;    
	int ldesc;
	struct lockentry *lptr;
        	
	extern struct  lockentry locktab[NLOCKS];
	disable(ps);
	if ( (ldesc=newlock())==SYSERR ) {
		restore(ps);
		return(SYSERR);
	}
	
	locktab[ldesc].lcount = 1; /* initialize lock count to 1 when created */
        

	restore(ps);
	return(ldesc);
}

LOCAL int newlock()
{
	int ldesc;	
	int i;
	extern struct  lockentry locktab[NLOCKS];
	extern int nextlock;
	for (i=0 ; i<NLOCKS ; i++) {
		ldesc = nextlock--;
		if ( ldesc <= 0){
			nextlock = NLOCKS-1;
		//	kprintf("Hit this case");
		}
		if (locktab[ldesc].lstate == LFREE){
			locktab[ldesc].lstate = LCREATE;
			return(ldesc);
		}
	}
	return(SYSERR);
}
