//==========================================================================
// This file is part of TMOS Operating System.
// Copyright (C) 2007-2008 Informa Ltd.
//
// TMOS is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3 or (at your option) any later version.
//
// TMOS is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
//==========================================================================
//
// Author(s):     Miroslav Kostadinov
// Date:          2008-12-02
// Description:   string.h functions
//
//========================================================================
#include "port_asm.h"

//*----------------------------------------------------------------------------
//*		int memcmp( const void *buf1, const void *buf2, size_t count )
//*----------------------------------------------------------------------------
//*The return value indicates the relationship between the buffers.
//*
//* Return Value Relationship of First count Bytes of buf1 and buf2
//*  < 0 buf1 less than buf2
//*  0   buf1 identical to buf2
//*  > 0 buf1 greater than buf2
//*
//*----------------------------------------------------------------------------
FUNC(	memcmp		)
				cmp				r2, #0
				beq				3f

				push			{r4}
				mov				r12, r2
				movs			r2, #0

1:
				ldrb			r3, [r0, r2]
				ldrb			r4,	[r1, r2]
				cmp				r3, r4
				bne				2f
				adds			r2, #1
				cmp				r2, r12
				bne				1b
2:
				subs			r0, r3, r4
				pop				{r4}
				bx				lr
3:
				movs			r0, #0
  				bx				lr

ENDF(	memcmp		)

//*----------------------------------------------------------------------------
//*		void *memset(const void *dst, int c, unsigned int length)
//*----------------------------------------------------------------------------
//*	return: dst
//*----------------------------------------------------------------------------
FUNC(	memset		)

				cmp				r2, #0
				beq				2f
1:				subs			r2, #1
				strb			r1, [r0, r2]
				bne				1b
2:
				bx				lr
ENDF(	memset		)

//*----------------------------------------------------------------------------
//*		void *memclr(const void *dst, size_t length)
//*----------------------------------------------------------------------------
//*	faster than memset(..., 0, ...)
//*	return: dst
//*----------------------------------------------------------------------------
FUNC(	memclr		)

1:
				lsls			r2, r0, #30
				bne				6f				//align r0(ptr)
2:
				lsls			r3, r1, #30
				bne				7f				//align r1(len)
3:
				lsrs			r1, #3
				bcs				8f
				beq				5f
4:
				stmia			r0!, {r2, r3}
				subs			r1, #1
				bne				4b
5:
				bx				lr

6://align r0(ptr)
				subs			r1, #1
				blo				5b
				strb			r2, [r0]
				adds			r0, #1
				b				1b
7:
				subs			r1, #1
				strb			r3, [r0, r1]
				b				2b
8:
				stmia			r0!, {r2}
				bne				4b
				bx				lr

ENDF(	memclr		)

//*----------------------------------------------------------------------------
//*		void *memcpy(void *dst, void *src, unsigned int length)
//*----------------------------------------------------------------------------
//*	return: dst
//*----------------------------------------------------------------------------
FUNC(	memcpy		)
				subs			r2, #1
				blo				2f
1:
				ldrb			r3, [r1, r2]
				strb			r3, [r0, r2]
				subs			r2, #1
				bhs				1b
2:
				bx				lr

ENDF(	memcpy		)

//*----------------------------------------------------------------------------
//*		void *memmove(void *dst, void *src, unsigned int length)
//*----------------------------------------------------------------------------
//*	return: dst
//*----------------------------------------------------------------------------
FUNC(	memmove		)
				cmp				r1, r0
				bls				memcpy
				rsbs			r2, #0
				beq				2f
				subs			r0, r2
				subs			r1, r2
1:
				ldrb			r3, [r1, r2]
				strb			r3, [r0, r2]
				adds			r2, #1
				bne				1b
2:
				bx				lr
ENDF(	memmove		)

//*----------------------------------------------------------------------------
//*		void* memchr(const void* src, unsigned int ch, unsigned int i)
//*----------------------------------------------------------------------------
FUNC(	memchr		)
1:
				cmp				r2, #0		// len == 0
				beq				2f
				ldrb			r3, [r0]
				cmp 			r3, r1
				beq				3f
				adds			r0, #1
				subs			r2, #1
				b				1b
