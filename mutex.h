
#ifndef LOCKERS_LIB_MUTEX
#define LOCKERS_LIB_MUTEX



typedef struct lockers_mutex_t
{
	volatile int val_;
}lockers_mutex_t;

void lockers_mutex_init(struct lockers_mutex_t *mutex);

void lockers_mutex_destroy(struct lockers_mutex_t *mutex);

void lockers_mutex_lock(struct lockers_mutex_t *mutex);

void lockers_mutex_unlock(struct lockers_mutex_t *mutex);



#endif	//LOCKERS_LIB_MUTEX