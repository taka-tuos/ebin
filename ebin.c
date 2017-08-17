#define EBIN_MODULE_INCLUDE
#include <assert.h>
#include "ebin.h"
#include <stdio.h>
#include <stdlib.h>

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

int kbhit(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if (ch != EOF) {
		ungetc(ch, stdin);
		return 1;
	}

	return 0;
}

void ebin_init(ebin_ctl *ctl, void *e_memory)
{
	ctl->e_memory = e_memory;
	
	memset(ctl->e_intreg,0,36*4);
	
	ctl->e_branched = 0;
	
	ctl->e_pc = 0x1000;
	ctl->e_intreg[34] = ctl->e_intreg[32] = 0x1000 + 0x10000 + 0xffff;
	
	ctl->e_psp = 0xffff;
	ctl->e_opstack = (uint32_t *)malloc(65536 * 4);
}

uint32_t ebin_oppop(ebin_ctl *ctl)
{
	ctl->e_psp++;
	return ctl->e_opstack[ctl->e_psp];
}

uint32_t ebin_oppush(ebin_ctl *ctl, uint32_t data)
{
	ctl->e_opstack[ctl->e_psp] = data;
	ctl->e_psp--;
}

int ebin_pop(ebin_ctl *ctl)
{
	ctl->e_intreg[32] += 4;
	uint8_t *le = ((uint8_t *)ctl->e_memory) + ctl->e_intreg[32];
#ifdef __DEBUG__
	printf("pop : [sp]=%08x\n",ctl->e_intreg[32]-4);
	printf("pop : ->[sp]=%08x\n",ctl->e_intreg[32]);
	printf("pop : [x]=%08x\n",e_read32(le));
#endif
	return e_read32(le);
}

void ebin_push(ebin_ctl *ctl, int data)
{
	uint8_t *le = ((uint8_t *)ctl->e_memory) + ctl->e_intreg[32];
#ifdef __DEBUG__
	printf("push : [sp]=%08x\n",ctl->e_intreg[32]);
	printf("push : ->[sp]=%08x\n",ctl->e_intreg[32]-4);
	printf("push : [x]=%08x\n",data);
#endif
	e_write32(le,data);
	ctl->e_intreg[32] -= 4;
}

ebin_argments ebin_argments_pop(ebin_ctl *ctl)
{
	ebin_argments arg;
	
	arg.opt = ebin_oppop(ctl);
	arg.val = ebin_oppop(ctl);
	
	return arg;
}

void ebin_argments_push(ebin_ctl *ctl, ebin_argments arg)
{
	ebin_oppush(ctl,arg.val);
	ebin_oppush(ctl,arg.opt);
}

int ebin_argments_get_pointer(ebin_ctl *ctl, ebin_argments arg, void **p)
{
	switch(arg.opt & 0x0f) {
	case EFETCH_IREG:
		*p = &(ctl->e_intreg[arg.val]);
		break;
	case EFETCH_MREG:
		*p = ((uint8_t *)ctl->e_memory + (ctl->e_intreg[arg.val]));
		break;
	case EFETCH_IIMM:
		**((uint32_t **)p) = arg.val;
		break;
	case EFETCH_MIMM:
		*p = (ctl->e_memory + arg.val);
		break;
	default:
		assert(0);
		break;
	}
}

int64_t ebin_argments_read(ebin_ctl *ctl, ebin_argments arg, int sign)
{
	uint32_t pdst;
	void *p = &pdst;
	
	ebin_argments_get_pointer(ctl,arg,&p);
	
	int siz = arg.opt >> 4;
	int mem = arg.opt & 1;
	
	uint8_t val8 = *((uint8_t *)p);
	uint16_t val16 = mem ? e_read16((uint8_t *)p) : *((uint16_t *)p);
	uint32_t val32 = mem ? e_read32((uint8_t *)p) : *((uint32_t *)p);
	
	switch(siz) {
	case 0:
		if(sign) return *((int8_t *)&val8);
		else return *((uint8_t *)&val8);
	case 1:
		if(sign) return *((int16_t *)&val16);
		else return *((uint16_t *)&val16);
	case 2:
		if(sign) return *((int32_t *)&val32);
		else return *((uint32_t *)&val32);
	default:
		assert(0);
		break;
	}
	
	return 0;
}

