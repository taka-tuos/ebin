TARGET		= exec-ebin
OBJS_TARGET	= exec.o ebin.o

CFLAGS = -O0 -g -std=gnu99 -Wall
LIBS = 

include Makefile.in

sample : Makefile Makefile.in
	 as/as crt0.s crt0.o -coff
	 as/as sample.s sample.o -coff
	 ld/ld sample.bin 0x1000 crt0.o sample.o
