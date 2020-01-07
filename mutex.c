
#include <linux/futex.h>
#include <syscall.h>
#include <unistd.h>
#include "mutex.h"



/*

The following are true assuming a correct use of the primitive on the user's part.

Case 1:	p1 attempts to lock the mutex, and succeeds thanks to the atomic xchg

Case 2: p1 attempts to lock the mutex, xchg fails therefore p1 starts to spinlock while performing the xchg, and during an iteration, it succeeds

Case 3: p1 attempts to lock the mutex, none of the above work, p1 attempts a futex call:
	a)	call fails (p2 unlocked the mutex by increasing the value, as well as trying to wake up p1, which wasnt asleep anyway), p1 reattempts above steps
	b)	call succeeds, p1 will now sleep indefinetely, until it gets woken up by p2 whose unlocked the mutex, p1 reattempts above steps

*/

void lockers_mutex_init(struct lockers_mutex_t *mutex)
{
	mutex->val_ = 1;
}

void lockers_mutex_destroy(struct lockers_mutex_t *mutex)
{
	mutex->val_ = 0;	//this doesnt do anything lol
}

void lockers_mutex_lock(struct lockers_mutex_t *mutex)
{
	volatile int *val = &mutex->val_;

	while(1)
	{
		if(__sync_val_compare_and_swap(val, 1, 0))
		{
			//val was 1 and we acquired it
			return;
		}
		else
		{
			//val was 0
			//spinlock here
			int itr;
			for(itr = 0; itr < 500; ++itr)
			{
				asm volatile("pause" ::: "memory");	//relax the cpu
				//check again
				if(__sync_val_compare_and_swap(val, 1, 0))
				{
					//we acquired the lock during spinlock phase
					return;
				}
			}

			//we failed at acquiring the lock during spinlock phase
			//put the process to sleep
			syscall(SYS_futex, val, FUTEX_WAIT, 0, NULL, NULL, 0);	//wait indefinetely

			//we were awoken or the syscall failed
			//try to reacquire the lock, should we spinlock again?
		}
	}
}

void lockers_mutex_unlock(struct lockers_mutex_t *mutex)
{
	volatile int *val = &mutex->val_;

	__sync_val_compare_and_swap(val, 0, 1);
	//we increased the futex, some other kernel path/spinlock may enter here and acquire the lock, thus not putting the process to sleep

	//but in case everyone is currently sleeping, we force wake someone (if someone already acquired the lock and we wake someone up 
	//theyll try to acquire the lock, and possibly fail, going to sleep once again)
	syscall(SYS_futex, val, FUTEX_WAKE, 1, NULL, NULL, 0);
}