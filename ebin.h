#include <string.h>
#include <stdint.h>

#ifndef __EBIN__
#define __EBIN__

enum {
	EFETCH_IREG,
	EFETCH_MREG,
	EFETCH_IIMM,
	EFETCH_MIMM,
};

typedef struct {
	char *name;
	int type; // use on as
	int reqn;
	int branch;
} ebin_operation;

enum {
	EF_X=0x0000,
	EF_Z=0x0001,
	EF_E=0x0002,
	EF_A=0x0004,
	EF_B=0x0008,
};

#ifdef EBIN_MODULE_INCLUDE
ebin_operation operation_list[] = {
	{ "nop", 2, 0, 0 },
	{ "pi", 0, 0, 0 }, // 8 bytes
	{ "pr", 1, 0, 0 }, // 4 bytes
	{ "lr", 1, 1, 0 }, 
	{ "ldb", 2, 3, 0 }, // 2 bytes
	{ "ldw", 2, 3, 0 },
	{ "ldd", 2, 3, 0 },
	{ "stb", 2, 3, 0 },
	{ "stw", 2, 3, 0 },
	{ "std", 2, 3, 0 },
	{ "push", 2, 1, 0 },
	{ "pop", 2, 1, 0 },
	{ "add", 2, 3, 0 },
	{ "sub", 2, 3, 0 },
	{ "mul", 2, 3, 0 },
	{ "div", 2, 3, 0 },
	{ "addi", 2, 3, 0 },
	{ "subi", 2, 3, 0 },
	{ "muli", 2, 3, 0 },
	{ "divi", 2, 3, 0 },
	{ "rsub", 2, 3, 0 },
	{ "rsubi", 2, 3, 0 },
	{ "sigxb", 2, 2, 0 },
	{ "sigxw", 2, 2, 0 },
	{ "and", 2, 3, 0 },
	{ "or", 2, 3, 0 },
	{ "xor", 2, 3, 0 },
	{ "not", 2, 2, 0 },
	{ "lrs", 2, 3, 0 },
	{ "lls", 2, 3, 0 },
	{ "ars", 2, 3, 0 },
	{ "tx", 2, 1, 0 },
	{ "rx", 2, 1, 0 },
	{ "tst", 2, 1, 0 },
	{ "cmp", 2, 2, 0 },
	{ "bz", 2, 1, 1 },
	{ "bnz", 2, 1, 1 },
	{ "beq", 2, 1, 1 },
	{ "bne", 2, 1, 1 },
	{ "ba", 2, 1, 1 },
	{ "bae", 2, 1, 1 },
	{ "bb", 2, 1, 1 },
	{ "bbe", 2, 1, 1 },
	{ "b", 2, 1, 1 },
	{ "c", 2, 1, 1 },
	{ "r", 2, 0, 1 },
};
#else
extern ebin_operation operation_list[];
#endif

#ifdef EBIN_MODULE_INCLUDE
int operation_size[] = {
	8, 4, 2
};
#else
extern int operation_size[];
#endif

typedef struct {
	void *e_memory;
	uint32_t e_intreg[36]; /* R0 - R31, SP, Y, BP, F */
	uint32_t e_pc;
	uint32_t e_psp;
	uint32_t *e_opstack;
	uint32_t e_flag;
	
	int e_branched;
} ebin_ctl;

typedef struct {
	uint8_t e_opcode[2];
	uint8_t e_argopt;
	uint8_t e_srcreg;
	uint8_t e_srcimm[4];
} ebin_fetch;

typedef struct {
	uint32_t val;
	uint32_t opt;
} ebin_argments;

typedef void(*ebin_execute)(ebin_ctl *cpu, ebin_fetch *e_fetch, ebin_argments e_args[]);

#define e_read16(array) ((((uint16_t)(array)[1]) << 8) | ((uint16_t)(array)[0]))
#define e_read32(array) ((((uint32_t)(array)[3]) << 24) | (((uint32_t)(array)[2]) << 16) | (((uint32_t)(array)[1]) << 8) | ((uint32_t)(array)[0]))

#define e_write16(array,data) (array)[0] = (data) & 0xff, (array)[1] = ((data) >> 8) & 0xff
#define e_write32(array,data) (array)[0] = (data) & 0xff, (array)[1] = ((data) >> 8) & 0xff, (array)[2] = ((data) >> 16) & 0xff, (array)[3] = ((data) >> 24) & 0xff


void ebin_init(ebin_ctl *ctl, void *e_memory);
void ebin_exec(ebin_ctl *ctl);

#endif