void ebin_argments_write(ebin_ctl *ctl, ebin_argments arg, uint32_t *val)
{
	uint32_t pdst;
	void *p = &pdst;
	
	ebin_argments_get_pointer(ctl,arg,&p);
	
	int siz = arg.opt >> 4;
	int mem = arg.opt & 1;
	
	switch(siz) {
	case 0:
		*((uint8_t *)p) = *((uint8_t *)val);
		break;
	case 1:
		if(mem) e_write16((uint8_t *)p,*((uint16_t *)val));
		else *((uint16_t *)p) = *((uint16_t *)val);
		break;
	case 2:
		if(mem) e_write32((uint8_t *)p,*((uint32_t *)val));
		else *((uint32_t *)p) = *((uint32_t *)val);
		break;
	default:
		assert(0);
		break;
	}
}

void ebin_exec_pi(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	ebin_argments arg;
	
	arg.opt = e_fetch->e_argopt;
	arg.val = e_read32(e_fetch->e_srcimm);
	
	ebin_argments_push(cpu,arg);
}

void ebin_exec_pr(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	ebin_argments arg;
	
	arg.opt = e_fetch->e_argopt;
	arg.val = e_fetch->e_srcreg;
	
	ebin_argments_push(cpu,arg);
}

void ebin_exec_xx(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	
}

void ebin_exec_lr(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	ebin_argments arg;
	
	arg.opt = e_fetch->e_argopt;
	arg.val = e_fetch->e_srcreg;
	
	uint32_t v = ebin_argments_read(cpu,e_args[0],0);
	ebin_argments_write(cpu,arg,&v);
}

void ebin_exec_ldb(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t ptr = ebin_argments_read(cpu,e_args[1],0);
	int32_t ofs = ebin_argments_read(cpu,e_args[2],1);
	
	uint32_t dat = *(((uint8_t *)cpu->e_memory)+ptr+ofs);
	
	ebin_argments_write(cpu,e_args[0],&dat);
}

void ebin_exec_ldw(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t ptr = ebin_argments_read(cpu,e_args[1],0);
	int32_t ofs = ebin_argments_read(cpu,e_args[2],1);
	
	uint32_t dat = e_read16(((uint8_t *)cpu->e_memory)+ptr+ofs);
	
	ebin_argments_write(cpu,e_args[0],&dat);
}

void ebin_exec_ldd(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t ptr = ebin_argments_read(cpu,e_args[1],0);
	int32_t ofs = ebin_argments_read(cpu,e_args[2],1);
	
#ifdef __DEBUG__
	printf("ldd : address(%08x)(%08x+%08x)\n",ptr+ofs,ptr,ofs);
#endif

	uint32_t dat = e_read32(((uint8_t *)cpu->e_memory)+ptr+ofs);
	
	ebin_argments_write(cpu,e_args[0],&dat);
}

void ebin_exec_stb(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t ptr = ebin_argments_read(cpu,e_args[1],0);
	int32_t ofs = ebin_argments_read(cpu,e_args[2],1);
	
	*(((uint8_t *)cpu->e_memory)+ptr+ofs) = ebin_argments_read(cpu,e_args[0],0);
}

void ebin_exec_stw(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t ptr = ebin_argments_read(cpu,e_args[1],0);
	int32_t ofs = ebin_argments_read(cpu,e_args[2],1);
	
	uint32_t data = ebin_argments_read(cpu,e_args[0],0);
	
	e_write16((uint8_t *)(cpu->e_memory+ptr+ofs),data);
}

void ebin_exec_std(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t ptr = ebin_argments_read(cpu,e_args[1],0);
	int32_t ofs = ebin_argments_read(cpu,e_args[2],1);
	
	uint32_t data = ebin_argments_read(cpu,e_args[0],0);
	
	e_write32((uint8_t *)(cpu->e_memory+ptr+ofs),data);
}

void ebin_exec_add(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t a = ebin_argments_read(cpu,e_args[1],0);
	uint32_t b = ebin_argments_read(cpu,e_args[2],0);
	uint32_t c = a + b;
	ebin_argments_write(cpu,e_args[0],&c);
}

void ebin_exec_sub(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t a = ebin_argments_read(cpu,e_args[1],0);
	uint32_t b = ebin_argments_read(cpu,e_args[2],0);
	uint32_t c = a - b;
	ebin_argments_write(cpu,e_args[0],&c);
}

