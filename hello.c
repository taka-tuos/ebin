void putchar(int c)
{
	asm("\tpi($8)\n");
	asm("\tpr(%d34)\n");
	asm("\tpr(%d24)\n");
	asm("\tldd()\n");
	asm("\tpr(%d24)\n");
	asm("\ttx()\n");
}

void puts(char *s)
{
	for(;*s;s++) putchar(*s);
}

void main(void)
{
	putchar('h');
	putchar('e');
	putchar('l');
	putchar('l');
	putchar('o');
	putchar(',');
	putchar('w');
	putchar('o');
	putchar('r');
	putchar('l');
	putchar('d');
}
