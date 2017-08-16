/*
Copyright (c) 2012-2015, Alexey Frunze
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*****************************************************************************/
/*                                                                           */
/*                                Smaller C                                  */
/*                                                                           */
/*                 A simple and small single-pass C compiler                 */
/*                                                                           */
/*                           ebin code generator                             */
/*                                                                           */
/*****************************************************************************/
#include <stdint.h>

enum {
	EF_X=0x0000,
	EF_Z=0x0001,
	EF_E=0x0002,
	EF_A=0x0004,
	EF_B=0x0008,
};

#define MAX_GLOBALS_TABLE_LEN 16384

STATIC
void GenInit(void)
{
	// initialization of target-specific code generator
	SizeOfWord = 4;
	OutputFormat = FormatSegmented;
	/*CodeHeaderFooter[0] = "\tsection .text";
	DataHeaderFooter[0] = "\tsection .data";
	RoDataHeaderFooter[0] = "\tsection .rodata";
	BssHeaderFooter[0] = "\tsection .bss";*/
	CodeHeaderFooter[0] = "";
	DataHeaderFooter[0] = "";
	RoDataHeaderFooter[0] = "";
	BssHeaderFooter[0] = "";
}

STATIC
int GenInitParams(int argc, char** argv, int* idx)
{
	(void)argc;
	(void)argv;
	(void)idx;
	// initialization of target-specific code generator with parameters
	
	return 0;
}

STATIC
void GenInitFinalize(void)
{
	// finalization of initialization of target-specific code generator
}

STATIC
void GenStartCommentLine(void)
{
	printf2("\t; ");
}

STATIC
void GenWordAlignment(int bss)
{
	(void)bss;
	printf2("\talign($2)\n");
}

STATIC
void GenLabel(char* Label, int Static)
{
	{
		if (!Static && GenExterns)
			printf2("\tglobal(._%s)\n", Label);
		printf2("._%s\n", Label);
	}
	GenAddGlobal(Label, 1);
}

STATIC
void GenPrintLabel(char* Label)
{
	{
		if (isdigit(*Label))
			printf2(".L%s", Label);
		else
			printf2("._%s", Label);
	}
}

STATIC
void GenNumLabel(int Label)
{
	printf2(".L%d\n", Label);
}

STATIC
void GenPrintNumLabel(int label)
{
	printf2(".L%d", label);
}

STATIC
void GenZeroData(unsigned Size, int bss)
{
	(void)bss;
	printf2("\tfill($%d)\n", Size);
}

STATIC
void GenIntData(int Size, int Val)
{
	Val = truncInt(Val);
	if (Size == 1)
		printf2("\tdb($%d)\n", Val);
	else if (Size == 2)
		printf2("\tdw($%d)\n", Val);
	else if (Size == 4)
		printf2("\tdd($%d)\n", Val);
}

STATIC
void GenStartAsciiString(void)
{
	printf2("\ttext(");
}

STATIC
void GenEndAsciiString(void)
{
	printf2(")\n");
}

char GlobalsTable[MAX_GLOBALS_TABLE_LEN];
int GlobalsTableLen = 0;


STATIC
void GenAddGlobal(char* s, int use)
{
	int i = 0;
	int l;
	if (GenExterns)
	{
		while (i < GlobalsTableLen)
		{
			if (!strcmp(GlobalsTable + i + 2, s))
			{
				GlobalsTable[i] |= use;
				return;
			}
			i += GlobalsTable[i + 1] + 2;
		}
		l = strlen(s) + 1;
		if (GlobalsTableLen + l + 2 > MAX_GLOBALS_TABLE_LEN)
			error("Table of globals exhausted\n");
		GlobalsTable[GlobalsTableLen++] = use;
		GlobalsTable[GlobalsTableLen++] = l;
		memcpy(GlobalsTable + GlobalsTableLen, s, l);
		GlobalsTableLen += l;
	}
}

STATIC
void GenAddrData(int Size, char* Label, int ofs)
{
	ofs = truncInt(ofs);
	if (Size == 1)
		printf2("\t.db\t");
	else if (Size == 2)
		printf2("\t.dw\t");
	else if (Size == 4)
		printf2("\t.dd\t");
	GenPrintLabel(Label);
	if (ofs)
		printf2(" %+d", ofs);
	puts2("");
	if (!isdigit(*Label))
		GenAddGlobal(Label, 2);
}

STATIC
int GenFxnSizeNeeded(void)
{
	return 0;
}

STATIC
void GenRecordFxnSize(char* startLabelName, int endLabelNo)
{
	(void)startLabelName;
	(void)endLabelNo;
}

#define ebinInstrNop		0x00

#define ebinInstrMov		0x80

#define ebinInstrPi			0x80
#define ebinInstrPr			0x81
#define ebinInstrLr			0x82

#define ebinInstrLB			0x02
#define ebinInstrLW			0x03
#define ebinInstrL			0x04
#define ebinInstrSB			0x05
#define ebinInstrSW			0x06
#define ebinInstrS			0x07

#define ebinInstrAdd		0x0A
#define ebinInstrSub		0x0B
#define ebinInstrRSub		0x0C
#define ebinInstrAnd		0x0D
#define ebinInstrOr			0x0E
#define ebinInstrXor		0x0F
#define ebinInstrNot		0x10
#define ebinInstrLLS		0x11
#define ebinInstrLRS		0x12
#define ebinInstrARS		0x13
#define ebinInstrMul		0x14
#define ebinInstrSDiv		0x15
#define ebinInstrDiv		0x16

#define ebinInstrRJmp		0x17
#define ebinInstrRCall		0x18
#define ebinInstrCall		0x19
#define ebinInstrRet		0x1A
#define ebinInstrPush		0x1B
#define ebinInstrPop		0x1C

#define ebinInstrJE			0x20
#define ebinInstrJAE		0x21
#define ebinInstrJBE		0x22
#define ebinInstrJA			0x23
#define ebinInstrJB			0x24
#define ebinInstrJZ			0x25
#define ebinInstrJNZ		0x26
#define ebinInstrJNE		0x27
#define ebinInstrIf			0x28

STATIC
void GenPrintInstr(int instr, int val)
{
	char *p="nop";
	
	switch (instr)
	{
	case ebinInstrNop	: p = "nop"; break;

	case ebinInstrPi	: p = "pi"; break;
	case ebinInstrPr	: p = "pr"; break;
	case ebinInstrLr	: p = "lr"; break;

	case ebinInstrLB	: p = "ldb"; break;
	case ebinInstrLW	: p = "ldw"; break;
	case ebinInstrL		: p = "ldd"; break;
	case ebinInstrSB	: p = "stb"; break;
	case ebinInstrSW	: p = "stw"; break;
	case ebinInstrS		: p = "stb"; break;
	
	case ebinInstrAdd	: p = "add"; break;
	case ebinInstrSub	: p = "sub"; break;
	case ebinInstrRSub	: p = "rsub"; break;
	case ebinInstrAnd	: p = "and"; break;
	case ebinInstrOr	: p = "or"; break;
	case ebinInstrXor	: p = "xor"; break;
	case ebinInstrNot	: p = "not"; break;
	case ebinInstrLLS	: p = "lls"; break;
	case ebinInstrLRS	: p = "lrs"; break;
	case ebinInstrARS	: p = "ars"; break;
	case ebinInstrMul	: p = "mul"; break;
	case ebinInstrSDiv	: p = "divi"; break;
	case ebinInstrDiv	: p = "div"; break;

	case ebinInstrRJmp	: p = "b"; break;
	case ebinInstrRCall	: p = "c"; break;
	case ebinInstrCall	: p = "c"; break;
	case ebinInstrRet	: p = "r"; break;
	case ebinInstrPush	: p = "push"; break;
	case ebinInstrPop	: p = "pop"; break;

	case ebinInstrIf	: p = "cmp"; break;
	
	case ebinInstrJE	: p = "beq"; break;
	case ebinInstrJAE	: p = "bae"; break;
	case ebinInstrJBE	: p = "bbe"; break;
	case ebinInstrJA	: p = "ba"; break;
	case ebinInstrJB	: p = "bb"; break;
	case ebinInstrJZ	: p = "bz"; break;
	case ebinInstrJNZ	: p = "bnz"; break;
	case ebinInstrJNE	: p = "bne"; break;
	}

	printf2("\t%s", p);
}