void ebin_exec_mul(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t a = ebin_argments_read(cpu,e_args[1],0);
	uint32_t b = ebin_argments_read(cpu,e_args[2],0);
	uint32_t c = a * b;
	ebin_argments_write(cpu,e_args[0],&c);
}

void ebin_exec_div(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t a = ebin_argments_read(cpu,e_args[1],0);
	uint32_t b = ebin_argments_read(cpu,e_args[2],0);
	uint32_t c = a / b;
	cpu->e_intreg[33] = a % b;
	ebin_argments_write(cpu,e_args[0],&c);
}

void ebin_exec_addi(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	int32_t a = ebin_argments_read(cpu,e_args[1],1);
	int32_t b = ebin_argments_read(cpu,e_args[2],1);
	int32_t c = a + b;
	ebin_argments_write(cpu,e_args[0],(uint32_t *)&c);
}

void ebin_exec_subi(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	int32_t a = ebin_argments_read(cpu,e_args[1],1);
	int32_t b = ebin_argments_read(cpu,e_args[2],1);
	int32_t c = a - b;
	ebin_argments_write(cpu,e_args[0],(uint32_t *)&c);
}

void ebin_exec_muli(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	int32_t a = ebin_argments_read(cpu,e_args[1],1);
	int32_t b = ebin_argments_read(cpu,e_args[2],1);
	int32_t c = a * b;
	ebin_argments_write(cpu,e_args[0],(uint32_t *)&c);
}

void ebin_exec_divi(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	int32_t a = ebin_argments_read(cpu,e_args[1],1);
	int32_t b = ebin_argments_read(cpu,e_args[2],1);
	int32_t c = a / b;
	cpu->e_intreg[33] = a % b;
	ebin_argments_write(cpu,e_args[0],(uint32_t *)&c);
}

void ebin_exec_rsubi(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	int32_t a = ebin_argments_read(cpu,e_args[1],1);
	int32_t b = ebin_argments_read(cpu,e_args[2],1);
	int32_t c = b - a;
	ebin_argments_write(cpu,e_args[0],(uint32_t *)&c);
}

void ebin_exec_rsub(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t a = ebin_argments_read(cpu,e_args[1],0);
	uint32_t b = ebin_argments_read(cpu,e_args[2],0);
	uint32_t c = b - a;
	ebin_argments_write(cpu,e_args[0],&c);
}

void ebin_exec_and(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t a = ebin_argments_read(cpu,e_args[1],0);
	uint32_t b = ebin_argments_read(cpu,e_args[2],0);
	uint32_t c = a & b;
	ebin_argments_write(cpu,e_args[0],&c);
}

void ebin_exec_or(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t a = ebin_argments_read(cpu,e_args[1],0);
	uint32_t b = ebin_argments_read(cpu,e_args[2],0);
	uint32_t c = a | b;
	ebin_argments_write(cpu,e_args[0],&c);
}

void ebin_exec_xor(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t a = ebin_argments_read(cpu,e_args[1],0);
	uint32_t b = ebin_argments_read(cpu,e_args[2],0);
	uint32_t c = a ^ b;
	ebin_argments_write(cpu,e_args[0],&c);
}

void ebin_exec_not(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t a = ebin_argments_read(cpu,e_args[1],0);
	uint32_t c = ~a;
	ebin_argments_write(cpu,e_args[0],&c);
}

void ebin_exec_lrs(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t a = ebin_argments_read(cpu,e_args[1],0);
	uint32_t b = ebin_argments_read(cpu,e_args[2],0);
	uint32_t c = a >> b;
	ebin_argments_write(cpu,e_args[0],&c);
}

void ebin_exec_lls(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t a = ebin_argments_read(cpu,e_args[1],0);
	uint32_t b = ebin_argments_read(cpu,e_args[2],0);
	uint32_t c = a << b;
	ebin_argments_write(cpu,e_args[0],&c);
}

void ebin_exec_ars(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t a = ebin_argments_read(cpu,e_args[1],0);
	uint32_t b = ebin_argments_read(cpu,e_args[2],0);
	uint32_t c = a >> b | (a & 0x80000000);
	ebin_argments_write(cpu,e_args[0],&c);
}

void ebin_exec_tx(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t c = ebin_argments_read(cpu,e_args[0],0);
	//printf("tx() : \'%c\'\n",c);
	putchar(c);
	//if(c == 0x0a) exit(0);
}

