#include <stdlib.h>
#include <stdio.h>

int main(int argc, char * argv[])
{
	int sys_call_num, arg;

	if(argc != 2) {
		printf("%s: syscall-num\n", argv[0]);
                exit(EXIT_FAILURE);
        }

	sys_call_num = strtol(argv[1],NULL,10);
	syscall(sys_call_num);

	return 0;
}
