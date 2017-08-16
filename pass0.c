#include "xprintf.h"

void putchar(unsigned char c)
{
	asm("\tpi($8)\n");
	asm("\tpr(%d34)\n");
	asm("\tpr(%d24)\n");
	asm("\tldd()\n");
	asm("\tpr(%d24)\n");
	asm("\ttx()\n");
}

void main(void)
{
	int n = 114514;
	
	xdev_out(putchar);
	
	//xputs("The C++114514 TOO LATE Programming Language\n");
	xprintf("The C++%d TOO LATE Programming Language\n",n);
	while(1);
}