2:
				movs			r0, #0
3:
				bx				lr

ENDF(	memchr		)


//*----------------------------------------------------------------------------
//*		void *strcpy(void *dst, void *src)
//*----------------------------------------------------------------------------
//*	return: dst
//*----------------------------------------------------------------------------
FUNC(	strcpy		)

				cmp				r0, #0
				beq				3f
				cmp				r1, #0
				beq				3f
				movs			r2, #0
1:
				ldrb			r3, [r1, r2]
  				strb			r3, [r0, r2]
				adds			r2, #1
				cmp				r3, #0
  				bne				1b
3:
  				bx				lr

ENDF(	strcpy		)

//*----------------------------------------------------------------------------
//*		char* strncpy(char* dst, const char* src, unsigned int len)
//*----------------------------------------------------------------------------
//*	return: If a NUL character is written to the destination, the function shall return
//* 		the address of the first such NUL character. Otherwise, it shall return &dst[len]
//*----------------------------------------------------------------------------
FUNC(	strncpy		)

				cmp				r0, #0
				beq				5f				// if (dst==NULL)
				cmp				r1, #0
				beq				5f				// if (src==NULL)

				rsbs			r2, #0
				beq				2f				// len  = 0
				subs			r0, r2
				subs			r1, r2
1:
				ldrb			r3, [r1, r2]
				strb			r3, [r0, r2]
				cmp				r3, #0
  				beq				3f
				adds			r2, #1
				bne				1b
2: // len=0
				strb			r2,	[r0]		// dst[len] = 0 (not a standard?)
				bx				lr

3: // NUL, fill
				adds			r0, r2
				adds			r2, #1
				beq				5f
				rsbs			r2, #0
4:
				strb			r3, [r0, r2]
				subs			r2, #1
				bne				3b
5:
				bx				lr

ENDF(	strncpy		)

//*----------------------------------------------------------------------------
//*		unsigned int strlen(const char* dst)
//*----------------------------------------------------------------------------
FUNC(	strlen		)

				cmp				r0, #0
				beq				2f
				adds			r2, r0, #1
1:
				ldrb			r3, [r0]
				adds			r0, #1
				cmp				r3, #0
  				bne				1b
  				subs			r0, r2
2:
				bx				lr

ENDF(	strlen		)

//*----------------------------------------------------------------------------
//*		char* strchr(const char* dst, unsigned int i)
//*----------------------------------------------------------------------------
FUNC(	strchr		)

				movs			r3, r0
				beq				9f
1:
				ldrb			r0, [r3]
				cmp				r0, #0
				beq				9f
				adds			r3, #1
				cmp				r0, r1
				bne				1b
				subs			r0, r3, #1
9:
				bx				lr

ENDF(	strchr		)

//*----------------------------------------------------------------------------
//*		char* strrchr(const char* dst, unsigned int i)
//*----------------------------------------------------------------------------
FUNC(	strrchr		)
				movs			r2, r0
				beq				2f
				movs			r0, #0
1:
				ldrb			r3, [r2]
				cmp				r3, #0
				beq				2f
				adds			r2, #1
				cmp				r3, r1
				bne				1b
				subs			r0, r2, #1
				b				1b
2:
				bx				lr

ENDF(	strrchr		)

//*----------------------------------------------------------------------------
//*		const char * strpbrk ( const char * str1, const char * str2 );
//*----------------------------------------------------------------------------
FUNC(	strpbrk		)

				cmp				r0, #0
				beq				4f				// if (!str1) return NULL
				cmp				r1, #0
				beq				3f				// if (!str2) return NULL
				mov				r12, r1
  				ldrb			r2, [r0]
				cmp				r2, #0
				beq				3f				// not found, return NULL
1:
				ldrb			r3, [r1]
				cmp				r3, #0
				beq				2f				//next str1
				adds			r1, #1
				cmp				r2, r3
				bne				1b
				bx				lr
