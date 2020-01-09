
#ifndef LOCKERS_LIB_MUTEX
#define LOCKERS_LIB_MUTEX



typedef struct lockers_mutex_t
{
	//0 for unlocked
	//1 for locked
	//2 for locked and contended
	volatile int state_;
}lockers_mutex_t;	//this might as well just be a define

int lockers_mutex_init(struct lockers_mutex_t *mutex);

int lockers_mutex_destroy(struct lockers_mutex_t *mutex);

void lockers_mutex_lock(struct lockers_mutex_t *mutex);

void lockers_mutex_unlock(struct lockers_mutex_t *mutex);



#endif	//LOCKERS_LIB_MUTEX