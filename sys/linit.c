#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <q.h>
#include <stdio.h>

struct  lockentry locktab[NLOCKS];
int nr = 0;
void linit()
{
	struct	lockentry *lptr;
	//extern struct  lockentry locktab[NLOCKS];
	nextlock = NLOCKS-1;

	int i,j;	

	kprintf("\nBefore Initializing");

	for (i = 0 ; i<NLOCKS ; i++) {	
		(lptr = &locktab[i])->lstate = LFREE;
		lptr->ltype = EMPTY;
	
		lptr->ltail = 1 + (lptr->lhead = newqueue());
			
		  /* initialize priority */
       		lptr->lprio = 0;
	
	 	for (j = 0 ; j<NPROC ; j++) {
			lptr->lproc[j] = LOCKNOTACQ;
		}
	}
	kprintf("\nAfter Initializing");

/*
	for (i = 0 ; i<NLOCKS ; i++) {
		lptr = &locktab[i];
		for (j = 0 ; j < NPROC ; j++) {
                        if(lptr->lproc[j] > 0)
				kprintf("\n LPTR Check Failed");
                }
	}
*/
}