2:
				adds			r0, #1
				mov				r1, r12
  				ldrb			r2, [r0]
				cmp				r2, #0
				bne				1b
3:
  				movs			r0, #0
4:
				bx				lr


ENDF(	strpbrk		)



#if USE_FIXED_CPAGE
//*----------------------------------------------------------------------------
//*		int islower(unsigned int c)
//*----------------------------------------------------------------------------
FUNC(	islower		)
.weak islower
				uxtb			r2, r0
				ldr				r1, =_char_2_type_
				ldrb			r0, [r2, r1]
				lsls			r0, #31
				bx				lr

ENDF(	islower		)

//*----------------------------------------------------------------------------
//*		int isupper(unsigned int c)
//*----------------------------------------------------------------------------
FUNC(	isupper		)
.weak isupper
				uxtb			r2, r0
				ldr				r1, =_char_2_type_
				ldrb			r0, [r2, r1]
				lsrs			r0, #7
				bx				lr

ENDF(	isupper		)
#endif //USE_FIXED_CPAGE

//*----------------------------------------------------------------------------
//*		int strcmp(const char* dst, const char* src)
//*----------------------------------------------------------------------------
//*	return: A value greater than zero indicates that the first character that
//* 		does not match has a greater value in str1 than in str2
//*----------------------------------------------------------------------------
FUNC(	strcmp		)

				movs			r2,	r0
				beq				4f
				movs			r3, r1
				beq				3f
1:
				ldrb			r2, [r0]
				ldrb			r3, [r1]
				cmp				r2, #0
				beq				2f
				adds			r0, #1
				adds			r1, #1
				cmp				r2, r3
				beq				1b
2:
				subs			r0, r2, r3
  				bx				lr

3:
                ldrb            r2, [r0]
4:
				movs			r3, r1
				beq				2b
				ldrb			r3, [r1]
				b				2b


ENDF(	strcmp		)

//*----------------------------------------------------------------------------
//*		int strncmp(const char* dst, const char* src, unsigned int len)
//*----------------------------------------------------------------------------
//*	return:
//*----------------------------------------------------------------------------
FUNC(	strncmp		)

				adds			r3, r0, r2			//r3 = dst+len
				rsbs			r0, r2, #0			//r0 = -len
				beq				3f
				adds			r1, r2				//r1 = src+len
				mov				r12, r4
1:
				ldrb			r4, [r0, r3]
				ldrb			r2, [r0, r1]
				cmp				r4, r2
				bne				2f
				cmp				r4, #0
				beq				2f
				adds			r0, #1
				bne				1b
2:
				subs			r0, r4, r2
				mov				r4, r12
3:
				bx				lr

ENDF(	strncmp		)

//*----------------------------------------------------------------------------
//*     const char * strstr ( const char * str1, const char * str2 );
//*     char * strstr (       char * str1, const char * str2 );
//*
//*     char* strstr(const char*, const char*);
//*----------------------------------------------------------------------------
//* return:
//*----------------------------------------------------------------------------
FUNC(  strstr      )

                push            {r4,r5}
1:
                mov             r3, r1
                ldrb            r5, [r3]	        //first = *str2++
                cmp             r5, #0
                beq				9f         		    //found
                adds			r3, #1
2:
                ldrb            r4, [r0]	        //r4 = *str1++
                cmp             r4, #0
                beq				7f 		            //fail
                adds			r0, #1
                cmp             r4, r5
                bne             2b
                //first char matches
                mov             r2, r0
3:
                ldrb            r5, [r3]		    //r5 = *str2++
                cmp             r5, #0
                beq				9f           		//found
                adds			r3, #1
                ldrb            r4, [r2]	        //r4 = *tmp++
                adds			r2, #1
                cmp             r4, r5
                beq	            3b
                cmp             r4, #0
                bne             1b
7:	// fail
                movs			r0, #0
9:
                pop             {r4, r5}
                bx              lr
ENDF(   strstr     )

