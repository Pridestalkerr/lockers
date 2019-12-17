#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/futex.h>
#include <linux/version.h>

#include <linux/highmem.h>
#include <asm/unistd.h>


unsigned long long *sys_call_table;

/*
SYSCALL_DEFINE6(futex, u32 __user *, uaddr, int, op, u32, val,
				struct __kernel_timespec __user *, utime, u32 __user *, uaddr2,
				u32, val3)
*/

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0)

asmlinkage long (*real_futex) (const struct pt_regs *);

asmlinkage long fake_futex(const struct pt_regs *regs)
{
	u32 __user *uaddr 							= (u32 __user*)							regs->di;
	int futex_op								= 										regs->si;
	u32 val										= 										regs->dx;
	struct __kernel_timespec __user *timeout	= (struct __kernel_timespec __user*)	regs->r10;
	u32 __user *uaddr2							= (u32 __user*)							regs->r8;
	u32 val3									= 										regs->r9;

	if(futex_op == 38162332)
		printk(KERN_INFO "Intercepting call to futex with futex_op = %d, forwarding...\n", futex_op);

	return real_futex(regs);
}

#else

asmlinkage long (*real_futex) (u32 __user*, int, u32, struct __kernel_timespec __user*, u32 __user*, u32);

asmlinkage long fake_futex(u32 __user *uaddr, int futex_op, u32 val, struct __kernel_timespec __user *timeout, u32 __user *uaddr2, u32 val3)
{
	if(futex_op == 38162332)
		printk(KERN_INFO "Intercepting call to futex with futex_op = %d, forwarding...\n", futex_op);

	return real_futex(uaddr, futex_op, val, timeout, uaddr2, val3);
}

#endif


//make the memory page writable
int make_rw(unsigned long long address)
{
	unsigned int level;
	pte_t *pte = lookup_address(address, &level);

	if(pte->pte & ~_PAGE_RW)
		pte->pte |= _PAGE_RW;

	return 0;	
}

//make the memory page read only
int make_ro(unsigned long long address)
{
	unsigned int level;
	pte_t *pte = lookup_address(address, &level);

	pte->pte &= ~_PAGE_RW;

	return 0;
}


static int __init init_hook(void)
{
	printk(KERN_INFO "Attempting to install hook.\n");

	sys_call_table = (unsigned long long*) 0xffffffffbe200260;		//sudo cat /proc/kallsyms | grep sys_call_table
	real_futex = (void*) sys_call_table[__NR_futex];				//save the address of the real futex

	make_rw((unsigned long long) sys_call_table);					//make page writable

	sys_call_table[__NR_futex] = (unsigned long long) fake_futex;	//patch the table

	make_ro((unsigned long long) sys_call_table);					//restore privileges

	return 0;	//no error
}

static void __exit clean_hook(void)
{
	printk(KERN_INFO "Uninstalling hook.\n");

	make_rw((unsigned long long) sys_call_table);					//make page writable

	sys_call_table[__NR_futex] = (unsigned long long) real_futex;	//restore the table

	make_ro((unsigned long long) sys_call_table);					//restore privileges
}


module_init(init_hook);
module_exit(clean_hook);
MODULE_LICENSE("GPL");