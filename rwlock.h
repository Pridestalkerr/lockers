
#ifndef LOCKERS_LIB_RWLOCK
#define LOCKERS_LIB_RWLOCK



#include "mutex.h"

typedef struct lockers_rwlock_t
{
	volatile int rd_count_;
	struct lockers_mutex_t wr_lock_;
	struct lockers_mutex_t rd_lock_;
}lockers_rwlock_t;

void lockers_rwlock_init(struct lockers_rwlock_t *rwlock);

void lockers_rwlock_destroy(struct lockers_rwlock_t *rwlock);

void lockers_rwlock_rdlock(struct lockers_rwlock_t *rwlock);

void lockers_rwlock_wrlock(struct lockers_rwlock_t *rwlock);

void lockers_rwlock_rdunlock(struct lockers_rwlock_t *rwlock);

void lockers_rwlock_wrunlock(struct lockers_rwlock_t *rwlock);



#endif	//LOCKERS_LIB_RWLOCK