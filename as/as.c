#define EBIN_MODULE_INCLUDE
#include "../ebin.h"
#include <stdio.h>
#include <stdlib.h>

#define INVALID		-0x7fffffff
#define SIZE_SYMBOL (64 + (16 * 4))

typedef struct {
	ebin_fetch e_data;
	int mode;
} binary_t;


typedef struct {
	uint32_t d,t,s;
} argments_t;

typedef struct {
	char name[64];
	uint32_t address;
} label_t;

typedef struct {
	char name[64];
	unsigned int attribute[16];
} symbol_t;

label_t label[4096];
symbol_t symbol[4096];
uint32_t replace_addr[65536];
int last_label = 0;
int last_replace = 0;
int last_symbol = 0;
int now_address = 0;
int org_offset = 0;
int now_line = 0;
int scan_label = 0;
int scan_symbol = 0;
char *image;

int hash(char *sz)
{
	int i;
	for(i = 0;*sz;sz++) i += *sz;
	return i;
}

void errmsg(char *sz)
{
	printf("%s at line %d\n",sz, now_line);
	exit(hash(sz));
}

char *gpc_getop(char *s)
{
	char *str = (char *)malloc(4096);

	int i;
	char *p = s;

	for(;*p==' '||*p==0x09;p++);
	for(i = 0;*p!='('&&*p;p++,i++) {
		str[i]=*p;
	}

	str[i]=0;

	return str;
}

char *gpc_getargstr(char *s,int num)
{
	char *str = (char *)malloc(4096);

	int i;
	char *p = s;

	for(;*p==' '||*p==0x09;p++);
	for(i = 0;*p!='(';p++,i++);

	p++;

	for(i=0;i<num;i++) {
		for(;*p!=','&&*p!=')';p++);
		p++;
	}

	for(i=0;*p!=','&&*p!=')';p++,i++) {
		str[i]=*p;
	}

	str[i]=0;

	return str;
}

char *gpc_getargstrEx(char *s)
{
	char *str = (char *)malloc(4096);
	
	int i;
	char *p = s;

	for(;*p==' '||*p==0x09;p++);
	for(i = 0;*p!='(';p++,i++);

	p++;

	for(i=0;*p!=')';p++,i++) {
		str[i]=*p;
	}

	str[i]=0;

	return str;
}

int get_bytecode(char *s)
{
	char *d = gpc_getop(s);
	int i,j;
	
	for(j=0;j<strlen(d);j++) {
		if(d[j]>='A'&& d[j]<='Z') d[j]+=0x20;
	}
	
	for(i=0;i<sizeof(operation_list)/sizeof(ebin_operation);i++) {
		if(strcmp(d,operation_list[i].name)==0) return i;
	}
	
	return -1;
}

int get_instsize(char *s)
{
	int n = get_bytecode(s);
	
	return operation_size[operation_list[n].type];
}

char *skipspace(char *p)
{
	for (; *p == ' '; p++) { }
	return p;
}