void ebin_exec_rx(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t c;
	c = kbhit() ? getchar() : 0xff;
	
	ebin_argments_write(cpu,e_args[0],&c);
}

void ebin_exec_tst(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t c = ebin_argments_read(cpu,e_args[0],0);
	cpu->e_intreg[35] = 0;
	if(c == 0) cpu->e_intreg[35] |= EF_Z;
}

void ebin_exec_cmp(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t c1 = ebin_argments_read(cpu,e_args[0],0); // d0
	uint32_t c2 = ebin_argments_read(cpu,e_args[1],0); // 9
	cpu->e_intreg[35] = 0;
	if(c1 ==  0) cpu->e_intreg[35] |= EF_Z;
	if(c1 == c2) cpu->e_intreg[35] |= EF_E;
	if(c1  > c2) cpu->e_intreg[35] |= EF_A;
	if(c1  < c2) cpu->e_intreg[35] |= EF_B;
}

void ebin_exec_bz(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t c = ebin_argments_read(cpu,e_args[0],0);
	if((cpu->e_intreg[35] & EF_Z) != 0) {
		cpu->e_pc = c;
		cpu->e_branched = 1;
	}
}

void ebin_exec_bnz(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t c = ebin_argments_read(cpu,e_args[0],0);
	if((cpu->e_intreg[35] & EF_Z) == 0) {
		cpu->e_pc = c;
		cpu->e_branched = 1;
	}
}

void ebin_exec_beq(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t c = ebin_argments_read(cpu,e_args[0],0);
	if((cpu->e_intreg[35] & EF_E) != 0) {
		cpu->e_pc = c;
		cpu->e_branched = 1;
	}
}

void ebin_exec_bne(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t c = ebin_argments_read(cpu,e_args[0],0);
	if((cpu->e_intreg[35] & EF_E) == 0) {
		cpu->e_pc = c;
		cpu->e_branched = 1;
	}
}

void ebin_exec_ba(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t c = ebin_argments_read(cpu,e_args[0],0);
	if((cpu->e_intreg[35] & EF_A) != 0) {
		cpu->e_pc = c;
		cpu->e_branched = 1;
	}
}

void ebin_exec_bb(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t c = ebin_argments_read(cpu,e_args[0],0);
	if((cpu->e_intreg[35] & EF_B) != 0) {
		cpu->e_pc = c;
		cpu->e_branched = 1;
	}
}

void ebin_exec_bae(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t c = ebin_argments_read(cpu,e_args[0],0);
	if((cpu->e_intreg[35] & EF_A) != 0 || (cpu->e_intreg[35] & EF_E) != 0) {
		cpu->e_pc = c;
		cpu->e_branched = 1;
	}
}

void ebin_exec_bbe(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t c = ebin_argments_read(cpu,e_args[0],0);
	if((cpu->e_intreg[35] & EF_B) != 0 || (cpu->e_intreg[35] & EF_E) != 0) {
		cpu->e_pc = c;
		cpu->e_branched = 1;
	}
}

void ebin_exec_b(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t c = ebin_argments_read(cpu,e_args[0],0);
	cpu->e_pc = c;
	cpu->e_branched = 1;
}

void ebin_exec_c(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t c = ebin_argments_read(cpu,e_args[0],0);
	
	ebin_push(cpu,cpu->e_pc+2);
	
	cpu->e_pc = c;
	cpu->e_branched = 1;
}

void ebin_exec_r(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	cpu->e_pc = ebin_pop(cpu);
	
	cpu->e_branched = 1;
}

void ebin_exec_push(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t c = ebin_argments_read(cpu,e_args[0],0);
	
	ebin_push(cpu,c);
}

void ebin_exec_pop(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t c = ebin_pop(cpu);
	
	ebin_argments_write(cpu,e_args[0],&c);
}

void ebin_exec_sigxb(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t a = ebin_argments_read(cpu,e_args[1],0);
	uint32_t c = a | (((a >> 7) & 1) * 0xFFFFFF00); 
	ebin_argments_write(cpu,e_args[0],&c);
}

void ebin_exec_sigxw(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[])
{
	uint32_t a = ebin_argments_read(cpu,e_args[1],0);
	uint32_t c = a | (((a >> 15) & 1) * 0xFFFF0000); 
	ebin_argments_write(cpu,e_args[0],&c);
}

