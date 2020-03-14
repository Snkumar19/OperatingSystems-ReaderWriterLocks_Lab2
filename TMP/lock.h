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

#define NOTINWQ  -1

#define LAVAILABLE 110
#define LNOTAVAILABLE 111
#define BEFORELCREATE 112

void linit();
SYSCALL lcreate ();
SYSCALL ldelete (int lockdescriptor);
SYSCALL lock (int ldes1, int type, int priority); 

int findMaxPriority(int ldes1);
//int updateMaxPrio(int ldes1, int currpid);
int findPrio(int pid);
void findProcessWithLock(int ldes1);
void lockAcquired(int highestWriterOrReader, int lockdes, int type);
void waitForLock(int currpid, int ldes1, int type, int priority);


#define isbadlock(s)     (s<0 || s>=NLOCKS)
struct lockentry{
	char lstate;
	char ltype;
	int lprio;
	int lhead;
	int ltail;
	int lproc[NPROC];
	int deleteTracker[NPROC];
	//int lwaittime[NPROC];
};

extern struct	lockentry locktab[];
extern	int	nextlock;

#endif