int64_t get_immediate_sub(char **pp, int priority)
{
	char *p = *pp;
	int64_t i = INVALID, j;
	p = skipspace(p);

	if (*p == '+') {
		p = skipspace(p + 1);
		i = get_immediate_sub(&p, 0);
	} else if (*p == '-') {
		p = skipspace(p + 1);
		i = get_immediate_sub(&p, 0);
		if (i != INVALID) {
			i = - i;
		}
	} else if (*p == '~') {
		p = skipspace(p + 1);
		i = get_immediate_sub(&p, 0);
		if (i != INVALID) {
			i = ~i;
		}
	} else if (*p == '(') {
		p = skipspace(p + 1);
		i = get_immediate_sub(&p, 9);
		if (*p == ')') {
			p = skipspace(p + 1);
		} else {
			i = INVALID;
		}
	} else if ('0' <= *p && *p <= '9') { 
		i = strtol(p, &p, 0);
	} else {
		i = INVALID;
	}

	for (;;) {
		if (i == INVALID) {
			break;
		}
		p = skipspace(p);
		if (*p == '+' && priority > 2) {
			p = skipspace(p + 1);
			j = get_immediate_sub(&p, 2);
			if (j != INVALID) {
				i += j;
			} else {
				i = INVALID;
			}
		} else if (*p == '-' && priority > 2) {
			p = skipspace(p + 1);
			j = get_immediate_sub(&p, 2);
			if (j != INVALID) {
				i -= j;
			} else {
				i = INVALID;
			}
		} else if (*p == '*' && priority > 1) {
			p = skipspace(p + 1);
			j = get_immediate_sub(&p, 1);
			if (j != INVALID) {
				i *= j;
			} else {printf("DEFINE %s : %08x\n",label[last_label].name,now_address);
				i = INVALID;
			}
		} else if (*p == '/' && priority > 1) {
			p = skipspace(p + 1);
			j = get_immediate_sub(&p, 1);
			if (j != INVALID && j != 0) {
				i /= j;
			} else {
				i = INVALID;
			}
		} else if (*p == '%' && priority > 1) {
			p = skipspace(p + 1);
			j = get_immediate_sub(&p, 1);
			if (j != INVALID && j != 0) {
				i %= j;
			} else {
				i = INVALID;
			}
		} else if (*p == '<' && p[1] == '<' && priority > 3) {
			p = skipspace(p + 2);
			j = get_immediate_sub(&p, 3);
			if (j != INVALID && j != 0) {
				i <<= j;
			} else {
				i = INVALID;
			}
		} else if (*p == '>' && p[1] == '>' && priority > 3) {
			p = skipspace(p + 2);
			j = get_immediate_sub(&p, 3);
			if (j != INVALID && j != 0) {
				i >>= j;
			} else {
				i = INVALID;
			}
		} else if (*p == '&' && priority > 4) {
			p = skipspace(p + 1);
			j = get_immediate_sub(&p, 4);
			if (j != INVALID) {
				i &= j;
			} else {
				i = INVALID;
			}
		} else if (*p == '^' && priority > 5) {
			p = skipspace(p + 1);
			j = get_immediate_sub(&p, 5);
			if (j != INVALID) {
				i ^= j;
			} else {
				i = INVALID;
			}
		} else if (*p == '|' && priority > 6) {
			p = skipspace(p + 1);
			j = get_immediate_sub(&p, 6);
			if (j != INVALID) {
				i |= j;
			} else {
				i = INVALID;
			}
		} else {
			break;
		}
	}
	p = skipspace(p);
	*pp = p;
	return i;
}

int get_immediate(char *s)
{
	char *p = (char *)malloc(4096);
	strcpy(p,s);
	
	return get_immediate_sub(&p,10);
}

unsigned int get_labeladdr(char *s)
{
	int i;
	char *d = s;

	for(i=0;i<4096;i++) {
		if(strcmp(d,label[i].name)==0) return label[i].address;
	}

	return -1;
}

void replacepoint(int offset)
{
	replace_addr[last_replace] = now_address + offset;
	last_replace++;
}

int get_symboladdr(char *s)
{
	int i;
	char *d = s;

	for(i=0;i<4096;i++) {
		if(symbol[i].name[0] != 0 && strcmp(d,symbol[i].name)==0) return i;
	}

	return -1;
}

argments_t get_argments_sub(char *d, int issub)
{
	int64_t t,u,i;
	char *e;
	argments_t sub;
	
	e = d + 1;
	
	switch(*d) {
	case '%': // %(b/w/d)0 ~ %31 REGISTERS
		t = EFETCH_IREG;
		if(*e == 'b') i = 0;
		if(*e == 'w') i = 1;
		if(*e == 'd') i = 2;
		u = get_immediate(e+1);
		if(u >= 36) errmsg("register number exceeds 35");
		break;
	case 'b': case 'w': case 'd':
		if(issub) errmsg("memory data is used for memory address");
		if(*d == 'b') i = 0;
		if(*d == 'w') i = 1;
		if(*d == 'd') i = 2;
		sub = get_argments_sub(e,1);
		t = sub.t | 1;
		u = sub.d;
		break;
	case '$': // $0 ~ IMMEDIATES
		u = get_immediate(e);
		t = EFETCH_IIMM;
		i = 2;
		break;
	case '#': // ASCII CHARCTERS
		t = EFETCH_IIMM;
		i = 2;
		u = e[0];
		break;
	case '.': // LABEL
		t = EFETCH_IIMM;
		u = get_labeladdr(d);
		if(scan_label && !scan_symbol) {
			int q = get_labeladdr(d);
			int m;
			if(q < 0) {
				m = get_symboladdr(d);
				if(m >= 0) {
					u = symbol[m].attribute[0];
				} else {
					errmsg("an undefined label is used");
				}
			} else {
				u = q;
			}
			if(scan_label && !scan_symbol) printf("READ(MEM) %s : %08lx\n",d,u);
			if(scan_label && !scan_symbol) replacepoint(4);
		}
		i = 2;
		break;
	default:
		errmsg("unknown argument");
	}
	
	argments_t ret;
	
	if(u < 0) {
		if(u < -2147483648) u = -2147483648;
		int32_t ud = u;
		*((int32_t *)&(ret.d)) = ud;
	} else {
		if(u > 2147483647) u = 2147483647;
		ret.d = u;
	}
	ret.t = t;
	ret.s = i;
	
	return ret;
}

