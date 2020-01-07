#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "lockers.h"


typedef struct pair_s
{
	lockers_sem_t *sem;
	lockers_mutex_t *mut;
	int id;
	int *count;
}pair_s;

void barrier_point(lockers_sem_t *sem, lockers_mutex_t *mut, int *count)
{
	lockers_mutex_lock(mut);
	(*count)--;
	int tmp = *count; //use a temp variable for the upcoming check
	lockers_mutex_unlock(mut);

	if(tmp == 0)
	{
		int itr;
		/*for(itr = 0; itr < 4; ++itr)
		{
			lockers_sem_post(sem, 1);
		}*/
		lockers_sem_post(sem, 5);
	}
	else
		lockers_sem_wait(sem);
}

void *tfun(void *data)
{
	printf("%d reached barrier\n", ((pair_s*)data)->id);
	barrier_point(((pair_s*)data)->sem, ((pair_s*)data)->mut, ((pair_s*)data)->count);
	printf("%d passed barrier\n", ((pair_s*)data)->id);

	return NULL;
}

int main(int argc, char **argv)
{
	lockers_sem_t sem;
	lockers_sem_init(&sem, 0);
	int total = 5;
	lockers_mutex_t mut;
	lockers_mutex_init(&mut);

	pthread_t thread_pool_s[5];
	struct pair_s data_s[5] = {{&sem, &mut, 0, &total}, {&sem, &mut, 0, &total}, {&sem, &mut, 0, &total}, {&sem, &mut, 0, &total}, {&sem, &mut, 0, &total}};

	int itr;
	for(itr = 0; itr < 5; ++itr)
	{
		data_s[itr].id = itr;
		pthread_create(&thread_pool_s[itr], NULL, tfun, &data_s[itr]);
	}

	for(itr = 0; itr < 5; ++itr)
	{
		pthread_join(thread_pool_s[itr], NULL);
	}

	lockers_sem_destroy(&sem);
	lockers_mutex_destroy(&mut);

	return 0;
}