#if USE_FIXED_CPAGE
//*----------------------------------------------------------------------------
//*		int *strcasecmp(void *dst, void *src)
//*----------------------------------------------------------------------------
//*	return:
//*----------------------------------------------------------------------------
#if USE_FIXED_CPAGE == 1250
FUNC_SECT(  strcasecmp, .text.casecmp   )
.weak strcasecmp
                push            {r4, r5}
                adr             r2, _char_2_type_
1:
                ldrb            r3, [r0], #1
                ldrb            r4, [r2, r3]
                lsrs            r4, #7              //isupper
                beq             3f
                lsrs            r4, r3, #6
                cmp             r4, #2
                bne             2f
                cmp             r3, #0xA5        //A
                it              eq
                moveq           r3, #0xB9 - 16   //a
                cmp             r3, #0xBC        //L
                it              eq
                moveq           r3, 0xBE -16     //l
                subs            r3, #16
2:
                adds            r3, #32
3:

                ldrb            r5, [r1], #1
                cbz             r3, 6f
                ldrb            r4, [r2, r5]
                lsrs            r4, #7              //isupper
                beq             5f
                lsrs            r4, r5, #6
                cmp             r4, #2
                bne             4f
                cmp             r5, #0xA5        //A
                it              eq
                moveq           r5, #0xB9 - 16   //a
                cmp             r5, #0xBC        //L
                it              eq
                moveq           r5, 0xBE -16     //l
                subs            r5, #16
4:
                adds            r5, #32
5:

                cmp             r3, r5
                beq             1b
6:
                subs            r0, r3, r5
                pop             {r4, r5}
                bx              lr

ENDF(   strcasecmp      )
#endif // USE_FIXED_CPAGE == 1250

#if USE_FIXED_CPAGE == 1251
FUNC_SECT(  strcasecmp, .text.casecmp   )
.weak strcasecmp
				push			{r4, r5}
				ldr				r2, =_char_2_type_
				subs			r0, r1
1:
				ldrb			r3, [r0, r1]
				ldrb			r4, [r2, r3]
				lsrs			r4, #7				//isupper
				lsls			r4, #5
				adds			r3, r4				//tolower
				ldrb			r5, [r1]
				adds			r1, #1
				ldrb			r4, [r2, r5]
				lsrs			r4, #7				//isupper
				lsls			r4, #5
				adds			r5, r4				//tolower
				cmp				r3, #0
				beq				2f
				cmp				r3, r5
				beq				1b
2:
				subs			r0, r3, r5
				pop				{r4, r5}
  				bx				lr

ENDF(	strcasecmp		)
#endif // USE_FIXED_CPAGE == 1251

//*----------------------------------------------------------------------------
//*		int *strncasecmp(void *dst, void *src, unsigned int len)
//*----------------------------------------------------------------------------
//*	return:
//*----------------------------------------------------------------------------
#if USE_FIXED_CPAGE == 1250
FUNC_SECT(  strncasecmp,    .text.casecmp   )
.weak strncasecmp

                cbz             r2, 9f
                adr             r3, _char_2_type_
                push            {r4, r5, r6}
1:
                ldrb            r6, [r0], #1
                ldrb            r4, [r3, r6]
                lsrs            r4, r4, #7          //isupper
                beq             3f
                lsrs            r4, r6, #6
                cmp             r4, #2
                bne             2f
                cmp             r6, #0xA5        //A
                it              eq
                moveq           r6, #0xB9 - 16   //a
                cmp             r6, #0xBC        //L
                it              eq
                moveq           r6, 0xBE -16     //l
                subs            r6, #16
2:
                adds            r6, #32
3:

                ldrb            r5, [r1], #1
                cbz             r6, 6f
                ldrb            r4, [r3, r5]
                lsrs            r4, r4, #7          //isupper
                beq             5f
                lsrs            r4, r5, #6
                cmp             r4, #2
                bne             4f
                cmp             r5, #0xA5        //A
                it              eq
                moveq           r5, #0xB9 - 16   //a
                cmp             r5, #0xBC        //L
                it              eq
                moveq           r5, 0xBE -16     //l
                subs            r5, #16
