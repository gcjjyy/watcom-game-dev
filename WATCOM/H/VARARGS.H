/*
 *  varargs.h	Variable argument macros (UNIX System V definition)
 *		definitions for use with variable argument lists
 *
 *  Copyright by WATCOM International Corp. 1988-1996.  All rights reserved.
 */
#ifndef _VARARGS_H_INCLUDED
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

#ifndef _STDARG_H_INCLUDED
 #include <stdarg.h>
#else
 #error stdarg.h has already been included
#endif

#define va_alist	void *__alist, ...
#define va_dcl

#undef va_start
#if defined(__HUGE__) || defined(__SW_ZU)
#define va_start(ap)	((ap)[0]=(char __far *)&__alist,(void)0)
#else
#define va_start(ap)	((ap)[0]=(char *)&__alist,(void)0)
#endif

#define _VARARGS_H_INCLUDED
#ifdef __cplusplus
};
#endif
#endif