#define ebinOpReg0											0x00
//...
#define ebinOpRegY											0x21
#define ebinOpRegBp											0x22
#define ebinOpRegSp											0x20
//...
#define ebinOpRegFlags										0x23

#define ebinOpIndReg0										0x30
//...
#define ebinOpIndRegY										0x51
#define ebinOpIndRegBp										0x52
#define ebinOpIndRegSp										0x50
//...
#define ebinOpIndRegFlags									0x53

#define ebinOpConst											0x80
#define ebinOpLabel											0x81
#define ebinOpNumLabel										0x82

#define MAX_TEMP_REGS 29 // this many temp registers used beginning with R1 to hold subexpression results
#define TEMP_REG_A 30 // two temporary registers used for momentary operations, similarly to the MIPS AT/R1 register
#define TEMP_REG_B 31

#define tokRevMinus		0x100 // reversed subtraction, RSB
#define tokRevIdent		0x101
#define tokRevLocalOfs	0x102

int GenRegsUsed; // bitmask of registers used by the function being compiled

int GenMemPrefix[] = { 'x', 'b', 'w', 'x', 'd' };

STATIC
void GenPrintOperand(int op, long val)
{
	if (op >= ebinOpReg0 && op <= ebinOpRegFlags)
	{
		GenRegsUsed |= 1 << op;
		printf2("%%d%d", op);
	}
	else if (op >= ebinOpIndReg0 && op <= ebinOpIndRegFlags)
	{
		printf2("%%d%d", op - ebinOpIndReg0);
	}
	else
	{
		switch (op)
		{
		case ebinOpConst: printf2("$%10ld", truncInt(val)); break;
		case ebinOpLabel: GenPrintLabel(IdentTable + val); break;
		case ebinOpNumLabel: GenPrintNumLabel(val); break;

		default:
			//error("WTF!\n");
			errorInternal(100);
			break;
		}
	}
}

STATIC
void GenPrintOperandSeparator(void)
{
	printf2(",");
}

STATIC
void GenPrintOperandStarter(void)
{
	printf2("(");
}

STATIC
void GenPrintOperandEnder(void)
{
	printf2(")");
}

STATIC
void GenPrintNewLine(void)
{
	puts2("");
}

STATIC
void GenPrintInstrNoOperand(int instr, long instrval)
{
	GenPrintInstr(instr, instrval);
	GenPrintOperandStarter();
	GenPrintOperandEnder();
	GenPrintNewLine();
}

STATIC
void GenPrintInstrOneOperand(int instr, int instrval, int operand, long operandval)
{
	GenPrintInstr(instr, instrval);
	GenPrintOperandStarter();
	GenPrintOperand(operand, operandval);
	GenPrintOperandEnder();
	GenPrintNewLine();
}

STATIC
void GenPrintInstr1Operand(int instr, int instrval, int operand, long operandval);
STATIC
void GenPrintInstr2Operands(int instr, int instrval, int operand1, long operand1val, int operand2, long operand2val);
STATIC
void GenPrintInstr3Operands(int instr, int instrval, int operand1, long operand1val, int operand2, long operand2val, int operand3, long operand3val);

STATIC
void GenPrintInstr1Operand(int instr, int instrval, int operand, long operandval)
{
	if(instr == ebinInstrNot)
	{
		GenPrintInstr2Operands(instr, instrval,
														operand, operandval,
														operand, operandval);
	} else {
		GenPrintInstrOneOperand(operand < ebinOpConst ? ebinInstrPr : ebinInstrPi, 0, operand, operandval);
		
		GenPrintInstrNoOperand(instr, instrval);
	}
}

STATIC
void GenPrintInstr2Operands(int instr, int instrval, int operand1, long operand1val, int operand2, long operand2val)
{
	if(instr == ebinInstrMov) {
		GenPrintInstrOneOperand(operand2 < ebinOpConst ? ebinInstrPr : ebinInstrPi, 0, operand2, operand2val);
		
		GenPrintInstrOneOperand(ebinInstrLr, 0, operand1, operand1val);
	} else {
		if(instr >= ebinInstrAdd && instr <= ebinInstrDiv) {
			GenPrintInstr3Operands(instr, instrval,
														operand1, operand1val,
														operand1, operand1val,
														operand2, operand2val);
		} else if(instr >= ebinInstrLB && instr <= ebinInstrL) {
			GenPrintInstr3Operands(instr, instrval,
														operand1, operand1val,
														operand2, operand2val,
														ebinOpConst, operand2val);
		} else if(instr >= ebinInstrSB && instr <= ebinInstrS) {
			GenPrintInstr3Operands(instr, instrval,
														operand2, operand2val,
														operand1, operand1val,
														ebinOpConst, operand1val);
		} else {
			GenPrintInstrOneOperand(operand2 < ebinOpConst ? ebinInstrPr : ebinInstrPi, 0, operand2, operand2val);
			GenPrintInstrOneOperand(operand1 < ebinOpConst ? ebinInstrPr : ebinInstrPi, 0, operand1, operand1val);
			
			GenPrintInstrNoOperand(instr, instrval);
		}
	}
}

STATIC
void GenPrintInstr3Operands(int instr, int instrval, int operand1, long operand1val, int operand2, long operand2val, int operand3, long operand3val)
{
	GenPrintInstrOneOperand(operand3 < ebinOpConst ? ebinInstrPr : ebinInstrPi, 0, operand3, operand3val);
	GenPrintInstrOneOperand(operand2 < ebinOpConst ? ebinInstrPr : ebinInstrPi, 0, operand2, operand2val);
	GenPrintInstrOneOperand(operand1 < ebinOpConst ? ebinInstrPr : ebinInstrPi, 0, operand1, operand1val);
	
	GenPrintInstrNoOperand(instr, instrval);
}

STATIC
void GenExtendRegIfNeeded(int reg, int opSz)
{
	if (opSz == -1)
	{
		GenPrintInstr2Operands(ebinInstrMov, 0,
													 TEMP_REG_A, 0,
													 reg, 0);
		GenPrintInstr2Operands(ebinInstrLRS, 0,
													 TEMP_REG_A, 0,
													 ebinOpConst, 7);
		GenPrintInstr2Operands(ebinInstrAnd, 0,
													 TEMP_REG_A, 0,
													 ebinOpConst, 1);
		GenPrintInstr2Operands(ebinInstrMul, 0,
													 TEMP_REG_A, 0,
													 ebinOpConst, 0xFFFFFF00);
		GenPrintInstr2Operands(ebinInstrOr, 0,
													 reg, 0,
													 TEMP_REG_A, 0);
	}
	else if (opSz == 1)
	{
		GenPrintInstr2Operands(ebinInstrAnd, 0,
													 reg, 0,
													 ebinOpConst, 0xFF);
	}
	else if (opSz == -2)
	{
		GenPrintInstr2Operands(ebinInstrMov, 0,
													 TEMP_REG_A, 0,
													 reg, 0);
		GenPrintInstr2Operands(ebinInstrLRS, 0,
													 TEMP_REG_A, 0,
													 ebinOpConst, 15);
		GenPrintInstr2Operands(ebinInstrAnd, 0,
													 TEMP_REG_A, 0,
													 ebinOpConst, 1);
		GenPrintInstr2Operands(ebinInstrMul, 0,
													 TEMP_REG_A, 0,
													 ebinOpConst, 0xFFFF0000);
		GenPrintInstr2Operands(ebinInstrOr, 0,
													 reg, 0,
													 TEMP_REG_A, 0);
	}
	else if (opSz == 2)
	{
		GenPrintInstr2Operands(ebinInstrAnd, 0,
													 reg, 0,
													 ebinOpConst, 0xFFFF);
	}
}

STATIC
void GenJumpUncond(int label)
{
	GenPrintInstr1Operand(ebinInstrRJmp, 0,
												ebinOpNumLabel, label);
}

extern int GenWreg; // GenWreg is defined below

