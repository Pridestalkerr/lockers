

/*

All right, make up a futex with the following available params
	u32 __user *uaddr, int futex_op = :(, u32 val, ktime_t *timeout, u32 __user *uaddr2, u32 val3	//for 4.16
	struct pt_reg	//for 4.17

*/

//maybe limit the kernels work to just blocking?

long futex_wait(u32 __user *uaddr, u32 val)
{
	//in case *uaddr was already decremented during the call 
	/*if(*uaddr != val)
		return -1;	//spurious wake up (what the hell is that though...)

	freezable_schedule();*/

	asm volatile("mfence" ::: "memory");
	while(!atomic_cmpxchg(uaddr, 1, 0))
	{
		cpu_relax();
		//printf("not yet...%d\n", getpid());
	}
	asm volatile("mfence" ::: "memory");

	return 0;
}

long futex_wake(u32 __user *uaddr, u32 val)
{
	asm volatile("mfence" ::: "memory");
	atomic_cmpxchg(uaddr, 0, 1);
	asm volatile("mfence" ::: "memory");

	return 0;
}