ebin_execute ebin_execute_func[] = {
	ebin_exec_xx,
	ebin_exec_pi,
	ebin_exec_pr,
	ebin_exec_lr,
	ebin_exec_ldb,
	ebin_exec_ldw,
	ebin_exec_ldd,
	ebin_exec_stb,
	ebin_exec_stw,
	ebin_exec_std,
	ebin_exec_push,
	ebin_exec_pop,
	ebin_exec_add,
	ebin_exec_sub,
	ebin_exec_mul,
	ebin_exec_div,
	ebin_exec_addi,
	ebin_exec_subi,
	ebin_exec_muli,
	ebin_exec_divi,
	ebin_exec_rsub,
	ebin_exec_rsubi,
	ebin_exec_sigxb,
	ebin_exec_sigxw,
	ebin_exec_and,
	ebin_exec_or,
	ebin_exec_xor,
	ebin_exec_not,
	ebin_exec_lrs,
	ebin_exec_lls,
	ebin_exec_ars,
	ebin_exec_tx,
	ebin_exec_rx,
	ebin_exec_tst,
	ebin_exec_cmp,
	ebin_exec_bz,
	ebin_exec_bnz,
	ebin_exec_beq,
	ebin_exec_bne,
	ebin_exec_ba,
	ebin_exec_bae,
	ebin_exec_bb,
	ebin_exec_bbe,
	ebin_exec_b,
	ebin_exec_c,
	ebin_exec_r,
};

#ifdef __DEBUG__
void ebin_print_argment(ebin_argments arg)
{
	int siz = arg.opt >> 4;
	int bch = "bwd"[siz];
	switch(arg.opt & 0x0f) {
	case EFETCH_IREG:
		printf("%%%c%d",bch,arg.val);
		break;
	case EFETCH_MREG:
		printf("%c%%d%d",bch,arg.val);
		break;
	case EFETCH_IIMM:
		printf("$0x%x[%d](%c)",arg.val,arg.val,bch);
		break;
	case EFETCH_MIMM:
		printf("%c$0x%x[%d]",bch,arg.val,arg.val);
		break;
	}
}
#endif

void ebin_exec(ebin_ctl *ctl)
{
	ebin_fetch *e_fetch = (ebin_fetch *)((uint8_t *)ctl->e_memory + ctl->e_pc);
	
	uint16_t e_opcode = e_read16(e_fetch->e_opcode);
	
	if(e_opcode < sizeof(operation_list)/sizeof(ebin_operation)) {
		ebin_argments args[4];
		for(int i = 0; i < operation_list[e_opcode].reqn; i++) {
			args[i] = ebin_argments_pop(ctl);
		}
#ifdef __DEBUG__
		printf("%08x : %s(",ctl->e_pc,operation_list[e_opcode].name);
		ebin_argments arg;
		
		if(operation_list[e_opcode].type < 2) {
			
			arg.opt = e_fetch->e_argopt;
			if(operation_list[e_opcode].type == 0) arg.val = e_read32(e_fetch->e_srcimm);
			if(operation_list[e_opcode].type == 1) arg.val = e_fetch->e_srcreg;
			ebin_print_argment(arg);
		}
		puts(")");
		
		if(operation_list[e_opcode].type < 2) {
			printf("[x]=0x%08x\n",ebin_argments_read(ctl,arg,0));
			printf(">> ");
			ebin_print_argment(arg);
			puts("");
		}
		
		for(int i = 0; i < operation_list[e_opcode].reqn; i++) {
			printf("[%d]=0x%08x\n",i,ebin_argments_read(ctl,args[i],0));
			printf(">> ");
			ebin_print_argment(args[i]);
			puts("");
		}
#endif
		ebin_execute_func[e_opcode](ctl,e_fetch,args);
#ifdef __DEBUG__
		printf("->[x]=0x%08x\n",ebin_argments_read(ctl,arg,0));
		
		for(int i = 0; i < operation_list[e_opcode].reqn; i++) {
			printf("->[%d]=0x%08x\n",i,ebin_argments_read(ctl,args[i],0));
		}
		
		printf("PSP=0x%08x\n",ctl->e_psp);
#endif
		if(!ctl->e_branched) ctl->e_pc += operation_size[operation_list[e_opcode].type];
		ctl->e_branched = 0;
	} else {
		ctl->e_pc++;
	}
}