#ifndef USE_SWITCH_TAB
STATIC
void GenJumpIfEqual(int val, int label)
{
#ifndef NO_ANNOTATIONS
	printf2("\t; JumpIfEqual\n");
#endif
	GenPrintInstr2Operands(ebinInstrIf, 0,
												 GenWreg, 0,
												 ebinOpConst, val);
	GenPrintInstr1Operand(ebinInstrJE, 0,
												ebinOpNumLabel, label);
}
#endif

STATIC
void GenJumpIfZero(int label)
{
#ifndef NO_ANNOTATIONS
	printf2("\t; JumpIfZero\n");
#endif
	GenPrintInstr2Operands(ebinInstrIf, 0,
												 GenWreg, 0,
												 GenWreg, 0);
	GenPrintInstr1Operand(ebinInstrJZ, 0,
												ebinOpNumLabel, label);
}

STATIC
void GenJumpIfNotZero(int label)
{
#ifndef NO_ANNOTATIONS
	printf2("\t; JumpIfNotZero\n");
#endif
	GenPrintInstr2Operands(ebinInstrIf, 0,
												 GenWreg, 0,
												 GenWreg, 0);
	GenPrintInstr1Operand(ebinInstrJNZ, 0,
												ebinOpNumLabel, label);
}

STATIC
void GenSaveRestoreRegs(int save)
{
	int rstart, rstop, rinc, r;
	int mask = GenRegsUsed;
	mask &= ~(1 << ebinOpReg0); // not preserved
//	mask &= ~(1 << ebinOpRegY); // TBD??? Y is preserved, right???
	mask &= ~(1 << ebinOpRegBp); // taken care of
	mask &= ~(1 << ebinOpRegSp); // taken care of
	mask &= ~(1 << ebinOpRegFlags); // TBD??? flags aren't preserved, right???

	if (save)
		rstart = ebinOpReg0, rstop = ebinOpRegFlags, rinc = 1;
	else
		rstart = ebinOpRegFlags, rstop = ebinOpReg0, rinc = -1;

	for (r = rstart; r != rstop + rinc; r += rinc)
	{
		int used = (mask & (1 << r)) != 0;
		if (save || used)
		{
			if(used) printf2(save ? "\tpr(%%d%02d)\n" : "\tpr(%%d%02d)\n",r);
			else printf2(save ? "\t;pr %%d%02d \n" : "\t;pr %%d%02d \n",r);
			if(used) printf2(save ? "\tpush()\n" : "\tpop()\n");
			else printf2(save ? "\t;push  \n" : "\t;pop  \n");
		}
	}
	GenRegsUsed = mask; // undo changes in GenRegsUsed by GenPrintOperand()
}

void GenIsrProlog(void)
{
	// TBD???
}

void GenIsrEpilog(void)
{
	// TBD???
}

long GenPrologPos;
long GenPrologSiz;

STATIC
void GenWriteFrameSize(void)
{
	unsigned size = -CurFxnMinLocalOfs;
	//if(!size) GenStartCommentLine();
	GenPrintInstr2Operands(ebinInstrSub, 0,
										ebinOpRegSp, 0,
										ebinOpConst, size*4);
	GenSaveRestoreRegs(1);
}

STATIC
void GenUpdateFrameSize(void)
{
	long pos;
	pos = ftell(OutFile);
	fseek(OutFile, GenPrologPos, SEEK_SET);
	GenWriteFrameSize();
	fseek(OutFile, GenPrologSiz, SEEK_SET);
	GenPrologSiz -= GenPrologPos;
	fseek(OutFile, pos, SEEK_SET);
}

STATIC
void GenFxnProlog(void)
{
	GenRegsUsed = 0;

	GenPrintInstr1Operand(ebinInstrPush, 0,
												ebinOpRegBp, 0);

	GenPrintInstr2Operands(ebinInstrMov, 0,
												 ebinOpRegBp, 0,
												 ebinOpRegSp, 0);
	
	GenPrintInstr2Operands(ebinInstrAdd, 0,
												 ebinOpRegBp, 0,
												 ebinOpConst, 4);

	GenPrologPos = ftell(OutFile);
	GenWriteFrameSize();
	GenPrologSiz = ftell(OutFile);
	GenPrologSiz -= GenPrologPos;
}

STATIC
void GenGrowStack(int size)
{
	if (!size)
		return;
	GenPrintInstr2Operands(ebinInstrSub, 0,
												 ebinOpRegSp, 0,
												 ebinOpConst, size);
}

STATIC
void GenFxnEpilog(void)
{
	GenUpdateFrameSize();

	GenSaveRestoreRegs(0);

	GenPrintInstr2Operands(ebinInstrSub, 0,
												 ebinOpRegBp, 0,
												 ebinOpConst, 4);

	GenPrintInstr2Operands(ebinInstrMov, 0,
												 ebinOpRegSp, 0,
												 ebinOpRegBp, 0);

	GenPrintInstr1Operand(ebinInstrPop, 0,
												ebinOpRegBp, 0);

	GenPrintInstrNoOperand(ebinInstrRet, 0);
}

STATIC
int GenMaxLocalsSize(void)
{
	return 0x7FFFFFFF;
}

STATIC
int GenGetBinaryOperatorInstr(int tok)
{
	switch (tok)
	{
	case tokPostAdd:
	case tokAssignAdd:
	case '+':
		return ebinInstrAdd;
	case tokPostSub:
	case tokAssignSub:
	case '-':
		return ebinInstrSub;
	case tokRevMinus:
		return ebinInstrRSub;
	case '&':
	case tokAssignAnd:
		return ebinInstrAnd;
	case '^':
	case tokAssignXor:
		return ebinInstrXor;
	case '|':
	case tokAssignOr:
		return ebinInstrOr;
	case '<':
	case '>':
	case tokLEQ:
	case tokGEQ:
	case tokEQ:
	case tokNEQ:
	case tokULess:
	case tokUGreater:
	case tokULEQ:
	case tokUGEQ:
		return ebinInstrNop;
	case '*':
	case tokAssignMul:
		return ebinInstrMul;
	case '/':
	case '%':
	case tokAssignDiv:
	case tokAssignMod:
		return ebinInstrSDiv;
	case tokUDiv:
	case tokUMod:
	case tokAssignUDiv:
	case tokAssignUMod:
		return ebinInstrDiv;
	case tokLShift:
	case tokAssignLSh:
		return ebinInstrLLS;
	case tokRShift:
	case tokAssignRSh:
		return ebinInstrARS;
	case tokURShift:
	case tokAssignURSh:
		return ebinInstrLRS;

	default:
		//error("Error: Invalid operator\n");
		errorInternal(101);
		return 0;
	}
}

STATIC
void GenReadIdent(int regDst, int opSz, int label)
{
	int instr = ebinInstrL;
	if (opSz == -1 || opSz == 1)
	{
		instr = ebinInstrLB;
	}
	else if (opSz == -2 || opSz == 2)
	{
		instr = ebinInstrLW;
	}
	GenPrintInstr2Operands(instr, 0,
												 regDst, 0,
												 ebinOpLabel, label);

	if (opSz == -1) {
		GenPrintInstr2Operands(ebinInstrMov, 0,
													 TEMP_REG_A, 0,
													 regDst, 0);
		GenPrintInstr2Operands(ebinInstrLRS, 0,
													 TEMP_REG_A, 0,
													 ebinOpConst, 7);
		GenPrintInstr2Operands(ebinInstrMul, 0,
													 TEMP_REG_A, 0,
													 ebinOpConst, 0xFFFFFF00);
		GenPrintInstr2Operands(ebinInstrOr, 0,
													 regDst, 0,
													 TEMP_REG_A, 0);
	} else if (opSz == -2) {
		GenPrintInstr2Operands(ebinInstrMov, 0,
													 TEMP_REG_A, 0,
													 regDst, 0);
		GenPrintInstr2Operands(ebinInstrLRS, 0,
													 TEMP_REG_A, 0,
													 ebinOpConst, 15);
		GenPrintInstr2Operands(ebinInstrMul, 0,
													 TEMP_REG_A, 0,
													 ebinOpConst, 0xFFFF0000);
		GenPrintInstr2Operands(ebinInstrOr, 0,
													 regDst, 0,
													 TEMP_REG_A, 0);
	}
}