4:
                adds            r5, #32
5:

                subs            r2, #1
                beq             6f
                cmp             r6, r5
                beq             1b
6:
                subs            r0, r6, r5
                pop             {r4, r5, r6}
                bx              lr
9:
                movs            r0, #0
                bx              lr

ENDF(   strncasecmp     )
#endif // USE_FIXED_CPAGE == 1250

#if USE_FIXED_CPAGE == 1251
FUNC_SECT(  strncasecmp,    .text.casecmp   )
.weak strncasecmp

				rsbs			r2, #0
				beq				3f
				ldr				r3, =_char_2_type_
				push			{r4, r5, r6}
				subs			r0, r2
				subs			r1, r2
1:
				ldrb			r6, [r0, r2]
				ldrb			r4, [r3, r6]
				lsrs			r4, r4, #7			//isupper
				lsls			r4, #5
				add				r6, r4				//tolower
				ldrb			r5, [r1, r2]
				ldrb			r4, [r3, r5]
				lsrs			r4, r4, #7			//isupper
				lsls			r4, #5
				add				r5, r4				//tolower
				cmp				r6, #0
				beq				2f
				adds			r2, #1
				beq				2f
				cmp				r6, r5
				beq				1b
2:
				subs			r0, r6, r5
				pop				{r4, r5, r6}
  				bx				lr
3:
				movs			r0, #0
				bx				lr

ENDF(	strncasecmp		)
#endif // USE_FIXED_CPAGE == 1251

//*----------------------------------------------------------------------------
//*     int toupper(int c)
//*----------------------------------------------------------------------------
//* return:
//*----------------------------------------------------------------------------
#if USE_FIXED_CPAGE == 1250
FUNC_SECT(  toupper, .text.casecmp      )
.weak toupper
                ldr             r2, =_char_2_type_
                ldrb            r1, [r0, r2]
                lsls            r1, r1, #31             //islower
                beq             1f
                lsrs            r1, r0, #6
                cmp             r1, #2
                bne             4f
                cmp             r0, #0xB9    //a
                beq             2f
                cmp             r0, #0xBE    //l
                beq             3f
                subs            r0, #16
1:
                bx              lr

2:
                movs            r0, #0xA5    //A
                bx              lr

3:
                movs            r0, #0xBC    //L
                bx              lr

4:
                subs            r0, #32
                bx              lr

ENDF(   toupper     )
#endif // USE_FIXED_CPAGE == 1250

#if USE_FIXED_CPAGE == 1251
FUNC_SECT(  toupper, .text.casecmp      )
.weak toupper

                ldr             r2, =_char_2_type_
                ldrb            r1, [r0, r2]
                lsls            r1, r1, #31             //islower
                lsrs			r1, #31-5
                subs            r0, r1				   //tolower
                bx              lr

ENDF(   toupper     )
#endif // USE_FIXED_CPAGE == 1251

//*----------------------------------------------------------------------------
//*		int tolower(int c)
//*----------------------------------------------------------------------------
//*	return:
//*----------------------------------------------------------------------------
#if USE_FIXED_CPAGE == 1250
FUNC_SECT(  tolower, .text.casecmp      )
.weak tolower

                ldr             r2, =_char_2_type_
                ldrb            r1, [r0, r2]
                lsrs            r1, r1, #7              //isupper
                beq             1f
                lsrs            r1, r0, #6
                cmp             r1, #2
                bne             4f
                cmp             r0, #0xA5    //A
                beq             2f
                cmp             r0, #0xBC    //L
                beq             3f
                adds            r0, #16
1:
                bx              lr

2:
                movs            r0, #0xB9    //a
                bx              lr

3:
                movs            r0, #0xBE    //l
                bx              lr

4:
                adds            r0, #32
                bx              lr

ENDF(   tolower     )
#endif // USE_FIXED_CPAGE == 1250

