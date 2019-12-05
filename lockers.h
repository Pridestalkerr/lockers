
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

//self guard
#ifndef lockers
#define lockers


typedef struct lockers_mutex_t
{
	volatile int *count__;
}lockers_mutex_t;

void lockers_mutex_init(struct lockers_mutex_t *mutex)
{
	mutex->count__ = (volatile int*) malloc(sizeof(int));
	*(mutex->count__) = 1;
}

void lockers_mutex_destroy(struct lockers_mutex_t *mutex)
{
	free((void*) mutex->count__);
}

void lockers_mutex_lock(struct lockers_mutex_t *mutex)
{
	asm volatile("mfence" ::: "memory");
	volatile int *count = mutex->count__;
	while(!__sync_val_compare_and_swap(count, 1, 0))
	{
		/*while(!*count)
		{
			_mm_pause();
		}*/
		sleep(1);
		printf("not yet...%d\n", getpid());
	}
	asm volatile("mfence" ::: "memory");
}

void lockers_mutex_unlock(struct lockers_mutex_t *mutex)
{
	asm volatile("mfence" ::: "memory");
	volatile int *count = mutex->count__;
	__sync_val_compare_and_swap(count, 0, 1);
	asm volatile("mfence" ::: "memory");
}


typedef struct lockers_semaphore_t
{

}lockers_semaphore_t;

#endif //lockers guard