TARGET		= exec-ebin
OBJS_TARGET	= exec.o ebin.o

CFLAGS = -O0 -g -std=gnu99 -Wall
LIBS = 

include Makefile.in

sample : Makefile Makefile.in
	 as/as sample.eas sample.ebin