STATIC
void GenReadLocal(int regDst, int opSz, int ofs)
{
	int instr = ebinInstrL;
	if (opSz == -1 || opSz == 1)
	{
		instr = ebinInstrLB;
	}
	else if (opSz == -2 || opSz == 2)
	{
		instr = ebinInstrLW;
	}
	GenPrintInstr2Operands(instr, 0,
												 regDst, 0,
												 ebinOpIndRegBp, ofs);

	if (opSz == -1) {
		GenPrintInstr2Operands(ebinInstrMov, 0,
													 TEMP_REG_A, 0,
													 regDst, 0);
		GenPrintInstr2Operands(ebinInstrLRS, 0,
													 TEMP_REG_A, 0,
													 ebinOpConst, 7);
		GenPrintInstr2Operands(ebinInstrMul, 0,
													 TEMP_REG_A, 0,
													 ebinOpConst, 0xFFFFFF00);
		GenPrintInstr2Operands(ebinInstrOr, 0,
													 regDst, 0,
													 TEMP_REG_A, 0);
	} else if (opSz == -2) {
		GenPrintInstr2Operands(ebinInstrMov, 0,
													 TEMP_REG_A, 0,
													 regDst, 0);
		GenPrintInstr2Operands(ebinInstrLRS, 0,
													 TEMP_REG_A, 0,
													 ebinOpConst, 15);
		GenPrintInstr2Operands(ebinInstrMul, 0,
													 TEMP_REG_A, 0,
													 ebinOpConst, 0xFFFF0000);
		GenPrintInstr2Operands(ebinInstrOr, 0,
													 regDst, 0,
													 TEMP_REG_A, 0);
	}
}

STATIC
void GenReadIndirect(int regDst, int regSrc, int opSz)
{
	int instr = ebinInstrL;
	if (opSz == -1 || opSz == 1)
	{
		instr = ebinInstrLB;
	}
	else if (opSz == -2 || opSz == 2)
	{
		instr = ebinInstrLW;
	}
	GenPrintInstr2Operands(instr, 0,
												 regDst, 0,
												 regSrc + ebinOpIndReg0, 0);

	if (opSz == -1) {
		GenPrintInstr2Operands(ebinInstrMov, 0,
													 TEMP_REG_A, 0,
													 regDst, 0);
		GenPrintInstr2Operands(ebinInstrLRS, 0,
													 TEMP_REG_A, 0,
													 ebinOpConst, 7);
		GenPrintInstr2Operands(ebinInstrMul, 0,
													 TEMP_REG_A, 0,
													 ebinOpConst, 0xFFFFFF00);
		GenPrintInstr2Operands(ebinInstrOr, 0,
													 regDst, 0,
													 TEMP_REG_A, 0);
	} else if (opSz == -2) {
		GenPrintInstr2Operands(ebinInstrMov, 0,
													 TEMP_REG_A, 0,
													 regDst, 0);
		GenPrintInstr2Operands(ebinInstrLRS, 0,
													 TEMP_REG_A, 0,
													 ebinOpConst, 15);
		GenPrintInstr2Operands(ebinInstrMul, 0,
													 TEMP_REG_A, 0,
													 ebinOpConst, 0xFFFF0000);
		GenPrintInstr2Operands(ebinInstrOr, 0,
													 regDst, 0,
													 TEMP_REG_A, 0);
	}
}

STATIC
void GenWriteIdent(int regSrc, int opSz, int label)
{
	int instr = ebinInstrS;
	if (opSz == -1 || opSz == 1)
	{
		instr = ebinInstrSB;
	}
	else if (opSz == -2 || opSz == 2)
	{
		instr = ebinInstrSW;
	}
	GenPrintInstr2Operands(instr, 0,
												 ebinOpLabel, label,
												 regSrc, 0);
}

STATIC
void GenWriteLocal(int regSrc, int opSz, int ofs)
{
	int instr = ebinInstrS;
	if (opSz == -1 || opSz == 1)
	{
		instr = ebinInstrSB;
	}
	else if (opSz == -2 || opSz == 2)
	{
		instr = ebinInstrSW;
	}
	GenPrintInstr2Operands(instr, 0,
												 ebinOpIndRegBp, ofs,
												 regSrc, 0);
}

STATIC
void GenWriteIndirect(int regDst, int regSrc, int opSz)
{
	int instr = ebinInstrS;
	if (opSz == -1 || opSz == 1)
	{
		instr = ebinInstrSB;
	}
	else if (opSz == -2 || opSz == 2)
	{
		instr = ebinInstrSW;
	}
	GenPrintInstr2Operands(instr, 0,
												 regDst + ebinOpIndReg0, 0,
												 regSrc, 0);
}

STATIC
void GenIncDecIdent(int regDst, int opSz, int label, int tok)
{
	int instr = ebinInstrAdd;

	if (tok != tokInc)
		instr = ebinInstrSub;

	GenReadIdent(regDst, opSz, label);
	GenPrintInstr2Operands(instr, 0,
												 regDst, 0,
												 ebinOpConst, 1);
	GenWriteIdent(regDst, opSz, label);
	GenExtendRegIfNeeded(regDst, opSz);
}

STATIC
void GenIncDecLocal(int regDst, int opSz, int ofs, int tok)
{
	int instr = ebinInstrAdd;

	if (tok != tokInc)
		instr = ebinInstrSub;

	GenReadLocal(regDst, opSz, ofs);
	GenPrintInstr2Operands(instr, 0,
												 regDst, 0,
												 ebinOpConst, 1);
	GenWriteLocal(regDst, opSz, ofs);
	GenExtendRegIfNeeded(regDst, opSz);
}

STATIC
void GenIncDecIndirect(int regDst, int regSrc, int opSz, int tok)
{
	int instr = ebinInstrAdd;

	if (tok != tokInc)
		instr = ebinInstrSub;

	GenReadIndirect(regDst, regSrc, opSz);
	GenPrintInstr2Operands(instr, 0,
												 regDst, 0,
												 ebinOpConst, 1);
	GenWriteIndirect(regSrc, regDst, opSz);
	GenExtendRegIfNeeded(regDst, opSz);
}

STATIC
void GenPostIncDecIdent(int regDst, int opSz, int label, int tok)
{
	int instr = ebinInstrAdd;

	if (tok != tokPostInc)
		instr = ebinInstrSub;

	GenReadIdent(regDst, opSz, label);
	GenPrintInstr2Operands(instr, 0,
												 regDst, 0,
												 ebinOpConst, 1);
	GenWriteIdent(regDst, opSz, label);
	GenPrintInstr2Operands(instr, 0,
												 regDst, 0,
												 ebinOpConst, -1);
	GenExtendRegIfNeeded(regDst, opSz);
}

STATIC
void GenPostIncDecLocal(int regDst, int opSz, int ofs, int tok)
{
	int instr = ebinInstrAdd;

	if (tok != tokPostInc)
		instr = ebinInstrSub;

	GenReadLocal(regDst, opSz, ofs);
	GenPrintInstr2Operands(instr, 0,
												 regDst, 0,
												 ebinOpConst, 1);
	GenWriteLocal(regDst, opSz, ofs);
	GenPrintInstr2Operands(instr, 0,
												 regDst, 0,
												 ebinOpConst, -1);
	GenExtendRegIfNeeded(regDst, opSz);
}

STATIC
void GenPostIncDecIndirect(int regDst, int regSrc, int opSz, int tok)
{
	int instr = ebinInstrAdd;

	if (tok != tokPostInc)
		instr = ebinInstrSub;

	GenReadIndirect(regDst, regSrc, opSz);
	GenPrintInstr2Operands(instr, 0,
												 regDst, 0,
												 ebinOpConst, 1);
	GenWriteIndirect(regSrc, regDst, opSz);
	GenPrintInstr2Operands(instr, 0,
												 regDst, 0,
												 ebinOpConst, -1);
	GenExtendRegIfNeeded(regDst, opSz);
}

int CanUseTempRegs;
int TempsUsed;
int GenWreg = ebinOpReg0; // current working register (R0, R1, R2, etc)
int GenLreg, GenRreg; // left operand register and right operand register after GenPopReg()

