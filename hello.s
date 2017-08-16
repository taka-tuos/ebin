
	; glb putchar : (
	; prm     c : int
	;     ) void

	global(._putchar)
._putchar
	pr(%d34)
	push()
	pr(%d32)
	lr(%d34)
	pi($         4)
	pr(%d34)
	pr(%d34)
	add()
	pi($         0)
	pr(%d32)
	pr(%d32)
	sub()
	;pr %d00 
	;push  
	;pr %d01 
	;push  
	pr(%d02)
	push()
	;pr %d03 
	;push  
	;pr %d04 
	;push  
	;pr %d05 
	;push  
	;pr %d06 
	;push  
	;pr %d07 
	;push  
	;pr %d08 
	;push  
	;pr %d09 
	;push  
	;pr %d10 
	;push  
	;pr %d11 
	;push  
	;pr %d12 
	;push  
	;pr %d13 
	;push  
	;pr %d14 
	;push  
	;pr %d15 
	;push  
	;pr %d16 
	;push  
	;pr %d17 
	;push  
	;pr %d18 
	;push  
	;pr %d19 
	;push  
	;pr %d20 
	;push  
	;pr %d21 
	;push  
	;pr %d22 
	;push  
	;pr %d23 
	;push  
	;pr %d24 
	;push  
	;pr %d25 
	;push  
	;pr %d26 
	;push  
	;pr %d27 
	;push  
	;pr %d28 
	;push  
	;pr %d29 
	;push  
	;pr %d30 
	;push  
	;pr %d31 
	;push  
	;pr %d32 
	;push  
	;pr %d33 
	;push  
	pr(%d34)
	push()
	;pr %d35 
	;push  
	; loc     c : (@8): int
	pi($8)

	pr(%d34)

	pr(%d24)

	ldd()

	pr(%d24)

	tx()

.L1
	pr(%d34)
	pop()
	pr(%d02)
	pop()
	pi($         4)
	pr(%d34)
	pr(%d34)
	sub()
	pr(%d34)
	lr(%d32)
	pr(%d34)
	pop()
	r()

	; glb puts : (
	; prm     s : * char
	;     ) void

	global(._puts)
._puts
	pr(%d34)
	push()
	pr(%d32)
	lr(%d34)
	pi($         4)
	pr(%d34)
	pr(%d34)
	add()
	pi($         0)
	pr(%d32)
	pr(%d32)
	sub()
	;pr %d00 
	;push  
	;pr %d01 
	;push  
	pr(%d02)
	push()
	;pr %d03 
	;push  
	;pr %d04 
	;push  
	;pr %d05 
	;push  
	;pr %d06 
	;push  
	;pr %d07 
	;push  
	;pr %d08 
	;push  
	;pr %d09 
	;push  
	;pr %d10 
	;push  
	;pr %d11 
	;push  
	;pr %d12 
	;push  
	;pr %d13 
	;push  
	;pr %d14 
	;push  
	;pr %d15 
	;push  
	;pr %d16 
	;push  
	;pr %d17 
	;push  
	;pr %d18 
	;push  
	;pr %d19 
	;push  
	;pr %d20 
	;push  
	;pr %d21 
	;push  
	;pr %d22 
	;push  
	;pr %d23 
	;push  
	;pr %d24 
	;push  
	;pr %d25 
	;push  
	;pr %d26 
	;push  
	;pr %d27 
	;push  
	;pr %d28 
	;push  
	;pr %d29 
	;push  
	pr(%d30)
	push()
	;pr %d31 
	;push  
	;pr %d32 
	;push  
	;pr %d33 
	;push  
	pr(%d34)
	push()
	;pr %d35 
	;push  
 
	; loc     s : (@8): * char
	; for
