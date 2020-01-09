
#include <linux/futex.h>
#include <stdatomic.h>
#include <syscall.h>
#include <unistd.h>
#include "mutex.h"

#include <stdio.h>



/*
 *	typedef struct lockers_mutex_t
 *	{
 *		//0 for unlocked
 *		//1 for locked
 *		//2 for contended	
 *		volatile int state_;
 *	}lockers_mutex_t;
 */

/*
 *	What contended is:
 *		Just a state that replaces locked as soon as a process fails the spinlock phase.
 *		All this does is help us avoid employing the kernel as much as possible since
 *		if nobody fails the spinlock then we know for sure that no one is blocking.
 *		Once a lock switches to contended mode, it must remain in an unlocked/contended
 *		mode until every process that failed a spinlock manages to proceed.
 */

int lockers_mutex_init(struct lockers_mutex_t *mutex)
{
	mutex->state_ = 0;

	return 0;
}

int lockers_mutex_destroy(struct lockers_mutex_t *mutex)
{
	volatile int *state = &mutex->state_;

	int expect = 0;
	if(atomic_compare_exchange_strong_explicit(state, &expect, -1, memory_order_relaxed, memory_order_relaxed))
		return 0;
	else
		return 1;
}

void lockers_mutex_lock(struct lockers_mutex_t *mutex)
{
	volatile int *state = &mutex->state_;
	int itr, expect;


fastpath:	//spinlocking

	for(itr = 0; itr < 500; ++itr)
	{
		expect = 0;

		//Returns whether it succeeds, also updates expect to state on failure (which we do not need).
		if(atomic_compare_exchange_strong_explicit(state, &expect, 1, memory_order_acquire, memory_order_acquire))
			return;	//Old state, was the one we expected = 0, unlocked, we're done.

		/*
		 *	Relax the cpu, instruction that improves spinlocking performance.
		 *	Note that this is the correct instruction only for x86 arch (and mips, and others), therefore, this is not portable.
		 *	Different macros exist, Boost even has a cpu_relax header which seems to be pretty good, but thats off topic.
		 *	Here's a list of pre-defined compiler macros:
		 *	https://sourceforge.net/p/predef/wiki/Architectures/
		 */
		asm volatile("pause" ::: "memory");
	}


slowpath:	//blocking

	/*
	 *	We failed to acquire the lock during the spinlocking phase.
	 *	We must, therefore, set the lock's mode to contended.
	 */

	while((expect = atomic_exchange_explicit(state, 2, memory_order_acquire)))
	{
		/*
		 *	If the old state (expect) was anything other than unlocked, try to block the process.
		 *	Pass 2 to as expected value to futex, so that it may unblock in case a signal was missed.
		 */
		syscall(SYS_futex, state, FUTEX_WAIT, 2, NULL, NULL, 0);	//wait indefinitely

		/*
		 *	We were woken up or the syscall failed.
		 *	Try to reacquire the lock.
		 */
	}

	//we manageed to acquire the lock while in contended mode
	//atomic_thread_fence(memory_order_acquire);	//make sure cpu doesn't execute instructions post the actual unlock
}

void lockers_mutex_unlock(struct lockers_mutex_t *mutex)
{
	//atomic_thread_fence(memory_order_release);	//

	volatile int *state = &mutex->state_;
	int itr, expect;

	//The value of the lock should either be 1 or 2, that is, assuming a correct use of the interface.

	if(atomic_exchange_explicit(state, 0, memory_order_release) == 1)
		/*
		 *	Lock was not in contended mode, that is all of the other processes were either spinlocking,
		 *	or finished spinlocking but not yet switched the mode to contended.
		 */
		return;

contended:
	/*
	 *	We unlocked the mutex, but it was in a contended state so we still need to make sure someone grabs it.
	 *	Now, either the lock will be grabbed by a spinning process, or a process that failed the futex call
	 *	(we changed the state from contended to locked in the previous operation), in which case we dont need to do much.
	 *	If all of the processes were already blocked, we must make sure to wake someone up.
	 *	In order to minimize context switches we will first spinlock, and then employ the kernel as a last resource,
	 *	when we really cannot determine whether someone grabbed the lock or not.
	 */

	for(itr = 0; itr < 500; ++itr)
	{
		/*
		 *	If a spinlocking process grabbed the lock then it also reset the mode to locked.
		 *	We must switch it back, as there may still be blocked processes.
		 *	Keep in mind, knowing that someone grabbed the lock isn't enough, There is a recursive relation
	 	 *	that we must consider. That is, if a spinning process grabs the lock, it creates a subset of
	 	 *	processes concurring for the mutex. From their point of view, there are no blocked threads.
	 	 *	The subset breaks and merges with the other processes as soon as something sets the mode back to contended.
	 	 *	That could be, either a new process that failed the spinning, an already existing process that was still awake during
	 	 *	the slowpath, or, in the most basic case, us.
	 	 *	This is why, only if we observe the mutex in a not contended state, can we really be sure
	 	 *	that we do not need to wake up anyone.
		 */
		
		expect = 1;

		if(atomic_compare_exchange_strong_explicit(state, &expect, 2, memory_order_release, memory_order_relaxed))
			return;

		asm volatile("pause" ::: "memory");	//relax the cpu
	}

	/*
	 *	We were unable to determine whether we need to wake up someone or not.	 
	 *	Wake up someone and let them grab the lock, or concur against the others.
	 */
	syscall(SYS_futex, state, FUTEX_WAKE, 1, NULL, NULL, 0);
}