argments_t get_argments(char *s)
{
	char *d = gpc_getargstrEx(s);
	
	return get_argments_sub(d,0);
}

void assenble_fetch(FILE *in, FILE *out, int mode, int mode2)
{
	int i,j;
	char s[4096];
	fgets(s,4096,in);
	
	scan_label = mode;
	scan_symbol = mode2;
	
	int n = get_bytecode(s);
	if(s[0]=='.' && !mode && !mode2) {
		for(i=0;i<4096 && s[i]!=0x0d && s[i]!=0x0a && s[i]!=0;i++) {
			label[last_label].name[i]=s[i];
		}
		label[last_label].address = now_address + org_offset;
		printf("DEFINE %s : %08x\n",label[last_label].name,now_address);
		last_label++;
	} else {
		if(n >= 0) {
			ebin_fetch dat;
			memset(&dat,0,sizeof(ebin_fetch));
			if(operation_list[n].type != 2) {
				argments_t arg = get_argments(s);
				switch(arg.t) {
				case EFETCH_IREG: case EFETCH_MREG:
					dat.e_argopt = arg.t | (arg.s << 4);
					dat.e_srcreg = arg.d;
					break;
				case EFETCH_IIMM: case EFETCH_MIMM:
					dat.e_argopt = arg.t | (arg.s << 4);
					e_write32(dat.e_srcimm,arg.d);
				}
			}
			e_write16(dat.e_opcode,n);
			int size = get_instsize(s);
			if(mode && !mode2) memcpy(image+now_address,&dat,size);
			now_address += size;
		} else {
			char *d = s;
			for(;*d==' '||*d=='\t';d++);
			for(j=0;j<strlen(d);j++) {
				if(d[j]>='A'&& d[j]<='Z') d[j]+=0x20;
			}
			if(strncmp(d,"org",3) == 0) {
				argments_t arg = get_argments(s);
				org_offset = arg.d;
			}
			if(strncmp(d,"equ",3) == 0) {
				if(!mode) {
					char *e = gpc_getargstr(s,1);
					argments_t arg = get_argments(s);
					for(i=0;i<4096 && s[i]!=0x0d && s[i]!=0x0a && s[i]!=0;i++) {
						label[last_label].name[i]=e[i];
					}
					label[last_label].address = arg.d;
					last_label++;
				}
			}
			if(strncmp(d,"db",2) == 0) {
				argments_t arg = get_argments(s);
				if(mode && !mode2) memcpy(image+now_address,&(arg.d),1);
				now_address += 1;
			}
			if(strncmp(d,"dw",2) == 0) {
				argments_t arg = get_argments(s);
				uint8_t le[2];
				e_write16(le,arg.d);
				if(mode && !mode2) memcpy(image+now_address,le,2);
				now_address += 2;
			}
			if(strncmp(d,"dd",2) == 0) {
				argments_t arg = get_argments(s);
				uint8_t le[4];
				e_write32(le,arg.d);
				if(mode && !mode2) memcpy(image+now_address,le,4);
				now_address += 4;
			}
			if(strncmp(d,"text",4) == 0) {
				char *e = gpc_getargstrEx(s);
				if(mode && !mode2) memcpy(image+now_address,e,strlen(e));
				now_address += strlen(e);
			}
			if(strncmp(d,"global",6) == 0) {
				char *e = gpc_getargstrEx(s);
				char strs[1024];
				argments_t arg;
				int addr;
				strcpy(strs,e);
				if(mode2) {
					addr = get_labeladdr(strs);
					if(addr < 0) {
						if(get_symboladdr(strs) >= 0) errmsg("symbols already declared");
						for(i=0;i<256 && s[i]!=0x0d && s[i]!=0x0a && s[i]!=0;i++) {
							symbol[last_symbol].name[i]=e[i];
						}
						symbol[last_symbol].attribute[0] = last_symbol + 0x40000000;
						printf("EXTERN %s : %08x\n",symbol[last_symbol].name,symbol[last_symbol].attribute[0]);
						last_symbol++;
					} else {
						if(get_symboladdr(strs) >= 0) errmsg("symbols already declared");
						arg = get_argments(s);
						if(arg.t != EFETCH_IIMM) errmsg("symbol must be const");
						for(i=0;i<256 && s[i]!=0x0d && s[i]!=0x0a && s[i]!=0;i++) {
							symbol[last_symbol].name[i]=e[i];
						}
						symbol[last_symbol].attribute[0] = addr - org_offset;
						printf("GLOBAL %s : %08x\n",symbol[last_symbol].name,symbol[last_symbol].attribute[0]);
						last_symbol++;
					}
				}
			}
			
			if(strncmp(d,"align",5) == 0) {
				int i;
				argments_t arg;
				arg = get_argments(s);
				if(arg.t != EFETCH_IIMM) errmsg("alignment boundary must be const");
				for(i = 0; ((now_address + i) % arg.d) != 0; i++);
				if(mode && !mode2) memset(image+now_address,0,i);
				now_address += i;
			}
			
			if(strncmp(d,"fill",4) == 0) {
				argments_t arg;
				arg = get_argments(s);
				if(arg.t != EFETCH_IIMM) errmsg("fill length must be const");
				if(mode && !mode2) memset(image+now_address,0,arg.d);
				now_address += arg.d;
			}
		}
	}
}