/*
	General idea behind GenWreg, GenLreg, GenRreg:

	- In expressions w/o function calls:

		Subexpressions are evaluated in R0, R1, R2, ..., R<MAX_TEMP_REGS>. If those registers
		aren't enough, the stack is used additionally.

		The expression result ends up in R0, which is handy for returning from
		functions.

		In the process, GenWreg is the current working register and is one of: R0, R1, R2, ... .
		All unary operators are evaluated in the current working register.

		GenPushReg() and GenPopReg() advance GenWreg as needed when handling binary operators.

		GenPopReg() sets GenWreg, GenLreg and GenRreg. GenLreg and GenRreg are the registers
		where the left and right operands of a binary operator are.

		When the exression runs out of the temporary registers, the stack is used. While it is being
		used, GenWreg remains equal to the last temporary register, and GenPopReg() sets GenLreg = TEMP_REG_A.
		Hence, after GenPopReg() the operands of the binary operator are always in registers and can be
		directly manipulated with.

		Following GenPopReg(), binary operator evaluation must take the left and right operands from
		GenLreg and GenRreg and write the evaluated result into GenWreg. Care must be taken as GenWreg
		will be the same as either GenLreg (when the popped operand comes from R1-R<MAX_TEMP_REGS>)
		or GenRreg (when the popped operand comes from the stack in TEMP_REG_A).

	- In expressions with function calls:

		GenWreg is always R0. R1-R<MAX_TEMP_REGS> are not used. Instead the stack and TEMP_REG_A and TEMP_REG_B
		are used.
*/

STATIC
void GenWregInc(int inc)
{
	if (inc > 0)
	{
		// Advance the current working register to the next available temporary register
		GenWreg++;
	}
	else
	{
		// Return to the previous current working register
		GenWreg--;
	}
}

STATIC
void GenPushReg(void)
{
	if (CanUseTempRegs && TempsUsed < MAX_TEMP_REGS)
	{
		GenWregInc(1);
		TempsUsed++;
		return;
	}

	GenPrintInstr1Operand(ebinInstrPush, 0,
												GenWreg, 0);

	TempsUsed++;
}

STATIC
void GenPopReg(void)
{
	TempsUsed--;

	if (CanUseTempRegs && TempsUsed < MAX_TEMP_REGS)
	{
		GenRreg = GenWreg;
		GenWregInc(-1);
		GenLreg = GenWreg;
		return;
	}

	GenPrintInstr1Operand(ebinInstrPop, 0,
												TEMP_REG_A, 0);

	GenLreg = TEMP_REG_A;
	GenRreg = GenWreg;
}

STATIC
void GenPrep(int* idx)
{
	int tok;
	int oldIdxRight, oldIdxLeft, t0, t1;

	if (*idx < 0)
		//error("GenPrep(): idx < 0\n");
		errorInternal(100);

	tok = stack[*idx][0];

	oldIdxRight = --*idx;

	switch (tok)
	{
	case tokAssignMul:
	case tokUDiv:
	case tokUMod:
	case tokAssignUDiv:
	case tokAssignUMod:
		if (stack[oldIdxRight][0] == tokNumInt || stack[oldIdxRight][0] == tokNumUint)
		{
			unsigned m = truncUint(stack[oldIdxRight][1]);
			if (m && !(m & (m - 1)))
			{
				// Change multiplication to left shift, this helps indexing arrays of ints/pointers/etc
				if (tok == tokAssignMul)
				{
					t1 = 0;
					while (m >>= 1) t1++;
					stack[oldIdxRight][1] = t1;
					tok = tokAssignLSh;
				}
				// Change unsigned division to right shift and unsigned modulo to bitwise and
				else if (tok == tokUMod || tok == tokAssignUMod)
				{
					stack[oldIdxRight][1] = (int)(m - 1);
					tok = (tok == tokUMod) ? '&' : tokAssignAnd;
				}
				else
				{
					t1 = 0;
					while (m >>= 1) t1++;
					stack[oldIdxRight][1] = t1;
					tok = (tok == tokUDiv) ? tokURShift : tokAssignURSh;
				}
				stack[oldIdxRight + 1][0] = tok;
			}
		}
	}

	switch (tok)
	{
	case tokNumUint:
		stack[oldIdxRight + 1][0] = tokNumInt; // reduce the number of cases since tokNumInt and tokNumUint are handled the same way
		// fallthrough
	case tokNumInt:
	case tokIdent:
	case tokLocalOfs:
		break;

	case tokPostAdd:
	case tokPostSub:
	case '/':
	case '-':
	case '%':
	case tokUDiv:
	case tokUMod:
	case tokLShift:
	case tokRShift:
	case tokURShift:
	case tokLogAnd:
	case tokLogOr:
	case tokComma: {
		int xor;
	//case '-':
		switch(tok) {
			case '-':
				xor = '-' ^ tokRevMinus; break;
		}
		tok ^= xor;
		GenPrep(idx);
		// fallthrough
	}
	case tokShortCirc:
	case tokGoto:
	case tokUnaryStar:
	case tokInc:
	case tokDec:
	case tokPostInc:
	case tokPostDec:
	case '~':
	case tokUnaryPlus:
	case tokUnaryMinus:
	case tok_Bool:
	case tokVoid:
	case tokUChar:
	case tokSChar:
	case tokShort:
	case tokUShort:
		GenPrep(idx);
		break;

	case '=':
	//case '-':
	case tokAssignAdd:
	case tokAssignSub:
	case tokAssignMul:
	case tokAssignDiv:
	case tokAssignUDiv:
	case tokAssignMod:
	case tokAssignUMod:
	case tokAssignLSh:
	case tokAssignRSh:
	case tokAssignURSh:
	case tokAssignAnd:
	case tokAssignXor:
	case tokAssignOr:
		GenPrep(idx);
		oldIdxLeft = *idx;
		GenPrep(idx);
		// If the left operand is an identifier (with static or auto storage), swap it with the right operand
		// and mark it specially, so it can be used directly
		if ((t0 = stack[oldIdxLeft][0]) == tokIdent || t0 == tokLocalOfs)
		{
			t1 = stack[oldIdxLeft][1];
			memmove(stack[oldIdxLeft], stack[oldIdxLeft + 1], (oldIdxRight - oldIdxLeft) * sizeof(stack[0]));
			stack[oldIdxRight][0] = (t0 == tokIdent) ? tokRevIdent : tokRevLocalOfs;
			stack[oldIdxRight][1] = t1;
		}
		break;

	case '+':
	//case '-':
	case '*':
	case '&':
	case '^':
	case '|':
	case tokEQ:
	case tokNEQ:
	case '<':
	case '>':
	case tokLEQ:
	case tokGEQ:
	case tokULess:
	case tokUGreater:
	case tokULEQ:
	case tokUGEQ:
		GenPrep(idx);
		oldIdxLeft = *idx;
		GenPrep(idx);
		// If the right operand isn't a constant, but the left operand is, swap the operands
		// so the constant can become an immediate right operand in the instruction
		t1 = stack[oldIdxRight][0];
		t0 = stack[oldIdxLeft][0];
		if (t1 != tokNumInt && t0 == tokNumInt)
		{
			int xor;

			t1 = stack[oldIdxLeft][1];
			memmove(stack[oldIdxLeft], stack[oldIdxLeft + 1], (oldIdxRight - oldIdxLeft) * sizeof(stack[0]));
			stack[oldIdxRight][0] = t0;
			stack[oldIdxRight][1] = t1;

			switch (tok)
			{
			case '<':
			case '>':
				xor = '<' ^ '>'; break;
			case tokLEQ:
			case tokGEQ:
				xor = tokLEQ ^ tokGEQ; break;
			case tokULess:
			case tokUGreater:
				xor = tokULess ^ tokUGreater; break;
			case tokULEQ:
			case tokUGEQ:
				xor = tokULEQ ^ tokUGEQ; break;
			case '-':
				xor = '-' ^ tokRevMinus; break;
			default:
				xor = 0; break;
			}
			tok ^= xor;
		}
		if (stack[oldIdxRight][0] == tokNumInt)
		{
			unsigned m = truncUint(stack[oldIdxRight][1]);
			switch (tok)
			{
			case '*':
				// Change multiplication to left shift, this helps indexing arrays of ints/pointers/etc
				if (m && !(m & (m - 1)))
				{
					t1 = 0;
					while (m >>= 1) t1++;
					stack[oldIdxRight][1] = t1;
					tok = tokLShift;
				}
				break;
			case tokULEQ:
				// Change left <= const to left < const+1, but const+1 must be <=0xFFFFFFFFu
				if (m != 0xFFFFFFFF)
				{
					stack[oldIdxRight][1]++;
					tok = tokULess;
				}
				break;
			case tokUGEQ:
				// Change left >= const to left > const-1, but const-1 must be >=0u
				if (m)
				{
					stack[oldIdxRight][1]--;
					tok = tokUGreater;
				}
				break;
			}
		}
		stack[oldIdxRight + 1][0] = tok;
		break;

	case ')':
		while (stack[*idx][0] != '(')
		{
			GenPrep(idx);
			if (stack[*idx][0] == ',')
				--*idx;
		}
		--*idx;
		break;

	default:
		//error("GenPrep: unexpected token %s\n", GetTokenName(tok));
		errorInternal(101);
	}
}