#if USE_FIXED_CPAGE == 1251
FUNC_SECT(  tolower, .text.casecmp      )
.weak tolower

				ldr				r2, =_char_2_type_
				ldrb			r1, [r0, r2]
				lsrs			r1, #7					//isupper
				lsls			r1, #5
				add				r0, r1					//toupper
				bx				lr

ENDF(	tolower		)
#endif // USE_FIXED_CPAGE == 1251

//upper
#define	_U			0x80
//lower
#define	_L			0x01
//digit
#define	_N			0x44
//space
#define	_S			0x08
//punct
#define _P			0x10
//control
#define _C			0x20
//hex
#define _X			0x40
#define	_B			0x02

				.global G_char_2_type_
.weak G_char_2_type_
#if USE_FIXED_CPAGE == 1250
G_char_2_type_:
_char_2_type_:
    .byte   _C, _C, _C, _C, _C, _C, _C, _C                                  //00
    .byte   _C, _C|_S,  _C|_S,  _C|_S,  _C|_S,  _C|_S,  _C, _C              //08
    .byte   _C, _C, _C, _C, _C, _C, _C, _C                                  //10
    .byte   _C, _C, _C, _C, _C, _C, _C, _C                                  //18
    .byte   _S|_B,  _P, _P, _P, _P, _P, _P, _P                              //20  !"#$%&'
    .byte   _P, _P, _P, _P, _P, _P, _P, _P                                  //28 ()*+,-./
    .byte   _N, _N, _N, _N, _N, _N, _N, _N                                  //30 01234567
    .byte   _N, _N, _P, _P, _P, _P, _P, _P                                  //38 89:;<=>?
    .byte   _P, _U|_X,  _U|_X,  _U|_X,  _U|_X,  _U|_X,  _U|_X,  _U          //40 @ABCDEFG
    .byte   _U, _U, _U, _U, _U, _U, _U, _U                                  //48 HIJKLMNO
    .byte   _U, _U, _U, _U, _U, _U, _U, _U                                  //50 PQRSTUVW
    .byte   _U, _U, _U, _P, _P, _P, _P, _P                                  //58 XYZ[\]^_
    .byte   _P, _L|_X,  _L|_X,  _L|_X,  _L|_X,  _L|_X,  _L|_X,  _L          //60 'abcdefg
    .byte   _L, _L, _L, _L, _L, _L, _L, _L                                  //68 hijklmno
    .byte   _L, _L, _L, _L, _L, _L, _L, _L                                  //70 pqrstuvw
    .byte   _L, _L, _L, _P, _P, _P, _P, _C                                  //78 xyz{|}~
    .byte    0,  0, _P,  0, _P,  _P,  0,  0                                 //80 €�‚�„…†‡ 128
    .byte    0, _P, _U, _P, _U,  _U, _U, _U                                 //88 �‰Š‹ŚŤŽŹ 136
    .byte    0, _P, _P, _P, _P, _P, _P, _P                                  //90 �‘’“”•–— 144
    .byte    0,  0, _L, _P, _L, _L, _L, _L                                  //98 *™š›śťžź 152
    .byte   _C, _P, _P, _U,  0, _U, _P, _P                                  //A0  ˇ˘Ł¤Ą¦§ 160
    .byte   _P,  0, _U, _P,  0,  0,  0, _U                                  //A8 ¨©Ş«¬ ­®Ż 168
    .byte   _P, _P,  0, _L, _P,  0,  0,  0                                  //B0 °±˛ł´µ¶· 176
    .byte    0, _L, _L, _P, _U, _P, _L, _L                                  //B8 ¸ąş»Ľ˝ľż 184
    .byte   _U, _U, _U, _U, _U, _U, _U, _U                                  //C0 ŔÁÂĂÄĹĆÇ 192
    .byte   _U, _U, _U, _U, _U, _U, _U, _U                                  //C8 ČŇĘËĚÍÎĎ 200
    .byte   _U, _U, _U, _U, _U, _U, _U,  0                                  //D0 ĐŃŇÓÔŐÖ× 208
    .byte   _U, _U, _U, _U, _U, _U, _U,  0                                  //D8 ŘŮÚŰÜÝŢß 216
    .byte   _L, _L, _L, _L, _L, _L, _L, _L                                  //E0 ŕáâăäĺćç 224
    .byte   _L, _L, _L, _L, _L, _L, _L, _L                                  //E8 čéęëěíîď 232
    .byte   _L, _L, _L, _L, _L, _L, _L, _P                                  //F0 đńňóôőö÷ 240
    .byte   _L, _L, _L, _L, _L, _L, _L,  0                                  //F8 řůúűüý6˙ 248