int main(int argc, char *argv[]) {
	int coff;
	
	if(argc < 4) {
		puts("usage>as <source-file> <binary-file> [-flat|-coff]");
		exit(1);
	}
	
	if(strcmp(argv[3],"-flat") == 0) coff = 0;
	else if(strcmp(argv[3],"-coff") == 0) coff = 1;
	else errmsg("an unknown format is specified");
	
	FILE *in  = fopen(argv[1],"rt");
	FILE *out = fopen(argv[2],"wb");
	
	if(!in) {
		puts("invalid open input file.");
		return 2;
	}
	
	if(!out) {
		puts("invalid open output file.");
		return 3;
	}
	
	now_address = 0;
	org_offset = 0;
	now_line = 1;
	
	do {
		assenble_fetch(in,out,0,0);
		now_line++;
	} while(!feof(in));
	
	now_address = 0;
	org_offset = 0;
	now_line = 1;
	
	fseek(in,0,SEEK_SET);
	
	do {
		assenble_fetch(in,out,1,1);
		now_line++;
	} while(!feof(in));
	
	fseek(in,0,SEEK_SET);
	
	image = (char *)malloc(now_address);
	
	now_address = 0;
	org_offset = 0;
	now_line = 1;
	
	do {
		assenble_fetch(in,out,1,0);
		now_line++;
	} while(!feof(in));
	
	if(coff) {
		
		uint32_t repaddr = 4 * 4 + last_symbol * SIZE_SYMBOL;
		fprintf(out,"EBIN");
		uint8_t ls_le[4];
		uint8_t ra_le[4];
		uint8_t lr_le[4];
		
		printf("entno : %d\nrepaddr : %x\nrepno : %d\n\n",last_symbol,repaddr,last_replace);
		
		e_write32(ls_le,last_symbol);
		e_write32(ra_le,repaddr);
		e_write32(lr_le,last_replace);
		
		fwrite(ls_le,1,4,out);
		fwrite(ra_le,1,4,out);
		fwrite(lr_le,1,4,out);
		
		for(int i = 0; i < last_symbol; i++) {
			fwrite(symbol[i].name,1,64,out);
			uint8_t le[4];
			for(int j = 0; j < 16; j++) {
				e_write32(le,symbol[i].attribute[j]);
				fwrite(le,1,4,out);
			}
		}
		
		for(int i = 0; i < last_replace; i++) {
			uint8_t le[4];
			e_write32(le,replace_addr[i]);
			fwrite(le,1,4,out);
		}
	}
	
	fwrite(image,1,now_address,out);
	
	fclose(in);
	fclose(out);
	
	return 0;
}
