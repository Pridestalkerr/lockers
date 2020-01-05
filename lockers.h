
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <linux/futex.h>
#include <syscall.h>


//self guard
#ifndef lockers
#define lockers

#define CUSTOM_FUTEX_OP 38162332

typedef struct lockers_mutex_t
{
	volatile int *val_;
}lockers_mutex_t;

//remove pointers...
void lockers_mutex_init(struct lockers_mutex_t *mutex)
{
	mutex->val_ = (volatile int*) malloc(sizeof(int));
	*(mutex->val_) = 1;
}

void lockers_mutex_destroy(struct lockers_mutex_t *mutex)
{
	free((void*) mutex->val_);
}

void lockers_mutex_lock(struct lockers_mutex_t *mutex)
{
	volatile int *val = mutex->val_;
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
			//try to reacquire the lock
		}
	}
}

void lockers_mutex_unlock(struct lockers_mutex_t *mutex)
{
	volatile int *val = mutex->val_;

	__sync_val_compare_and_swap(val, 0, 1);
	//we increased the futex, some other kernel path/spinlock may enter here and acquire the lock, thus not putting the process to sleep

	//but in case everyone is currently sleeping, we force wake someone (if someone already acquired the lock and we wake someone up 
	//theyll try to acquire the lock, and possibly fail, going to sleep once again)
	syscall(SYS_futex, val, FUTEX_WAKE, 1, NULL, NULL, 0);
}


typedef struct lockers_semaphore_t
{
	volatile int *val_;
	struct lockers_mutex_t *mutex_;
}lockers_semaphore_t;

void lockers_semaphore_init(struct lockers_semaphore_t *semaphore, int val)
{
	semaphore->val_ = (volatile int*) malloc(sizeof(int));
	*(semaphore->val_) = val;

	lockers_mutex_init(semaphore->mutex_);
}

void lockers_semaphore_destroy(struct lockers_semaphore_t *semaphore)
{
	free((void*) semaphore->val_);
	lockers_mutex_destroy(semaphore->mutex_);
}

void lockers_semaphore_wait(struct lockers_semaphore_t *semaphore)
{
	volatile int *val = semaphore->val_;

	while(1)
	{
		//atomic decrement if positive
		int first_access = __sync_add_and_fetch(val, 0);
		int second_access, dec;
		//these should probably get moved outside the loop by the compiler
		for(;;)
		{
			//this is a bit intensive on the cpu, maybe theres a better way? introducing a mutex is an option
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

void lockers_semaphore_post(struct lockers_semaphore_t *semaphore, int value)
{
	volatile int *val = semaphore->val_;

	//all we have to do, is increase the value of the semaphore, atomically
	__sync_fetch_and_add(val, value);

	//after this incrementing, a process stuck in the wait loop or one that failed the futex call
	//might be able to grab the resource without going to sleep

	//we wake up someone, either way, let them compete for the resource along the processes that arent sleeping yet
	syscall(SYS_futex, val, FUTEX_WAKE, value, NULL, NULL, 0);	//wake up someone (should probably look into PI's)
	//would probably better to only wake up current val 
	//actually thinking about it... what happens if multiple posts happen simultaneously?
}


typedef struct lockers_rwlock_t
{
	volatile int val_;		//0 unlocked, 1 for read, -1 for write
	volatile int wr_word;	//writers go to sleep on this variable
	volatile int rd_word;	//readers go to sleep on this variable
}lockers_rwlock_t;

void lockers_rwlock_init(struct lockers_rwlock_t *rwlock)
{
	rwlock->val_ = 0;
}

void lockers_rwlock_destroy(struct lockers_rwlock_t *rwlock)
{

}

void lockers_rwlock_rdlock(struct lockers_rwlock_t *rwlock)
{
	//if val is 0 or 1 then we can proceed, otherwise we must wait
	
}

void lockers_rwlock_wrlock(struct lockers_rwlock_t *rwlock)
{

}


/*

unlocks should only wake up 1 path, 

*/
void lockers_rwlock_rdunlock(struct lockers_rwlock_t *rwlock)
{

}

void lockers_rwlock_wrunlock(struct lockers_rwlock_t *rwlock)
{
	//after a writer has finished, should it wake up readers or another writer?
}


#endif //lockers guard