#else
#if USE_FIXED_CPAGE == 1251
.align 2
G_char_2_type_:
_char_2_type_:
	.byte	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C									//00
	.byte	_C,	_C|_S,	_C|_S,	_C|_S,	_C|_S,	_C|_S,	_C,	_C				//08
	.byte	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C									//10
	.byte	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C									//18
	.byte	_S|_B,	_P,	_P,	_P,	_P,	_P,	_P,	_P								//20  !"#$%&'
	.byte	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P									//28 ()*+,-./
	.byte	_N,	_N,	_N,	_N,	_N,	_N,	_N,	_N									//30 01234567
	.byte	_N,	_N,	_P,	_P,	_P,	_P,	_P,	_P									//38 89:;<=>?
	.byte	_P,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U			//40 @ABCDEFG
	.byte	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U									//48 HIJKLMNO
	.byte	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U									//50 PQRSTUVW
	.byte	_U,	_U,	_U,	_P,	_P,	_P,	_P,	_P									//58 XYZ[\]^_
	.byte	_P,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L			//60 'abcdefg
	.byte	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L									//68 hijklmno
	.byte	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L									//70 pqrstuvw
	.byte	_L,	_L,	_L,	_P,	_P,	_P,	_P,	_C									//78 xyz{|}~
	.byte	0,	0,	0,	0,	0,	0,	0,	0									//80 ЂЃ‚ѓ„…†‡ 128
	.byte	0,	0,	0,	0,	0,	0,	0,	0									//88 €‰Љ‹ЊЌЋЏ 136
	.byte	0,	0,	0,	0,	0,	0,	0,	0									//90 ђ‘’“”•–— 144
	.byte	0,	0,	0,	0,	0,	0,	0,	0									//98 *™љ›њќћџ 152
	.byte	0,	0,	0,	0,	0,	0,	0,	0									//A0  ЎўЈ¤Ґ¦§ 160
	.byte	0,	0,	0,	0,	0,	0,	0,	0									//A8 Ё©Є«¬­®Ї 168
	.byte	0,	0,	0,	0,	0,	0,	0,	0									//B0 °±Ііґµ¶· 176
	.byte	0,	0,	0,	0,	0,	0,	0,	0									//B8 ё№є»јЅѕї 184
	.byte	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U									//C0 АБВГДЕЖЗ 192
	.byte	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U									//C8 ИТКЛМНОП 200
	.byte	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U									//D0 РСТУФХЦЧ 208
	.byte	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U									//D8 ШЩЪЫЬЭЮЯ 216
	.byte	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L									//E0 абвгдежз 224
	.byte	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L									//E8 ийклмноп 232
	.byte	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L									//F0 рстуфхцч 240
	.byte	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L									//F8 шщъыьэ6я 248

#endif // USE_FIXED_CPAGE == 1251
#endif // USE_FIXED_CPAGE == 1250

#endif //USE_FIXED_CPAGE

//*----------------------------------------------------------------------------
//*		unsigned int XNUM_TO_DIGIT(unsigned int c)
//*----------------------------------------------------------------------------
//*	return:
//*----------------------------------------------------------------------------
FUNC(	XNUM_TO_DIGIT		)

				cmp				r0, #9
				bhi				1f
				adds			r0, #'0'
				bx				lr
1:
				adds			r0, #'A'-10
				bx				lr


ENDF(	XNUM_TO_DIGIT		)


.end