.L5
	; RPN'ized expression: "s *u "
	; Expanded expression: "(@8) *(4) *(-1) "
	; local ofs
	; * (read dereference)
	pi($         8)
	pr(%d34)
	pr(%d0)
	ldd()
	; * (read dereference)
	pi($         0)
	pr(%d0)
	pr(%d0)
	ldb()
	pr(%d0)
	lr(%d30)
	pi($         7)
	pr(%d30)
	pr(%d30)
	lrs()
	pi($4294967040)
	pr(%d30)
	pr(%d30)
	mul()
	pr(%d30)
	pr(%d0)
	pr(%d0)
	or()
	; tokReturn
	; JumpIfZero
	pr(%d0)
	pr(%d0)
	cmp()
	pi(.L8)
	bz()
	; RPN'ized expression: "s ++p "
	; Expanded expression: "(@8) ++p(4) "
	; RPN'ized expression: "( s *u putchar ) "
	; Expanded expression: " (@8) *(4) *(-1)  putchar ()4 "
	; (
	; local ofs
	; * (read dereference)
	pi($         8)
	pr(%d34)
	pr(%d0)
	ldd()
	; * (read dereference)
	pi($         0)
	pr(%d0)
	pr(%d0)
	ldb()
	pr(%d0)
	lr(%d30)
	pi($         7)
	pr(%d30)
	pr(%d30)
	lrs()
	pi($4294967040)
	pr(%d30)
	pr(%d30)
	mul()
	pr(%d30)
	pr(%d0)
	pr(%d0)
	or()
	; ,
	; putchar
	pr(%d0)
	push()
	; ) fxn call
	pi(._putchar)
	c()
	pi($4294967292)
	pr(%d32)
	pr(%d32)
	sub()
.L6
	; local ofs
	; ++p
	pi($         8)
	pr(%d34)
	pr(%d0)
	ldd()
	pi($         1)
	pr(%d0)
	pr(%d0)
	add()
	pi($         8)
	pr(%d34)
	pr(%d0)
	stb()
	pi($4294967295)
	pr(%d0)
	pr(%d0)
	add()
	pi(.L5)
	b()
.L8
.L3
	pr(%d34)
	pop()
	pr(%d30)
	pop()
	pr(%d02)
	pop()
	pi($         4)
	pr(%d34)
	pr(%d34)
	sub()
	pr(%d34)
	lr(%d32)
	pr(%d34)
	pop()
	r()

	; glb main : (void) void

	global(._main)
