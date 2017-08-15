#define EBIN_MODULE_INCLUDE
#include "../ebin.h"
#include <stdio.h>
#include <stdlib.h>

#define INVALID		-0x7fffffff

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

label_t label[4096];
int last_label = 0;
int now_address = 0;
int org_offset = 0;

char *gpc_getop(char *s)
{
	char *str = (char *)malloc(4096);

	int i;
	char *p = s;

	for(;*p==' '||*p==0x09;p++);
	for(i = 0;*p!='(';p++,i++) {
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
			} else {
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
		if(strcmp(d,label[i].name)==0) break;
	}

	if(i >= 4096) return 0;
	return label[i].address;
}

argments_t get_argments_sub(char *d)
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
		break;
	case 'b': case 'w': case 'd':
		if(*d == 'b') i = 0;
		if(*d == 'w') i = 1;
		if(*d == 'd') i = 2;
		sub = get_argments_sub(e);
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
		i = 2;
		break;
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
	int n = get_bytecode(s);
	char *d = gpc_getargstrEx(s);
	
	return get_argments_sub(d);
}


void assenble_fetch(FILE *in, FILE *out, int mode)
{
	int i,j;
	char s[4096];
	fgets(s,4096,in);
	int n = get_bytecode(s);
	if(s[0]=='.' && !mode) {
		for(i=0;i<4096 && s[i]!=0x0d && s[i]!=0x0a && s[i]!=0;i++) {
			label[last_label].name[i]=s[i];
		}
		label[last_label].address = now_address + org_offset;
		last_label++;
	} else {
		if(n >= 0) {
			ebin_fetch dat;
			memset(&dat,0,sizeof(ebin_fetch));
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
			e_write16(dat.e_opcode,n);
			int size = get_instsize(s);
			if(mode) fwrite(&dat,1,size,out);
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
				if(mode) fwrite(&arg.d,1,1,out);
				now_address += 1;
			}
			if(strncmp(d,"dw",2) == 0) {
				argments_t arg = get_argments(s);
				if(mode) fwrite(&arg.d,1,2,out);
				now_address += 2;
			}
			if(strncmp(d,"dd",2) == 0) {
				argments_t arg = get_argments(s);
				if(mode) fwrite(&arg.d,1,4,out);
				now_address += 4;
			}
			if(strncmp(d,"text",4) == 0) {
				char *e = gpc_getargstrEx(s);
				if(mode) fwrite(e,1,strlen(e),out);
				now_address += strlen(e);
			}
		}
	}
}

int main(int argc, char *argv[]) {
	if(argc < 3) {
		puts("usage>easm <source path> <binary path>");
		return 1;
	}
	
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
	
	do {
		assenble_fetch(in,out,0);
	} while(!feof(in));
	
	now_address = 0;
	org_offset = 0;
	
	fseek(in,0,SEEK_SET);
	
	do {
		assenble_fetch(in,out,1);
	} while(!feof(in));
	
	fclose(in);
	fclose(out);
	
	return 0;
}
