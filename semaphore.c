
#include <linux/futex.h>
#include <syscall.h>
#include <unistd.h>
#include "semaphore.h"



void lockers_sem_init(struct lockers_sem_t *semaphore, int val)
{
	semaphore->val_ = val;
}

void lockers_sem_destroy(struct lockers_sem_t *semaphore)
{
	semaphore->val_ = 0;
}

void lockers_sem_wait(struct lockers_sem_t *semaphore)
{
	volatile int *val = &semaphore->val_;

	while(1)
	{
		//atomic decrement if positive
		int first_access = __sync_add_and_fetch(val, 0);
		int second_access, dec;
		//these should probably get moved outside the loop by the compiler
		for(;;)
		{
			dec = first_access - 1;
			if(dec < 0)
			{
				//no resources available, go to sleep
				syscall(SYS_futex, val, FUTEX_WAIT, first_access, NULL, NULL, 0);	//wait indefinetely

				//either the call failed (a post occured)
				//or we were woken up by a post
				//all we have to do is try to grab the resource again
				break;
			}

			second_access = __sync_val_compare_and_swap(val, first_access, dec);

			if(second_access == first_access)
				return;		//we grabbed a resource, we can return

			//no definitive answer, try again
			first_access = second_access;
		}
	}
}

void lockers_sem_post(struct lockers_sem_t *semaphore, int value)
{
	volatile int *val = &semaphore->val_;

	//all we have to do, is increase the value of the semaphore, atomically
	__sync_fetch_and_add(val, value);

	//after this incrementing, a process stuck in the wait loop or one that failed the futex call
	//might be able to grab the resource without going to sleep

	//we wake up someone, either way, let them compete for the resource along the processes that arent sleeping yet
	syscall(SYS_futex, val, FUTEX_WAKE, value, NULL, NULL, 0);	//wake up someone (should probably look into PI's)
	//would probably better to only wake up current val 
	//actually thinking about it... what happens if multiple posts happen simultaneously?
}