STATIC
void GenCmp(int* idx, int instr)
{
	// constness: 0 = zero const, 1 = non-zero const, 2 = non-const
	int constness = (stack[*idx - 1][0] == tokNumInt) ? (stack[*idx - 1][1] != 0) : 2;
	int constval = (constness == 1) ? truncInt(stack[*idx - 1][1]) : 0;
	// condbranch: 0 = no conditional branch, 1 = branch if true, 2 = branch if false
	int condbranch = (*idx + 1 < sp) ? (stack[*idx + 1][0] == tokIf) + (stack[*idx + 1][0] == tokIfNot) * 2 : 0;
	int label = condbranch ? stack[*idx + 1][1] : 0;

	GenStartCommentLine(); printf2("GenCmp const=%d cond=%d\n",constness,condbranch);

	uint32_t and,xor;

	switch(instr) {
		case ebinInstrJE	: and=EF_E,xor=EF_X; break;
		case ebinInstrJA	: and=EF_A,xor=EF_X; break;
		case ebinInstrJB	: and=EF_B,xor=EF_X; break;
		case ebinInstrJZ	: and=EF_Z,xor=EF_X; break;
		case ebinInstrJNZ	: and=EF_Z,xor=EF_Z; break;
		case ebinInstrJNE	: and=EF_E,xor=EF_E; break;
		
		case ebinInstrJAE	: and=EF_E|EF_A,xor=EF_X; break;
		case ebinInstrJBE	: and=EF_E|EF_B,xor=EF_X; break;
	}

	if (constness == 2)
		GenPopReg();

	if (condbranch)
	{
		if (constness == 2)
			GenPrintInstr2Operands(ebinInstrIf, 0,
														 GenLreg, 0,
														 GenRreg, 0);
		else
			GenPrintInstr2Operands(ebinInstrIf, 0,
														 GenWreg, 0,
														 ebinOpConst, constval);
		if(condbranch != 1) 
			GenPrintInstr2Operands(ebinInstrXor, 0,
														 ebinOpRegFlags, 0,
														 ebinOpConst, and);
		GenPrintInstr2Operands(ebinInstrXor, 0,
														 ebinOpRegFlags, 0,
														 ebinOpConst, xor);
		GenPrintInstr1Operand(instr, 0,
														 ebinOpNumLabel, label);
	}
	else
	{
													 
		// Slow, general, catch-all implementation
		if (constness == 2)
			GenPrintInstr2Operands(ebinInstrIf, 0,
														 GenLreg, 0,
														 GenRreg, 0);
		else
			GenPrintInstr2Operands(ebinInstrIf, 0,
														 GenWreg, 0,
														 ebinOpConst, constval);
		GenPrintInstr2Operands(ebinInstrMov, 0,
														 TEMP_REG_B, 0,
														 ebinOpRegFlags, 0);
		GenPrintInstr2Operands(ebinInstrAnd, 0,	
														 TEMP_REG_B, 0,
														 ebinOpConst, and);
		GenPrintInstr2Operands(ebinInstrXor, 0,
														 ebinOpRegFlags, 0,
														 ebinOpConst, xor);
		GenPrintInstr2Operands(ebinInstrMov, 0,
														 GenWreg, 0,
														 TEMP_REG_B, 0);
	}

	*idx += condbranch != 0;
}

STATIC
int GenIsCmp(int t)
{
	return
		t == '<' ||
		t == '>' ||
		t == tokGEQ ||
		t == tokLEQ ||
		t == tokULess ||
		t == tokUGreater ||
		t == tokUGEQ ||
		t == tokULEQ ||
		t == tokEQ ||
		t == tokNEQ;
}

