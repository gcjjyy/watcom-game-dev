/*
 *  setjmp.h
 *
 *  Copyright by WATCOM International Corp. 1988-1996.  All rights reserved.
 */
#ifndef _SETJMP_H_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

/* macros for MS compatibility */
#ifndef _CRTAPI1
 #define _CRTAPI1
#endif
#ifndef _CRTAPI2
 #define _CRTAPI2
#endif
#ifndef _CRTIMP
 #define _CRTIMP
#endif

typedef unsigned int jmp_buf[13];

extern int  _setjmp( jmp_buf __env );
#define setjmp(__env)	  _setjmp(__env)
#if defined(__386__)
 #pragma aux _setjmp __parm __caller [eax] __modify [8087];
#else
 #pragma aux _setjmp __modify [8087];
#endif

extern void longjmp( jmp_buf __env, int __val );

#pragma aux	longjmp __aborts;

#define _SETJMP_H_INCLUDED
#ifdef __cplusplus
};
#endif
#endif
