#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <sem.h>
#include <stdio.h>

#define DEFAULT_LOCK_PRIO 20
	


void lockWriter (char *msg, int lck, int control)
{
        int i = 0 , j = 0 ;
	if (control == 3) {
		kprintf ("  %s: to acquire lock\n", msg);
        	lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        	kprintf ("  %s: acquired lock,\n", msg);
        	kprintf ("  %s: to release lock\n", msg);
        	releaseall (1, lck);
	}

	if (control == 2) {
		kprintf ("  %s: to acquire lock\n", msg);
                lock (lck, WRITE, DEFAULT_LOCK_PRIO);
                kprintf ("  %s: acquired lock,Sleep for 2 seconds\n", msg);
		sleep(2);
		kprintf ("  %s: to release lock\n", msg);
                releaseall (1, lck);

	}

	if (control == 1){

		kprintf ("  %s: sleep 1 seconds \n", msg);
		int i = 0;
		for ( i = 0; i < 3000; i++);
		sleep(1); 
		kprintf ("  %s: ends \n", msg);
        }
}


void testLock()
{

	int     lck;
        int     one, two, three;

        kprintf("\n TEST : PRIORITY INHERITANCE R/W LOCKS \n");
        lck  = lcreate ();

	three = create(lockWriter, 2000, 50, "Writer", 3, "H", lck, 3);
        two = create(lockWriter, 2000, 30, "Writer", 3, "L", lck, 2);
        one = create(lockWriter, 2000, 40, "Reader", 3, "M", lck, 1);

	kprintf("Start L...\n");
        resume(two);
        sleep (1);

	kprintf("Start H...\n");
        resume (three);


	 kprintf("Start M...\n");
        resume (one);

	sleep (10);
	
}

void lockSem (char *msg, int sem, int control)
{
        int i = 0 , j = 0 ;
        if (control == 3) {
                kprintf ("  %s: to acquire semaphore\n", msg);
                wait (sem);
                kprintf ("  %s: acquired sem\n", msg);
                kprintf ("  %s: to release sem \n", msg);
                signal (sem) ; 
        }

        if (control == 2) {
                kprintf ("  %s: to acquire sem \n", msg);
                wait (sem);
                kprintf ("  %s: acquired sem,Sleep for 2 seconds\n", msg);
                sleep(2);
                kprintf ("  %s: to release lock\n", msg);
		signal (sem);
        }

        if (control == 1){

                kprintf ("  %s:, sleep 1 seconds \n", msg);
		int i = 0;
                for ( i = 0; i < 3000; i++);
                sleep(1);
                kprintf ("  %s:  ends \n", msg);
        }
}

void testSemaphore()
{

        int     lck;
        int     one, two, three;

        kprintf("\n TEST : SEMAPHORE WITH PRIORITY INVERSION \n");
        int sem  = screate (1);

        three = create(lockSem, 2000, 50, "Writer", 3, "H", sem, 3);
        two = create(lockSem, 2000, 30, "Writer", 3, "L", sem, 2);
        one = create(lockSem, 2000, 40, "Reader", 3, "M", sem, 1);

        kprintf("Start L - Semaphore... \n");
        resume(two);
        sleep (1);

        kprintf("Start H - Semaphore...\n");
        resume (three);


         kprintf("Start M - Semaphore...\n");
        resume (one);

        sleep (10);

}



void task1( )
{
	testLock();
	testSemaphore();

        //shutdown();
}


