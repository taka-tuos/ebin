TARGET		= exec-ebin
OBJS_TARGET	= exec.o ebin.o

CFLAGS := -O0 -g -std=gnu99 -Wall 
LIBS := 

include Makefile.in

debug : Makefile Makefile.in
	$(MAKE) CFLAGS+=-D__DEBUG__ $(TARGET)

sample : Makefile Makefile.in
	 as/as crt0.s crt0.o -coff
	 as/as sample.s sample.o -coff
	 ld/ld sample.bin 0x1000 crt0.o sample.o

sampleC : Makefile Makefile.in
	 as/as crt0.s crt0.o -coff
	 ucc/ucc -coff hello.c hello.o
	 ld/ld hello.bin 0x1000 crt0.o hello.o
	 
sampleXP : Makefile Makefile.in
	 as/as crt0.s crt0.o -coff
	 ucc/ucc -I golibc -coff pass0.c pass0.o
	 ucc/ucc -I golibc -coff xprintf.c xprintf.o
	 ld/ld xprintf.bin 0x1000 crt0.o pass0.o xprintf.o
