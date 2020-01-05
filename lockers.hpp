#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <linux/futex.h>
#include <syscall.h>

namespace lkr
{


class Mutex
{
	volatile int val_;

public:

	Mutex();

	~Mutex();

	void lock();

	void unlock();

}

Mutex::Mutex()


}