/*
 *  unistd.h
 *
 *  Copyright by WATCOM International Corp. 1988-1996.  All rights reserved.
 */
#ifndef _UNISTD_H_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#ifndef _IO_H_INCLUDED
#include <io.h>
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

#if !defined(NO_EXT_KEYS) && defined(__NT__) && (defined(__UNICODE__) || defined(_UNICODE))
/*
 * Unicode functions
 */
extern int	_uaccess( const _unichar *, int );
extern int	_uchdir( const _unichar * );
extern _unichar *_ugetcwd( _unichar *, size_t );
extern int	_urmdir( const _unichar * );
extern int	_uunlink( const _unichar * );
#endif

#define _UNISTD_H_INCLUDED
#ifdef __cplusplus
};
#endif
#endif
