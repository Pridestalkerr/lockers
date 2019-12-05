#include "lockers.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

lockers_mutex_t mutex;

void* thr_fun(void *data)
{
	lockers_mutex_lock(&mutex);
	printf("Inside thread...\n");
	//lockers_mutex_unlock(&mutex);

	return NULL;
}

int main(int argc, char **argv)
{
	pthread_t thr;
	lockers_mutex_init(&mutex);

	pthread_create(&thr, NULL, thr_fun, NULL);
	pthread_join(thr, NULL);

	lockers_mutex_lock(&mutex);

	lockers_mutex_destroy(&mutex);
	return 0;
}