/*
 *  fcntl.h	File control options used by open
 *
 *  Copyright by WATCOM International Corp. 1988-1996.  All rights reserved.
 */
#ifndef _FCNTL_H_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif
#if __WATCOMC__ > 1000
#pragma pack(__push,1);
#else
#pragma pack(1);
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

#define O_RDONLY	0x0000	/* open for read only */
#define _O_RDONLY	0x0000	/* open for read only */
#define O_WRONLY	0x0001	/* open for write only */
#define _O_WRONLY	0x0001	/* open for write only */
#define O_RDWR		0x0002	/* open for read and write */
#define _O_RDWR		0x0002	/* open for read and write */
#define O_APPEND	0x0010	/* writes done at end of file */
#define _O_APPEND	0x0010	/* writes done at end of file */
#define O_CREAT 	0x0020	/* create new file */
#define _O_CREAT 	0x0020	/* create new file */
#define O_TRUNC 	0x0040	/* truncate existing file */
#define _O_TRUNC 	0x0040	/* truncate existing file */
#define O_NOINHERIT	0x0080	/* file is not inherited by child process */
#define _O_NOINHERIT	0x0080	/* file is not inherited by child process */
#define O_TEXT		0x0100	/* text file */
#define _O_TEXT		0x0100	/* text file */
#define O_BINARY	0x0200	/* binary file */
#define _O_BINARY	0x0200	/* binary file */
#define O_EXCL		0x0400	/* exclusive open */
#define _O_EXCL		0x0400	/* exclusive open */

/*
 *  POSIX 1003.1 Prototypes.
 */
extern int open( const char *__path, int __oflag, ... );
extern int sopen( const char *__path, int __oflag, int __share, ... );

/*
 *  UNICODE functions
 */
#if !defined(NO_EXT_KEYS) && defined(__NT__) && (defined(__UNICODE__) || defined(_UNICODE))
extern int _ucreat( const _unichar *, int );
extern int _uopen( const _unichar *, int, ... );
extern int _usopen( const _unichar *, int, int, ... );
#endif

#if __WATCOMC__ > 1000
#pragma pack(__pop);
#else
#pragma pack();
#endif

#define _FCNTL_H_INCLUDED
#ifdef __cplusplus
};
#endif
#endif
