
#include <linux/futex.h>
#include <syscall.h>
#include "rwlock.h"
#include "mutex.h"



//we can maybe force only a certain number of readers/writers to grab the cpu before forcing it to switch (if possible, the forcing is kind of 
//	compilcated since we cant really predict when a lock opposite to the curr state will actually happen, aka. what if we check, theres no writers waiting, what do we do?
//	we can leave priority for readers, but what if immediately after we checked a writer wants to lock? concurrency is kinda weird)
void lockers_rwlock_init(struct lockers_rwlock_t *rwlock)
{
	rwlock->rd_count_ = 0;
	lockers_mutex_init(&rwlock->rd_lock_);
	lockers_mutex_init(&rwlock->wr_lock_);
};

void lockers_rwlock_destroy(struct lockers_rwlock_t *rwlock)
{
	rwlock->rd_count_ = 0;
	lockers_mutex_destroy(&rwlock->rd_lock_);
	lockers_mutex_destroy(&rwlock->wr_lock_);
};

void lockers_rwlock_rdlock(struct lockers_rwlock_t *rwlock)
{
	lockers_mutex_lock(&rwlock->rd_lock_);	//lock the readers

	rwlock->rd_count_++;	//increment the readers

	if(rwlock->rd_count_ == 1)
	{
		//first reader in queue will try to switch the mode from write to read

		lockers_mutex_lock(&rwlock->wr_lock_);		//wait until writers have finished		
	}

	lockers_mutex_unlock(&rwlock->rd_lock_);
};

void lockers_rwlock_rdunlock(struct lockers_rwlock_t *rwlock)
{
	lockers_mutex_lock(&rwlock->rd_lock_);	//lock the readers

	rwlock->rd_count_--;	//decrement the readers

	if(rwlock->rd_count_ == 0)
	{
		//there are no more readers in the queue

		lockers_mutex_unlock(&rwlock->wr_lock_);	//let the writers proceed or concur with a new reader		
	}

	lockers_mutex_unlock(&rwlock->rd_lock_);
};

void lockers_rwlock_wrlock(struct lockers_rwlock_t *rwlock)
{
	//try to acquire the write lock lol
	lockers_mutex_lock(&rwlock->wr_lock_);
};

void lockers_rwlock_wrunlock(struct lockers_rwlock_t *rwlock)
{
	//you guessed it... just unlock the writers so that readers/other writers can concur once again
	lockers_mutex_lock(&rwlock->wr_lock_);
};