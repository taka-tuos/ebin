#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define OPTION(s) strcmp(argv[i],s) == 0

int main(int argc, char *argv[], char *envp[])
{
	char cc1[4096];
	char cpp[4096];
	char as[4096];
	
	char binname[2048];
	char logname[2048] = "";
	
	char sz[512];
	char option_format[64] = "-flat";
	int i;
	int next = 0;
	int ret;
	int ptr = 0;
	int enlog = 0;
	
	
	char *dumppath = strdup(argv[0]);
	char *path = dirname(dumppath);
	
	sprintf(as, "%s/../as/as ",path);
	sprintf(cc1,"%s/../cc1/cc1 ",path);
	sprintf(cpp, "%s/../cpp/cpp ",path);
	
	if(argc < 3) {
		puts("Error : too few argments");
		printf("usage : %s [option] source output\n",argv[0]);
		return 3;
	}
	
	for(i = 1; i < argc; i++) {
		if(next == 1) {
			strcat(cpp,argv[i]);
			strcat(cpp," ");
			next = 0;
			continue;
		}
		if(next == 2) {
			strcpy(logname,argv[i]);
			next = 0;
			continue;
		}
		if(
			OPTION("-I") ||
			OPTION("-J")
		) {
			strcat(cpp,argv[i]);
			strcat(cpp," ");
			next = 1;
		} else if(
			OPTION("-C") ||
			OPTION("-s") ||
			OPTION("-l") ||
			OPTION("-CC") ||
			OPTION("-a") ||
			OPTION("-na") ||
			OPTION("-V") ||
			OPTION("-u") ||
			OPTION("-X") ||
			OPTION("-c90") ||
			OPTION("-w") ||
			OPTION("-zl") ||
			OPTION("-M") ||
			OPTION("-D") ||
			OPTION("-U") ||
			OPTION("-A") ||
			OPTION("-B") ||
			OPTION("-Y") ||
			OPTION("-Z") ||
			OPTION("-d") ||
			OPTION("-e") ||
			OPTION("-v") ||
			OPTION("-h")
		) {
			strcat(cpp,argv[i]);
			strcat(cpp," ");
		} else if(
			OPTION("-flat") ||
			OPTION("-coff")
		) {
			strcpy(option_format,argv[i]);
		} else if(
			OPTION("-log")
		) {
			next = 2;
			enlog = 1;
		} else if(
			OPTION("-help")
		) {
			puts("micro portable cc `UCC` by kagura1050");
			puts("Version : git");
			printf("usage : %s [option] source output\n",argv[0]);
			return 1;
		} else {
			if(ptr < 2) {
				if(ptr) {
					strcpy(binname,argv[i]);
				}
				if(!ptr) {
					strcat(cpp,argv[i]);
					strcat(cpp," ");
				}
			} else {
				puts("Error : too many filename");
				return 1;
			}
			ptr++;
		}
	}
	
	if(ptr < 2) {
		puts("Error : unable to get output filename");
		return 2;
	}
	
	if(logname[0] == 0 && enlog) {
		puts("Error : log file is not specified");
		return 4;
	}
	
	int tmp = time(NULL);
	
	char *allname = strdup(binname);
	char *purename = basename(allname);
	
	sprintf(sz,"_ucc_%08x_%s_asm.tmp %s %s",tmp,purename,binname,option_format);
	strcat(as,sz);
	
	sprintf(sz,"-o _ucc_%08x_%s_cxx.tmp",tmp,purename);
	strcat(cpp,sz);
	
	sprintf(sz,"_ucc_%08x_%s_cxx.tmp _ucc_%08x_%s_asm.tmp",tmp,purename,tmp,purename);
	strcat(cc1,sz);
	
	int child_ret = -2017;
	FILE *fp;
	
	FILE *logfp;
	
	if(enlog) {
		logfp = fopen(logname,"wt");
	}
	
	char *stdout_d[4096];
	int stdout_n = 0;
	
	char stdout_b[1024];
	
	puts(cpp);
	fp = popen(cpp,"r");
	while(fp && fgets(stdout_b,1024,fp)) {
		stdout_d[stdout_n] = strdup(stdout_b);
		if(enlog) fputs(stdout_d[stdout_n],logfp);
		stdout_n++;
	}
	if(!fp || (child_ret = pclose(fp)) != 0) {
		if(child_ret != -2017) printf("Preprocessor Returnd %d\n",child_ret);
		else printf("Preprocessor execute failed\n");
		printf("Compilation failed.\n");
		printf("stdout :\n");
		for(i = 0; i < stdout_n; i++) printf(stdout_d[i]);
		if(enlog) fclose(logfp);
		return -1;
	}
	
	stdout_n = 0;
	child_ret = -2017;
	
	puts(cc1);
	fp = popen(cc1,"r");
	while(fp && fgets(stdout_b,1024,fp)) {
		stdout_d[stdout_n] = strdup(stdout_b);
		if(enlog) fputs(stdout_d[stdout_n],logfp);
		stdout_n++;
	}
	if(!fp || (child_ret = pclose(fp)) != 0) {
		if(child_ret != -2017) printf("Compiler Returnd %d\n",child_ret);
		else printf("Compiler execute failed\n");
		printf("Compilation failed.\n");
		printf("stdout :\n");
		for(i = 0; i < stdout_n; i++) printf(stdout_d[i]);
		if(enlog) fclose(logfp);
		return -1;
	}
	
	stdout_n = 0;
	child_ret = -2017;
	
	puts(as);
	fp = popen(as,"r");
	while(fp && fgets(stdout_b,1024,fp)) {
		stdout_d[stdout_n] = strdup(stdout_b);
		if(enlog) fputs(stdout_d[stdout_n],logfp);
		stdout_n++;
	}
	if(!fp || (child_ret = pclose(fp)) != 0) {
		if(child_ret != -2017) printf("Assembler Returnd %d\n",child_ret);
		else printf("Assembler execute failed\n");
		printf("Compilation failed.\n");
		printf("stdout :\n");
		for(i = 0; i < stdout_n; i++) printf(stdout_d[i]);
		if(enlog) fclose(logfp);
		return -1;
	}
	
	if(enlog) fclose(logfp);
	
	return 0;
}
