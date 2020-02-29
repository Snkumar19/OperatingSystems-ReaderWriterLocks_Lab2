#ifndef _LOCK_H_
#define _LOCK_H_

#ifndef	NLOCKS
#define NLOCKS	50 	/* number of locks */
#endif

#ifndef DELETED  
#define LDELETED	105		/* lock is deleted*/
#endif

#define LCREATE		100
#define LFREE		101		/* lock is free	*/
#define	LUSED		102		/* lock acquired*/

#define READ		103 	        /* Read lock */
#define WRITE 		104      	/* Write lock*/

#define NOSTATE		108

#define LOCKACQ 	1
#define LOCKNOTACQ	-1

void linit();
int lcreate ();
int ldelete (int lockdescriptor);
int lock (int ldes1, int type, int priority); 

#define isbadlock(s)     (s<0 || s>=NLOCKS)
struct lockentry{
	char lstate;
	char ltype;
	int lprio;
	int lhead;
	int ltail;
	int lcount;
	int lproc[NPROC];
	//int lwaittime[NPROC];
};

extern struct	lockentry locktab[];
extern	int	nextlock;
extern  int     nr;

#endif
