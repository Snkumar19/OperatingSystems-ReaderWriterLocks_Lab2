#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <q.h>
#include <stdio.h>

#define DEFAULT_LOCK_PRIO 20

#define assert(x,error) if(!(x)){ \
            kprintf(error);\
            return;\
            }
int mystrncmp(char* des,char* target,int n){
    int i;
    for (i=0;i<n;i++){
        if (target[i] == '.') continue;
        if (des[i] != target[i]) return 1;
    }
    return 0;
}

/*--------------------------------Test 1--------------------------------*/
 
void reader1 (char *msg, int lck)
{
	lock (lck, READ, DEFAULT_LOCK_PRIO);
	kprintf ("  %s: acquired lock, sleep 2s\n", msg);
	sleep (2);
	kprintf ("  %s: to release lock\n", msg);
	releaseall (1, lck);
}

void test1 ()
{
	int	lck;
	int	pid1;
	int	pid2;

	kprintf("\nTest 1: readers can share the rwlock\n");
	int i = 0;
	//for (i = 0 ; i < NLOCKS; i++) {
	lck  = lcreate ();
	//	kprintf("\n Lck is %d", lck);
	//}
	kprintf ("\n");
	assert (lck != SYSERR, "Test 1 failed");
//	shutdown();

	
	pid1 = create(reader1, 2000, 20, "reader a", 2, "reader a", lck);
	pid2 = create(reader1, 2000, 20, "reader b", 2, "reader b", lck);

	resume(pid1);
	resume(pid2);

	sleep (15);
	ldelete (lck);
/*	
	sleep (5);
//	ldelete (lck);

	struct  lockentry *lptr;
	struct pentry *pptr;
	int j;
	for (i = 0 ; i < NLOCKS; i++) {
		lptr = &locktab[i];
        	pptr = &proctab[i];
		for (j = 0 ; j < NLOCKS; j++){
			if (lptr->lproc[j] != -1)
				kprintf("\nlptr=%d, i = %d\n",lptr->lproc[j],i);
			if (pptr->locksState[j] != 108)
				kprintf("\npptr=%d", pptr->locksState[j]);
		}
	}
*/
	ldelete (lck);
	kprintf ("Test 1 ok\n");
}

/*----------------------------------Test 2---------------------------*/

char output2[10];
int count2;
void reader2 (char msg, int lck, int lprio)
{
        int     ret;

        kprintf ("  %c: to acquire lock\n", msg);
        lock (lck, READ, lprio);
        output2[count2++]=msg;
        kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        sleep (3);
        output2[count2++]=msg;
        kprintf ("  %c: to release lock\n", msg);
	releaseall (1, lck);
}

void writer2 (char msg, int lck, int lprio)
{
	kprintf ("  %c: to acquire lock\n", msg);
        lock (lck, WRITE, lprio);
        output2[count2++]=msg;
        kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        sleep (3);
        output2[count2++]=msg;
        kprintf ("  %c: to release lock.....................\n", msg);
        releaseall (2, lck, lck);
}

void test2 ()
{
        count2 = 0;
        int     lck;
        int     rd1, rd2, rd3, rd4;
        int     wr1;

        kprintf("\nTest 2: wait on locks with priority. Expected order of"
		" lock acquisition is: reader A, reader B, reader D, writer C & reader E\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Test 2 failed");

	rd1 = create(reader2, 2000, 20, "reader2", 3, 'A', lck, 20);
	rd2 = create(reader2, 2000, 20, "reader2", 3, 'B', lck, 30);
	rd3 = create(reader2, 2000, 20, "reader2", 3, 'D', lck, 25);
	rd4 = create(reader2, 2000, 20, "reader2", 3, 'E', lck, 20);
        wr1 = create(writer2, 2000, 20, "writer2", 3, 'C', lck, 28);
	
        kprintf("-start reader A, then sleep 1s. lock granted to reader A\n");
        resume(rd1);
        sleep (1);

        kprintf("-start writer C, then sleep 1s. writer waits for the lock\n");
        resume(wr1);
        sleep10 (1);


        kprintf("-start reader B, D, E. reader B is granted lock.\n");
        resume (rd2);
	resume (rd3);
	resume (rd4);
	int prev;

	struct  lockentry *lptr = &locktab[lck];
	prev = q[lptr->ltail].qprev;
	/*  while(prev<NPROC)
          {
               kprintf("\nQ: %d", q[prev].qkey);
		kprintf("\nType: %d", lptr->lproc[q[prev].qkey]);
                prev =  q[prev].qprev;
           }
	*/
        sleep (15);
        kprintf("output=%s\n", output2);
        assert(mystrncmp(output2,"ABABCCDEED",10)==0,"Test 2 FAILED\n");
        kprintf ("Test 2 OK\n");
}

/*----------------------------------Test 3---------------------------*/

