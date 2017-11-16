#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <asm/cacheflush.h>
#include <asm/page.h>
#include <asm/current.h>
#include <linux/sched.h>
#include <linux/kallsyms.h>

#include <linux/utsname.h>

#define MODNAME "SYSCALL HACKING"
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kazuki Igeta <igetakazuki@gmail.com>");
MODULE_DESCRIPTION("Lerning Syscall Hooking");

// 32-bit machine addresses
// sudo cat /boot/System.map-$(uname -r) | grep
unsigned long sys_call_table = 0xc1672140;
unsigned long sys_newuname = 0xc106c280;

typedef asmlinkage long (*orig_uname_t)(struct new_utsname *);
orig_uname_t orig_uname = NULL;

// 64-bit machine addresses
//unsigned long sys_call_table = 0xffffffff00000000;
//unsigned long sys_ni_syscall = 0xffffffff00000000;


asmlinkage long hooked_uname(struct new_utsname *name)
{
	orig_uname(name);
	printk("%s: uname is hooked.\n", MODNAME);
}

int restore;

int init_module(void)
{
	unsigned long * p = (unsigned long *) sys_call_table;
	int i;
	unsigned long cr0;

	orig_uname = (orig_uname_t) sys_newuname;
	printk("%s: init_module\n", MODNAME);

	cr0 = read_cr0();
	write_cr0(cr0 & ~X86_CR0_WP);

	for (i=0; i<256; i++){
		if (p[i] == sys_newuname) {
			printk("%s: table entry %d keeps address %p\n",MODNAME,i,(void*)p[i]);
			restore = i;
			break;
		}
	}

	p[restore] = (unsigned long)hooked_uname;

	write_cr0 (cr0);

	return 0;
}

void cleanup_module(void)
{
	unsigned long * p = (unsigned long *) sys_call_table;
	unsigned long cr0;

	cr0 = read_cr0();
	write_cr0(cr0 & ~X86_CR0_WP);

	p[restore] = sys_newuname;

	write_cr0 (cr0);

	printk("%s: cleanup_module\n", MODNAME);

	return;
}
