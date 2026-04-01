/*
 *  stddef.h	Standard definitions
 *
 *  Copyright by WATCOM International Corp. 1988-1996.  All rights reserved.
 */
#ifndef _STDDEF_H_INCLUDED
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

#ifndef _SIZE_T_DEFINED_
#define _SIZE_T_DEFINED_
 typedef unsigned size_t;
#endif

#ifndef _WCHAR_T_DEFINED
#define _WCHAR_T_DEFINED
#ifndef _WCHAR_T_DEFINED_
#define _WCHAR_T_DEFINED_
#ifdef __cplusplus
typedef long char wchar_t;
#else
typedef unsigned short wchar_t;
#endif
#else
#ifndef _WCHAR_T_DEFINED_
#define _WCHAR_T_DEFINED_
#endif
#endif
#endif

#ifndef NULL
 #if defined(__SMALL__) || defined(__MEDIUM__) || defined(__386__)
  #define NULL   0
 #else
  #define NULL   0L
 #endif
#endif

#ifndef _PTRDIFF_T_DEFINED_
#define _PTRDIFF_T_DEFINED_
 #if defined(__HUGE__)
  typedef long	ptrdiff_t;
 #else
  typedef int	ptrdiff_t;
 #endif
#endif

#ifdef __cplusplus
#define offsetof(__typ,__id) __offsetof(__typ,__id)
#else
#define offsetof(__typ,__id) ((size_t)&(((__typ*)0)->__id))
#endif
#if !defined(NO_EXT_KEYS) /* extensions enabled */
#ifdef __386__
 extern int *__threadid(void);	/* pointer to thread id */
 #define _threadid (__threadid())
#else
 extern int __far *_threadid;			/* pointer to thread id */
#endif
#endif
#define _STDDEF_H_INCLUDED
#ifdef __cplusplus
};
#endif
#endif