void reader3 (char *msg, int lck)
{
        int     ret;

        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock\n", msg);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void writer3 (char *msg, int lck)
{
        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock, sleep 10s\n", msg);
        sleep (10);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void test3 ()
{
        int     lck;
        int     rd1, rd2;
        int     wr1;

        kprintf("\nTest 3: test the basic priority inheritence\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Test 3 failed");

        rd1 = create(reader3, 2000, 25, "reader3", 2, "reader A", lck);
        rd2 = create(reader3, 2000, 30, "reader3", 2, "reader B", lck);
        wr1 = create(writer3, 2000, 20, "writer3", 2, "writer", lck);

        kprintf("-start writer, then sleep 1s. lock granted to write (prio 20)\n");
        resume(wr1);
        sleep (1);

        kprintf("-start reader A, then sleep 1s. reader A(prio 25) blocked on the lock\n");
        resume(rd1);
        sleep (1);
	assert (getprio(wr1) == 25, "Test 3 failed...................");

        kprintf("-start reader B, then sleep 1s. reader B(prio 30) blocked on the lock\n");
        resume (rd2);
	sleep (1);
	assert (getprio(wr1) == 30, "Test 3 failed");
	
	kprintf("-kill reader B, then sleep 1s\n");
	kill (rd2);
	sleep (1);
	assert (getprio(wr1) == 25, "Test 3 failed");

	kprintf("-kill reader A, then sleep 1s\n");
	kill (rd1);
	sleep(1);
	assert(getprio(wr1) == 20, "Test 3 failed");

        sleep (8);
        kprintf ("Test 3 OK\n");
}


/*----------------------------------Test 4---------------------------*/
char output4[10];
int count4;
int lck1, lck2;
void reader4 (char msg, int lck, int lprio)
{
        int     ret;

        kprintf ("  %c: to acquire lock\n", msg);
        lock (lck, READ, lprio);
        output4[count4++]=msg;
        kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        sleep (3);
        output4[count4++]=msg;
        kprintf ("  %c: to release lock\n", msg);
	releaseall (1, lck);
}

void writer41 (char msg, int lck, int lprio)
{
	kprintf ("  %c: to acquire lock\n", msg);
        lock (lck1, WRITE, lprio);
        output4[count4++]=msg;
        kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        sleep (3);
        output4[count4++]=msg;
	lock (lck2, WRITE, lprio);
	kprintf ("  %c: acquired lock2, sleep 3s\n", msg);
	sleep (3);
        kprintf ("  %c: to release lock\n", msg);
        releaseall (2, lck1, lck2);
}
void writer42 (char msg, int lck, int lprio)
{
        kprintf ("  %c: to acquire lock\n", msg);
        lock (lck2, WRITE, lprio);
        output4[count4++]=msg;
        kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        sleep (5);
        output4[count4++]=msg;
        kprintf ("  %c: to release lock\n", msg);
        releaseall (1, lck2);
}
void writer43 (char msg, int lck, int lprio)
{
        kprintf ("  %c: to acquire lock\n", msg);
        lock (lck1, WRITE, lprio);
        output4[count4++]=msg;
        kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        sleep (3);
        output4[count4++]=msg;
        kprintf ("  %c: to release lock\n", msg);
        releaseall (1, lck1);
}


void test4 ()
{
        count4 = 0;
        int     lck;
        int     rd1, rd2, rd3, rd4;
        int     wr1,wr2,wr3,wr4;

        kprintf("\nTest 4: wait on locks with priority. Expected order of"
		" lock acquisition is: reader A, reader B, reader D, writer C & reader E\n");
        lck1  = lcreate ();
	lck2  = lcreate ();	
        //assert (lck != SYSERR, "Test 2 failed");

	//rd1 = create(reader4, 2000, 30, "reader2", 3, 'A', lck, 20);
	//rd2 = create(reader4, 2000, 32, "reader2", 3, 'B', lck, 30);
	//rd3 = create(reader4, 2000, 44, "reader2", 3, 'D', lck, 44);
	//rd4 = create(reader4, 2000, 45, "reader2", 3, 'E', lck, 45);
        wr1 = create(writer41, 2000, 40, "writer2", 3, 'A', lck, 32);
	wr2 = create(writer42, 2000, 42, "writer2", 3, 'B', lck, 32);
	wr3 = create(writer43, 2000, 44, "writer2", 3, 'C', lck, 32);
	//wr4 = create(writer4, 2000, 48, "writer2", 3, 'F', lck, 32);
	
        kprintf("-start reader A, then sleep 1s. lock granted to reader A\n");
        resume(wr1);
        sleep (1);
	
	kprintf("-start reader B. reader B is granted lock.\n");
        resume (wr2);
	sleep (1);

        kprintf("-start writer C, then sleep 1s. writer waits for the lock\n");
        resume(wr3);
        //sleep (1);




        sleep (15);
        kprintf("output=%s\n", output4);
        assert(mystrncmp(output4,"ABABCCDEED",10)==0,"Test 4 FAILED\n");
        kprintf ("Test 4 OK\n");
}


int main( )
{
        /* These test cases are only used for test purposes.
 *          * The provided results do not guarantee your correctness.
 *                   * You need to read the PA2 instruction carefully.
                            */
	test1();
	test2();
	test3();
	task1();
	test4();

        /* The hook to shutdown QEMU for process-like execution of XINU.
 *          * This API call exists the QEMU process.
 *                   */
        shutdown();
}



