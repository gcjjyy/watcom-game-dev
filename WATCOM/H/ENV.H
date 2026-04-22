/*
 *  env.h	Environment string operations
 *
 *  Copyright by WATCOM International Corp. 1988-1996.  All rights reserved.
 */
#ifndef _ENV_H_INCLUDED
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

#if defined(__NT__) && (defined(__UNICODE__) || defined(_UNICODE))
#ifndef _UNICHAR_DEFINED_
  typedef wchar_t _UNICHAR;
#define _UNICHAR_DEFINED_
#endif
#if !defined(NO_EXT_KEYS) /* extensions enabled */
 #define _unichar	_UNICHAR
#endif
#endif

/*
 *  POSIX 1003.1 Prototypes.
 */
extern int clearenv( void );
extern char *getenv( const char *__name );
extern int setenv( const char *__name, const char *__newvalue, int __overwrite );
#if !defined(NO_EXT_KEYS) /* extensions enabled */
extern int putenv( const char *__env_string );
#endif

/*
 * UNICODE functions
 */
#if !defined(NO_EXT_KEYS) && defined(__NT__) && (defined(__UNICODE__) || defined(_UNICODE))
#endif

#define _ENV_H_INCLUDED
#ifdef __cplusplus
};
#endif
#endif
