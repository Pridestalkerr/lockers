
#include <stdlib.h>

//self guard
#ifndef lockers
#define lockers


typedef struct lockers_mutex_t
{
	volatile int *count__;
}lockers_mutex_t;

extern inline void lockers_mutex_init(struct lockers_mutex_t *mutex)
{
	mutex->count__ = (volatile int*) malloc(sizeof(int));
	*(mutex->count__) = 1;
}

extern inline void lockers_mutex_destroy(struct lockers_mutex_t *mutex)
{
	free((void*) mutex->count__);
}

extern inline void lockers_mutex_lock(struct lockers_mutex_t *mutex)
{
	volatile int *count = mutex->count__;
	while(!__sync_val_compare_and_swap(count, 1, 0));
	asm volatile("mfence":::"memory");
}

extern inline void lockers_mutex_unlock(struct lockers_mutex_t *mutex)
{
	asm volatile("mfence":::"memory");
	volatile int *count = mutex->count__;
	__sync_val_compare_and_swap(count, 0, 1);
}


typedef struct lockers_semaphore_t
{

}lockers_semaphore_t;

#endif //lockers guard