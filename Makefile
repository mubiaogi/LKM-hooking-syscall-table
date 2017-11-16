obj-m := hook_uname.o
KDIR := /lib/modules/`uname -r`/build
PWD := `pwd`

default:
	make -C $(KDIR) M=$(PWD) modules
	gcc user.c -o user.o
clean: 
make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