._main
	pr(%d34)
	push()
	pr(%d32)
	lr(%d34)
	pi($         4)
	pr(%d34)
	pr(%d34)
	add()
	pi($         0)
	pr(%d32)
	pr(%d32)
	sub()
	;pr %d00 
	;push  
	;pr %d01 
	;push  
	pr(%d02)
	push()
	;pr %d03 
	;push  
	;pr %d04 
	;push  
	;pr %d05 
	;push  
	;pr %d06 
	;push  
	;pr %d07 
	;push  
	;pr %d08 
	;push  
	;pr %d09 
	;push  
	;pr %d10 
	;push  
	;pr %d11 
	;push  
	;pr %d12 
	;push  
	;pr %d13 
	;push  
	;pr %d14 
	;push  
	;pr %d15 
	;push  
	;pr %d16 
	;push  
	;pr %d17 
	;push  
	;pr %d18 
	;push  
	;pr %d19 
	;push  
	;pr %d20 
	;push  
	;pr %d21 
	;push  
	;pr %d22 
	;push  
	;pr %d23 
	;push  
	;pr %d24 
	;push  
	;pr %d25 
	;push  
	;pr %d26 
	;push  
	;pr %d27 
	;push  
	;pr %d28 
	;push  
	;pr %d29 
	;push  
	;pr %d30 
	;push  
	;pr %d31 
	;push  
	;pr %d32 
	;push  
	;pr %d33 
	;push  
	pr(%d34)
	push()
	;pr %d35 
	;push  
	; RPN'ized expression: "( 104 putchar ) "
	; Expanded expression: " 104  putchar ()4 "
	; (
	; 104
	pi($       104)
	lr(%d0)
	; ,
	; putchar
	pr(%d0)
	push()
	; ) fxn call
	pi(._putchar)
	c()
	pi($4294967292)
	pr(%d32)
	pr(%d32)
	sub()
	; RPN'ized expression: "( 101 putchar ) "
	; Expanded expression: " 101  putchar ()4 "
	; (
	; 101
	pi($       101)
	lr(%d0)
	; ,
	; putchar
	pr(%d0)
	push()
	; ) fxn call
	pi(._putchar)
	c()
	pi($4294967292)
	pr(%d32)
	pr(%d32)
	sub()
	; RPN'ized expression: "( 108 putchar ) "
	; Expanded expression: " 108  putchar ()4 "
	; (
	; 108
	pi($       108)
	lr(%d0)
	; ,
	; putchar
	pr(%d0)
	push()
	; ) fxn call
	pi(._putchar)
	c()
	pi($4294967292)
	pr(%d32)
	pr(%d32)
	sub()
	; RPN'ized expression: "( 108 putchar ) "
	; Expanded expression: " 108  putchar ()4 "
	; (
	; 108
	pi($       108)
	lr(%d0)
	; ,
	; putchar
	pr(%d0)
	push()
	; ) fxn call
	pi(._putchar)
	c()
	pi($4294967292)
	pr(%d32)
	pr(%d32)
	sub()
	; RPN'ized expression: "( 111 putchar ) "
	; Expanded expression: " 111  putchar ()4 "
	; (
	; 111
	pi($       111)
	lr(%d0)
	; ,
	; putchar
	pr(%d0)
	push()
	; ) fxn call
	pi(._putchar)
	c()
	pi($4294967292)
	pr(%d32)
	pr(%d32)
	sub()
	; RPN'ized expression: "( 44 putchar ) "
	; Expanded expression: " 44  putchar ()4 "
	; (
	; 44
	pi($        44)
	lr(%d0)
	; ,
	; putchar
	pr(%d0)
	push()
	; ) fxn call
	pi(._putchar)
	c()
	pi($4294967292)
	pr(%d32)
	pr(%d32)
	sub()
	; RPN'ized expression: "( 119 putchar ) "
	; Expanded expression: " 119  putchar ()4 "
	; (
	; 119
	pi($       119)
	lr(%d0)
	; ,
	; putchar
	pr(%d0)
	push()
	; ) fxn call
	pi(._putchar)
	c()
	pi($4294967292)
	pr(%d32)
	pr(%d32)
	sub()
	; RPN'ized expression: "( 111 putchar ) "
	; Expanded expression: " 111  putchar ()4 "
	; (
	; 111
	pi($       111)
	lr(%d0)
	; ,
	; putchar
	pr(%d0)
	push()
	; ) fxn call
	pi(._putchar)
	c()
	pi($4294967292)
	pr(%d32)
	pr(%d32)
	sub()
	; RPN'ized expression: "( 114 putchar ) "
	; Expanded expression: " 114  putchar ()4 "
	; (
	; 114
	pi($       114)
	lr(%d0)
	; ,
	; putchar
	pr(%d0)
	push()
	; ) fxn call
	pi(._putchar)
	c()
	pi($4294967292)
	pr(%d32)
	pr(%d32)
	sub()
	; RPN'ized expression: "( 108 putchar ) "
	; Expanded expression: " 108  putchar ()4 "
	; (
	; 108
	pi($       108)
	lr(%d0)
	; ,
	; putchar
	pr(%d0)
	push()
	; ) fxn call
	pi(._putchar)
	c()
	pi($4294967292)
	pr(%d32)
	pr(%d32)
	sub()
	; RPN'ized expression: "( 100 putchar ) "
	; Expanded expression: " 100  putchar ()4 "
	; (
	; 100
	pi($       100)
	lr(%d0)
	; ,
	; putchar
	pr(%d0)
	push()
	; ) fxn call
	pi(._putchar)
	c()
	pi($4294967292)
	pr(%d32)
	pr(%d32)
	sub()
	; 0
	pi($         0)
	lr(%d0)
	; tokReturn
.L9
	pr(%d34)
	pop()
	pr(%d02)
	pop()
	pi($         4)
	pr(%d34)
	pr(%d34)
	sub()
	pr(%d34)
	lr(%d32)
	pr(%d34)
	pop()
	r()



	; Syntax/declaration table/stack:
	; Bytes used: 145/15360


	; Macro table:
	; Macro __SMALLER_C__ = `0x0100`
	; Macro __SMALLER_C_32__ = ``
	; Macro __SMALLER_C_SCHAR__ = ``
	; Bytes used: 63/5120


	; Identifier table:
	; Ident __floatsisf
	; Ident __floatunsisf
	; Ident __fixsfsi
	; Ident __fixunssfsi
	; Ident __addsf3
	; Ident __subsf3
	; Ident __negsf2
	; Ident __mulsf3
	; Ident __divsf3
	; Ident __lesf2
	; Ident __gesf2
	; Ident putchar
	; Ident c
	; Ident puts
	; Ident s
	; Ident main
	; Ident <something>
	; Bytes used: 161/5632

	; Next label number: 11
	; Compilation succeeded.