// Improved register/stack-based code generator
// DONE: test 32-bit code generation
STATIC
void GenExpr0(void)
{
	int i;
	int gotUnary = 0;
	int maxCallDepth = 0;
	int callDepth = 0;
	int t = sp - 1;

	if (stack[t][0] == tokIf || stack[t][0] == tokIfNot || stack[t][0] == tokReturn)
		t--;
	GenPrep(&t);

	for (i = 0; i < sp; i++)
		if (stack[i][0] == '(')
		{
			if (++callDepth > maxCallDepth)
				maxCallDepth = callDepth;
		}
		else if (stack[i][0] == ')')
		{
			callDepth--;
		}

	CanUseTempRegs = maxCallDepth == 0;
	TempsUsed = 0;
	if (GenWreg != ebinOpReg0)
		errorInternal(102);

	for (i = 0; i < sp; i++)
	{
		int tok = stack[i][0];
		int v = stack[i][1];

#ifndef NO_ANNOTATIONS
		switch (tok)
		{
			case tokNumInt: GenStartCommentLine(); printf2("%d\n", truncInt(v)); break;
		//case tokNumUint: printf2(" ; %uu\n", truncUint(v)); break;
		case tokIdent: case tokRevIdent: GenStartCommentLine(); printf2("%s\n", IdentTable + v); break;
		case tokLocalOfs: case tokRevLocalOfs: GenStartCommentLine(); printf2("local ofs\n"); break;
		case ')': GenStartCommentLine(); printf2(") fxn call\n"); break;
		case tokUnaryStar: GenStartCommentLine(); printf2("* (read dereference)\n"); break;
		case '=': GenStartCommentLine(); printf2("= (write dereference)\n"); break;
		case tokShortCirc: GenStartCommentLine(); printf2("short-circuit "); break;
		case tokGoto: GenStartCommentLine(); printf2("sh-circ-goto "); break;
		case tokLogAnd: GenStartCommentLine(); printf2("short-circuit && target\n"); break;
		case tokLogOr: GenStartCommentLine(); printf2("short-circuit || target\n"); break;
		case tokIf: GenStartCommentLine(); printf2(__FUNCTION__ ); printf2(" if\n"); break;
		case tokIfNot: GenStartCommentLine(); printf2(__FUNCTION__ ); printf2(" !if\n"); break;
		case tokReturn: break;
		case tokRevMinus: GenStartCommentLine(); printf2("-r\n"); break;
		default: GenStartCommentLine(); printf2("%s\n", GetTokenName(tok)); break;
		}
#endif

		switch (tok)
		{
		// TBD??? forward tokNumInt and tokIdent to ',', push them directly, w/o extra moves
		case tokNumInt:
			if (!(i + 1 < sp && ((t = stack[i + 1][0]) == '+' ||
													 t == '-' ||
													 t == tokRevMinus ||
													 t == '*' ||
													 t == '&' ||
													 t == '^' ||
													 t == '|' ||
													 t == tokLShift ||
													 t == tokRShift ||
													 t == tokURShift ||
													 t == '/' ||
													 t == tokUDiv ||
													 t == '%' ||
													 t == tokUMod ||
													 GenIsCmp(t))))
			{
				if (gotUnary)
					GenPushReg();

				GenPrintInstr2Operands(ebinInstrMov, 0,
															 GenWreg, 0,
															 ebinOpConst, v);
			}
			gotUnary = 1;
			break;

		case tokIdent:
			if (gotUnary)
				GenPushReg();
			if (!(i + 1 < sp && ((t = stack[i + 1][0]) == ')' ||
													 t == tokUnaryStar ||
													 t == tokInc ||
													 t == tokDec ||
													 t == tokPostInc ||
													 t == tokPostDec)))
			{
				GenPrintInstr2Operands(ebinInstrMov, 0,
															 GenWreg, 0,
															 ebinOpLabel, v);
			}
			gotUnary = 1;
			break;

		case tokLocalOfs:
			if (gotUnary)
				GenPushReg();
			if (!(i + 1 < sp && ((t = stack[i + 1][0]) == tokUnaryStar ||
													 t == tokInc ||
													 t == tokDec ||
													 t == tokPostInc ||
													 t == tokPostDec)))
			{
				GenPrintInstr2Operands(ebinInstrMov, 0,
															 GenWreg, 0,
															 ebinOpRegBp, 0);
				GenPrintInstr2Operands(ebinInstrAdd, 0,
															 GenWreg, 0,
															 ebinOpConst, v);
			}
			gotUnary = 1;
			break;

		case '(':
			if (gotUnary)
				GenPushReg();
			gotUnary = 0;
			break;

		case ',':
			break;

		case ')':
			if (stack[i - 1][0] == tokIdent)
			{
				GenPrintInstr1Operand(ebinInstrRCall, 0,
															ebinOpLabel, stack[i - 1][1]);
			}
			else
			{
				GenPrintInstr1Operand(ebinInstrCall, 0,
															GenWreg, 0);
			}
			GenGrowStack(-v);
			break;

		case tokUnaryStar:
			if (stack[i - 1][0] == tokIdent)
				GenReadIdent(GenWreg, v, stack[i - 1][1]);
			else if (stack[i - 1][0] == tokLocalOfs)
				GenReadLocal(GenWreg, v, stack[i - 1][1]);
			else
				GenReadIndirect(GenWreg, GenWreg, v);
			break;

		case tokUnaryPlus:
			break;
		case '~':
			GenPrintInstr1Operand(ebinInstrNot, 0,
														GenWreg, 0);
			break;
		case tokUnaryMinus:
			GenPrintInstr2Operands(ebinInstrRSub, 0,
														 GenWreg, 0,
														 ebinOpConst, 0);
			break;

		case '+':
		case '-':
		case tokRevMinus:
		case '*':
		case '&':
		case '^':
		case '|':
		case tokLShift:
		case tokRShift:
		case tokURShift:
		case '/':
		case tokUDiv:
		case '%':
		case tokUMod:
			if (stack[i - 1][0] == tokNumInt)
			{
				int instr = GenGetBinaryOperatorInstr(tok);
				//GenPopReg();
				GenPrintInstr2Operands(instr, 0,
															 GenWreg, 0,
															 ebinOpConst, stack[i - 1][1]);
			}
			else
			{
				int instr = GenGetBinaryOperatorInstr(tok);
				GenPopReg();
				GenPrintInstr2Operands(ebinInstrMov, 0,
															 TEMP_REG_B, 0,
															 GenLreg, 0);
				GenPrintInstr2Operands(instr, 0,
															 TEMP_REG_B, 0,
															 GenRreg, 0);
				GenPrintInstr2Operands(ebinInstrMov, 0,
															 GenWreg, 0,
															 TEMP_REG_B, 0);
			}
			if (tok == '%' || tok == tokUMod)
				GenPrintInstr2Operands(ebinInstrMov, 0,
															 GenWreg, 0,
															 ebinOpRegY, 0);
			break;
		case tokInc:
		case tokDec:
			if (stack[i - 1][0] == tokIdent)
			{
				GenIncDecIdent(GenWreg, v, stack[i - 1][1], tok);
			}
			else if (stack[i - 1][0] == tokLocalOfs)
			{
				GenIncDecLocal(GenWreg, v, stack[i - 1][1], tok);
			}
			else
			{
				GenPrintInstr2Operands(ebinInstrMov, 0,
															 TEMP_REG_A, 0,
															 GenWreg, 0);
				GenIncDecIndirect(GenWreg, TEMP_REG_A, v, tok);
			}
			break;
		case tokPostInc:
		case tokPostDec:
			if (stack[i - 1][0] == tokIdent)
			{
				GenPostIncDecIdent(GenWreg, v, stack[i - 1][1], tok);
			}
			else if (stack[i - 1][0] == tokLocalOfs)
			{
				GenPostIncDecLocal(GenWreg, v, stack[i - 1][1], tok);
			}
			else
			{
				GenPrintInstr2Operands(ebinInstrMov, 0,
															 TEMP_REG_A, 0,
															 GenWreg, 0);
				GenPostIncDecIndirect(GenWreg, TEMP_REG_A, v, tok);
			}
			break;

		case tokPostAdd:
		case tokPostSub:
			{
				int instr = GenGetBinaryOperatorInstr(tok);
				GenPopReg();
				if (GenWreg == GenLreg)
				{
					GenPrintInstr2Operands(ebinInstrMov, 0,
																 TEMP_REG_B, 0,
																 GenLreg, 0);

					GenReadIndirect(GenWreg, TEMP_REG_B, v);
					GenPrintInstr2Operands(ebinInstrMov, 0,
																 TEMP_REG_A, 0,
																 GenWreg, 0);
					GenPrintInstr2Operands(instr, 0,
																 TEMP_REG_A, 0,
																 GenRreg, 0);
					GenWriteIndirect(TEMP_REG_B, TEMP_REG_A, v);
				}
				else
				{
					// GenWreg == GenRreg here
					GenPrintInstr2Operands(ebinInstrMov, 0,
																 TEMP_REG_B, 0,
																 GenRreg, 0);

					GenReadIndirect(GenWreg, GenLreg, v);
					GenPrintInstr2Operands(ebinInstrMov, 0,
																 TEMP_REG_B, 0,
																 GenWreg, 0);
					GenPrintInstr2Operands(instr, 0,
																 TEMP_REG_B, 0,
																 TEMP_REG_B, 0);
					GenWriteIndirect(GenLreg, TEMP_REG_B, v);
				}
			}
			break;

		case tokAssignAdd:
		case tokAssignSub:
		case tokAssignMul:
		case tokAssignAnd:
		case tokAssignXor:
		case tokAssignOr:
		case tokAssignLSh:
		case tokAssignRSh:
		case tokAssignURSh:
		case tokAssignDiv:
		case tokAssignUDiv:
		case tokAssignMod:
		case tokAssignUMod:
		//case '-':
		//case tokRevMinus:
			if (stack[i - 1][0] == tokRevLocalOfs || stack[i - 1][0] == tokRevIdent)
			{
				int instr = GenGetBinaryOperatorInstr(tok);

				if (stack[i - 1][0] == tokRevLocalOfs)
					GenReadLocal(TEMP_REG_B, v, stack[i - 1][1]);
				else
					GenReadIdent(TEMP_REG_B, v, stack[i - 1][1]);

				GenPrintInstr2Operands(ebinInstrMov, 0,
															 TEMP_REG_A, 0,
															 TEMP_REG_B, 0);
				GenPrintInstr2Operands(instr, 0,
															 TEMP_REG_A, 0,
															 GenWreg, 0);
				
				GenPrintInstr2Operands(ebinInstrMov, 0,
															 GenWreg, 0,
															 TEMP_REG_A, 0);

				if (tok == tokAssignMod || tok == tokAssignUMod)
					GenPrintInstr2Operands(ebinInstrMov, 0,
																 GenWreg, 0,
																 ebinOpRegY, 0);

				if (stack[i - 1][0] == tokRevLocalOfs)
					GenWriteLocal(GenWreg, v, stack[i - 1][1]);
				else
					GenWriteIdent(GenWreg, v, stack[i - 1][1]);
			}
			else
			{
				int instr = GenGetBinaryOperatorInstr(tok);
				int lsaved, rsaved;
				GenPopReg();
				if (GenWreg == GenLreg)
				{
					GenPrintInstr2Operands(ebinInstrMov, 0,
																 TEMP_REG_B, 0,
																 GenLreg, 0);
					lsaved = TEMP_REG_B;
					rsaved = GenRreg;
				}
				else
				{
					// GenWreg == GenRreg here
					GenPrintInstr2Operands(ebinInstrMov, 0,
																 TEMP_REG_B, 0,
																 GenRreg, 0);
					rsaved = TEMP_REG_B;
					lsaved = GenLreg;
				}

				GenReadIndirect(GenWreg, GenLreg, v); // destroys either GenLreg or GenRreg because GenWreg coincides with one of them
				GenPrintInstr2Operands(instr, 0,
															 GenWreg, 0,
															 rsaved, 0);

				if (tok == tokAssignMod || tok == tokAssignUMod)
					GenPrintInstr2Operands(ebinInstrMov, 0,
																 GenWreg, 0,
																 ebinOpRegY, 0);

				GenWriteIndirect(lsaved, GenWreg, v);
			}
			GenExtendRegIfNeeded(GenWreg, v);
			break;

		case '=':
			if (stack[i - 1][0] == tokRevLocalOfs)
			{
				GenWriteLocal(GenWreg, v, stack[i - 1][1]);
			}
			else if (stack[i - 1][0] == tokRevIdent)
			{
				GenWriteIdent(GenWreg, v, stack[i - 1][1]);
			}
			else
			{
				GenPopReg();
				GenWriteIndirect(GenLreg, GenRreg, v);
				if (GenWreg != GenRreg)
					GenPrintInstr2Operands(ebinInstrMov, 0,
																 GenWreg, 0,
																 GenRreg, 0);
			}
			GenExtendRegIfNeeded(GenWreg, v);
			break;

		case '<':				GenCmp(&i, ebinInstrJB	); break;
		case tokLEQ:			GenCmp(&i, ebinInstrJBE	); break;
		case '>':				GenCmp(&i, ebinInstrJA	); break;
		case tokGEQ:			GenCmp(&i, ebinInstrJAE	); break;
		case tokULess:			GenCmp(&i, ebinInstrJB	); break;
		case tokULEQ:			GenCmp(&i, ebinInstrJBE	); break;
		case tokUGreater:		GenCmp(&i, ebinInstrJA	); break;
		case tokUGEQ:			GenCmp(&i, ebinInstrJAE	); break;
		case tokEQ:				GenCmp(&i, ebinInstrJE	); break;
		case tokNEQ:			GenCmp(&i, ebinInstrJNE	); break;

		case tok_Bool:
			GenPrintInstr2Operands(ebinInstrIf, 0,
														 GenWreg, 0,
														 GenWreg, 0);
			GenPrintInstr2Operands(ebinInstrMov, 0,
														 GenWreg, 0,
														 ebinOpRegFlags, 1);
			GenPrintInstr2Operands(ebinInstrLRS, 0,
														 GenWreg, 0,
														 ebinOpConst, 6);
			GenPrintInstr2Operands(ebinInstrAnd, 0,
														 GenWreg, 0,
														 ebinOpRegFlags, 1);
			break;

		case tokSChar:
			GenPrintInstr2Operands(ebinInstrMov, 0,
														 TEMP_REG_A, 0,
														 GenWreg, 0);
			GenPrintInstr2Operands(ebinInstrLRS, 0,
														 TEMP_REG_A, 0,
														 ebinOpConst, 7);
														 
			GenPrintInstr2Operands(ebinInstrMul, 0,
														 TEMP_REG_A, 0,
														 ebinOpConst, 0xFFFFFF00);
			GenPrintInstr2Operands(ebinInstrOr, 0,
														 GenWreg, 0,
														 TEMP_REG_A, 0);
			break;
		case tokUChar:
			GenPrintInstr2Operands(ebinInstrAnd, 0,
														 GenWreg, 0,
														 ebinOpConst, 0xFF);
			break;
		case tokShort:
			GenPrintInstr2Operands(ebinInstrLRS, 0,
														 TEMP_REG_A, 0,
														 ebinOpConst, 15);
														 
			GenPrintInstr2Operands(ebinInstrMul, 0,
														 TEMP_REG_A, 0,
														 ebinOpConst, 0xFFFF0000);
			GenPrintInstr2Operands(ebinInstrOr, 0,
														 GenWreg, 0,
														 TEMP_REG_A, 0);
			break;
		case tokUShort:
			GenPrintInstr2Operands(ebinInstrAnd, 0,
														 GenWreg, 0,
														 ebinOpConst, 0xFFFF);
			break;

		case tokShortCirc:
#ifndef NO_ANNOTATIONS
			if (v >= 0)
				printf2("&&\n");
			else
				printf2("||\n");
#endif
			if (v >= 0) {
				printf2("\t; tokShortCirc +\n");
				GenJumpIfZero(v); // &&
			} else {
				printf2("\t; tokShortCirc -\n");
				GenJumpIfNotZero(-v); // ||
			}
			gotUnary = 0;
			break;
		case tokGoto:
#ifndef NO_ANNOTATIONS
			printf2("goto\n");
#endif
			GenJumpUncond(v);
			gotUnary = 0;
			break;
		case tokLogAnd:
			printf2("\t; tokLogAnd\n");
			GenNumLabel(v);
			break;
		case tokLogOr:
			GenNumLabel(v);
			break;

		case tokVoid:
			gotUnary = 0;
			break;

		case tokRevIdent:
		case tokRevLocalOfs:
		case tokComma:
			break;
		case tokReturn:
			printf2("\t; tokReturn\n");
			break;

		case tokIf:
			printf2("\t; tokIf\n");
			GenJumpIfNotZero(stack[i][1]);
			break;
		case tokIfNot:
			printf2("\t; tokIfNot\n");
			GenJumpIfZero(stack[i][1]);
			break;

		default:
			//error("Error: Internal Error: GenExpr0(): unexpected token %s\n", GetTokenName(tok));
			errorInternal(103);
			break;
		}
	}

	if (GenWreg != ebinOpReg0)
		printf("GenWreg=%d\n",GenWreg), errorInternal(104);
}

