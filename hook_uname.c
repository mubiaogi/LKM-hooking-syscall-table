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
#include <linux/debugfs.h>

#define MODNAME "SYSCALL HACKING"
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kazuki Igeta <igetakazuki@gmail.com>");
MODULE_DESCRIPTION("Lerning Syscall Hooking");

// 32-bit machine addresses
// sudo cat /boot/System.map-$(uname -r) | grep sys_xxx
unsigned long sys_call_table = 0xc1672140;
unsigned long sys_newuname = 0xc106c280;

typedef asmlinkage long (*orig_uname_t)(struct new_utsname *);
orig_uname_t orig_uname = NULL;

// 64-bit machine addresses
//unsigned long sys_call_table = 0xffffffff00000000;
//unsigned long sys_ni_syscall = 0xffffffff00000000;

static struct dentry *testfile;
static char testbuf[128];
int restore;

asmlinkage long hooked_uname(struct new_utsname *name)
{
	//Call original sys_newuname()
	orig_uname(name);
	//Add arbitrary process
	printk("%s: uname is hooked.\n", MODNAME);
}

ssize_t kernelToUser_read(struct file *f, char __user *buf, size_t len, loff_t *ppos)
{
	snprintf(testbuf, sizeof(testbuf), "%d\n", restore);
	return simple_read_from_buffer(buf, len, ppos, testbuf, strlen(testbuf));
}

static struct file_operations testfops = {
	.owner = THIS_MODULE,
	.read  = kernelToUser_read,
};


//Constractor function
int init_module(void)
{
	unsigned long * p = (unsigned long *) sys_call_table;
	int i;
	unsigned long cr0;

	printk("%s: init_module\n", MODNAME);

	orig_uname = (orig_uname_t) sys_newuname;
	
	//Read the bits of cr0
	cr0 = read_cr0();
	//
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

	testfile = debugfs_create_file("testfile", 0400, NULL, NULL, &testfops);
	if (testfile == NULL) {
		return -ENOMEM;
	}

	return 0;
}

//Destructor function
void cleanup_module(void)
{
	unsigned long * p = (unsigned long *) sys_call_table;
	unsigned long cr0;

	cr0 = read_cr0();
	write_cr0(cr0 & ~X86_CR0_WP);

	p[restore] = sys_newuname;
	
	//Restore the original bits of cr0
	write_cr0 (cr0);

	debugfs_remove(testfile);

	printk("%s: cleanup_module\n", MODNAME);

	return;
}
