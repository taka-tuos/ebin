#include "ebin.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
	if(argc < 2) {
		puts("usage>exec-ebin <binary>");
		return 1;
	}
	
	ebin_ctl ctl;
	ebin_init(&ctl, malloc(1024*1024));
	
	FILE *fp = fopen(argv[1],"rb");
	
	fread(ctl.e_memory+0x1000,1,65536,fp);
	
	fclose(fp);
	
	while(1) ebin_exec(&ctl);
	
	return 0;
}
