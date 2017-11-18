# hook-syscall
LKM for learning syscall hooking.

USAGE
1) Copy the addresses of sys_call_table and sys_newuname.
------
>sudo cat /boot/System.map-$(uname -r) | grep sys_call_table
>sudo cat /boot/System.map-$(uname -r) | grep sys_newuname
------

2) Replace these addresses in hook_uname.c.
3) Compile the files.
------
>make
------

4) Insert the LKM into Linux Kernel.
------
>sudo insmod hook_uname.ko
------

5) Test the LKM.

