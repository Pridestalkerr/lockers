
#ifndef LOCKERS_LIB_SEMAPHORE
#define LOCKERS_LIB_SEMAPHORE



typedef struct lockers_sem_t
{
	volatile int val_;
}lockers_sem_t;

void lockers_sem_init(struct lockers_sem_t *semaphore, int val);

void lockers_sem_destroy(struct lockers_sem_t *semaphore);

void lockers_sem_wait(struct lockers_sem_t *semaphore);

void lockers_sem_post(struct lockers_sem_t *semaphore, int value);



#endif	//LOCKERS_LIB_SEMAPHORE