STATIC
void GenDumpChar(int ch)
{
	if(ch > 0) printf2("\tdb($%u)\n", ch & 0xFFu);
}

STATIC
void GenExpr(void)
{
	if (GenExterns)
	{
		int i;
		for (i = 0; i < sp; i++)
			if (stack[i][0] == tokIdent && !isdigit(IdentTable[stack[i][1]]))
			GenAddGlobal(IdentTable + stack[i][1], 2);
	}
	GenExpr0();
}

STATIC
void GenFin(void)
{
	if (StructCpyLabel)
	{
		int lbl = LabelCnt++;

		puts2(CodeHeaderFooter[0]);

		GenNumLabel(StructCpyLabel);
		puts2("\tpush(%r1)\n"
					"\tpush(%r2)\n"
					"\tpush(%r3)\n"
					"\tmov(%r1,d@[%sp:$16])\n" // size
					"\tmov(%r2,d@[%sp:$20])\n" // source
					"\tmov(%r3,d@[%sp:$24])"); // destination
		GenNumLabel(lbl);
		puts2("\tmov(%r0,b%r2)\n"
					"\tadd(%r2,$1)\n"
					"\tadd(%r1,$-1)\n"
					"\tmov(b%r3,%r0)\n"
					"\tadd(%r3,$1)\n"
					"\tcmp(%r1,$0)");
		printf2("\tjne("); GenPrintNumLabel(lbl);
		puts2(")");
		puts2("\tmov(%r0,d@[%sp:$24])\n" // destination
					"\tpop(%r3)\n"
					"\tpop(%r2)\n"
					"\tpop(%r1)\n"
					"\tret()");

		puts2(CodeHeaderFooter[1]);
	}
	
	if (GenExterns)
	{
		int i = 0;

		puts2("");
		while (i < GlobalsTableLen)
		{
			if (GlobalsTable[i] == 2)
			{
				//printf2("\textern\t");
				//GenPrintLabel(GlobalsTable + i + 2);
				printf2("\tglobal(._%s)\n", GlobalsTable + i + 2);
				//puts2("");
			}
			i += GlobalsTable[i + 1] + 2;
		}
	}
}
