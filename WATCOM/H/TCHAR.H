/*
 *  tchar.h	Generic International Text Function Macros
 *
 *  Copyright by WATCOM International Corp. 1988-1996.  All rights reserved.
 */
#ifndef _TCHAR_H_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
 #if defined(__SMALL__) || defined(__MEDIUM__) || defined(__386__)
  #define NULL   0
 #else
  #define NULL   0L
 #endif
#endif

#include <string.h>
#include <time.h>

#if defined(__UNICODE__) || defined(_UNICODE)
// **************************** UNICODE **************************************
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

#ifndef _WCTYPE_T_DEFINED
typedef wchar_t wint_t;
typedef wchar_t wctype_t;
#define _WCTYPE_T_DEFINED
#endif

#ifndef __TCHAR_DEFINED
typedef wchar_t _TCHAR;
typedef wchar_t _TSCHAR;
typedef wchar_t _TUCHAR;
typedef wchar_t _TXCHAR;
typedef wchar_t _TINT;
#define __TCHAR_DEFINED
#endif

#ifndef _TCHAR_DEFINED
#if !defined(NO_EXT_KEYS) /* extensions enabled */
typedef wchar_t TCHAR;
#endif
#define _TCHAR_DEFINED
#endif

#define _TEOF		EOF

#define __T(x)		L ## x

/* Program */

#define _tmain		main
#define _tWinMain	WinMain
#define _tenviron	environ

/* Formatted i/o */

#define _tprintf	_uprintf
#define _ftprintf	_ufprintf
#define _stprintf	_usprintf
#define _vtprintf	_uvprintf
#define _vftprintf	_uvfprintf
#define _vstprintf	_uvsprintf
#define _tscanf		_uscanf
#define _ftscanf	_ufscanf
#define _stscanf	_usscanf


/* Unformatted i/o */

#define _fgettc		_ufgetc
#define _fgettchar	_ufgetchar
#define _fgetts		_ufgets
#define _fputtc		_ufputc
#define _fputtchar	_ufputchar
#define _fputts		_ufputs
#define _gettc		_ugetc
#define _gettchar	_ugetchar
#define _getts		_ugets
#define _puttc		_uputc
#define _puttchar	_uputchar
#define _putts		_uputs
#define _ungettc	_uungetc


/* String conversion functions */

#define _tcstod		_ustrtod
#define _tcstol		_ustrtol
#define _tcstoul	_ustrtoul

#define _itot		_uitoa
#define _ltot		_ultoa
#define _ultot		_uultoa
#define _ttoi		_uatoi
#define _ttol		_uatol


/* String functions */

#define _tcscat		_ustrcat
#define _tcschr		_ustrchr
#define _tcscpy		_ustrcpy
#define _tcscspn	_ustrcspn
#define _tcslen		_ustrlen
#define _tcsncat	_ustrncat
#define _tcsncpy	_ustrncpy
#define _tcspbrk	_ustrpbrk
#define _tcsrchr	_ustrrchr
#define _tcsspn		_ustrspn
#define _tcsstr		_ustrstr
#define _tcstok		_ustrtok

#define _tcsdup		_ustrdup
#define _tcsnset	_ustrnset
#define _tcsrev		_ustrrev
#define _tcsset		_ustrset

#define _tcscmp		_ustrcmp
#define _tcsicmp	_ustricmp
#define _tcsnccmp	_ustrncmp
#define _tcsncmp	_ustrncmp
#define _tcsncicmp	_ustrnicmp
#define _tcsnicmp	_ustrnicmp

#define _tcscoll	_ustrcoll


/* Execute functions */

#define _texecl		execl
#define _texecle	execle
#define _texeclp	execlp
#define _texeclpe	execlpe
#define _texecv		execv
#define _texecve	execve
#define _texecvp	execvp
#define _texecvpe	execvpe

#define _tspawnl	spawnl
#define _tspawnle	spawnle
#define _tspawnlp	spawnlp
#define _tspawnlpe	spawnlpe
#define _tspawnv	spawnv
#define _tspawnve	spawnve
#define _tspawnvp	spawnvp
#define _tspawnvpe	spawnvpe

#define _tsystem	system


/* Time functions */

#define _tasctime	_uasctime
#define _tctime		_uctime
#define _tstrdate	_ustrdate
#define _tstrtime	_ustrtime
#define _tutime		_uutime
#define _tcsftime	_ustrftime_ms


/* Directory functions */

#define _tchdir		_uchdir
#define _tgetcwd	_ugetcwd
#define _tgetdcwd	_ugetdcwd
#define _tmkdir		_umkdir
#define _trmdir		_urmdir


/* Environment/Path functions */

#define _tfullpath	_u_fullpath
#define _tgetenv	getenv
#define _tmakepath	_u_makepath
#define _tputenv	putenv
#define _tsearchenv	searchenv
#define _tsplitpath	_u_splitpath


/* Stdio functions */

#define _tfdopen	_ufdopen
#define _tfopen		_ufopen
#define _tfreopen	_ufreopen
#define _tperror	_uperror
#define _ttmpnam	_utmpnam


/* Io functions */

#define _taccess	_uaccess
#define _tchmod		_uchmod
#define _tcreat		_ucreat
#define _tfindfirst	_dos_findfirst
#define _tfindnext	_dos_findnext
#define _topen		_uopen
#define _tremove	_uremove
#define _trename	_urename
#define _tsopen		_usopen
#define _tunlink	_uunlink

#define _tfinddata_t	find_t


/* Stat functions */

#define _tstat		_ustat


/* Setlocale functions */

#define _tsetlocale	setlocale


/* Redundant "logical-character" mappings */

#define _tcsclen	_ustrlen
#define _tcsnccat	_ustrncat
#define _tcsnccpy	_ustrncpy
#define _tcsncset	_ustrnset

#define	_tcsdec		_ustrdec
#define	_tcsinc		_ustrinc
#define	_tcsnbcnt	_ustrncnt
#define	_tcsnccnt	_ustrncnt
#define	_tcsnextc	_ustrnextc
#define	_tcsninc	_ustrninc
#define	_tcsspnp	_ustrspnp

#define _tcslwr		_ustrlwr
#define _tcsupr		_ustrupr
#define _tcsxfrm	_ustrxfrm

#if defined(__cplusplus) && defined(__INLINE_FUNCTIONS__)
inline size_t _tclen( const wchar_t * ) {
    return( 1 );
}
inline void _tccpy( wchar_t *__p1, const wchar_t *__p2 ) {
    *__p1 = (wchar_t)*__p2;
}
inline int _tccmp( const wchar_t *__p1, const wchar_t *__p2 ) {
    return( (int) ((*__p1)-(*__p2)) );
}
#else
#define _tclen(__p)	(1)
#define _tccpy(__p1,__p2) ((*(__p1) = *(__p2)))
#define _tccmp(__p1,__p2) ((*(__p1))-(*(__p2)))
#endif


/* ctype functions */

#define _istalnum	_uisalnum
#define _istalpha	_uisalpha
#define _istascii	_uisascii
#define _istcntrl	_uiscntrl
#define _istdigit	_uisdigit
#define _istgraph	_uisgraph
#define _istlower	_uislower
#define _istprint	_uisprint
#define _istpunct	_uispunct
#define _istspace	_uisspace
#define _istupper	_uisupper
#define _istxdigit	_uisxdigit

#define _totupper	_utoupper
#define _totlower	_utolower

#define _istlegal(_c)   (1)
#define _istlead(_c)    (0)
#define _istleadbyte(_c)    (0)

#if defined(__cplusplus) && defined(__INLINE_FUNCTIONS__)
inline wchar_t *_ustrdec( const wchar_t *, const wchar_t *__p ) {
    return( (wchar_t *)(__p-1) );
}
inline wchar_t *_ustrinc( const wchar_t *__p ) {
    return( (wchar_t *)(__p+1) );
}
inline unsigned int _ustrnextc( const wchar_t *__p ) {
    return( (unsigned int)*__p );
}
inline wchar_t *_ustrninc( const wchar_t *__p, size_t __l ) {
    return( (wchar_t *)(__p+__l) );
}
inline size_t _ustrncnt( const wchar_t *__p, size_t __l ) {
    size_t __i;
    __i = _ustrlen(__p);
    return( (__i>__l) ? __l : __i );
}
inline wchar_t *_ustrspnp( const wchar_t *__p1, const wchar_t *__p2 ) {
    return( (*(__p1 += _ustrspn(__p1,__p2))!='\0') ? (wchar_t*)__p1 : NULL );
}
#else
#define _ustrdec(__p1, __p2)  ((__p2)-1)
#define _ustrinc(__p)	      ((__p)+1)
#define _ustrnextc(__p)	      ((unsigned int) *(__p))
#define _ustrninc(__p, __l)   (((__p)+(__l)))
#define _ustrncnt(__p, __l)   ((_ustrlen(__p)>__l) ? __l : _ustrlen(__p))
#define _ustrspnp(__p1, __p2) ((*((__p1)+_ustrspn(__p1,__p2))) ? ((__p1)+_ustrspn(__p1,__p2)) : NULL)
#endif

#else
// **************************** SBCS and MBCS ********************************


#define _TEOF		EOF

#define __T(x)		x

/* Program */

#define _tmain		main
#define _tWinMain	WinMain
#define _tenviron	environ

/* Formatted i/o */

#define _tprintf	printf
#define _ftprintf	fprintf
#define _stprintf	sprintf
#define _vtprintf	vprintf
#define _vftprintf	vfprintf
#define _vstprintf	vsprintf
#define _tscanf		scanf
#define _ftscanf	fscanf
#define _stscanf	sscanf


/* Unformatted i/o */

#define _fgettc		fgetc
#define _fgettchar	fgetchar
#define _fgetts		fgets
#define _fputtc		fputc
#define _fputtchar	fputchar
#define _fputts		fputs
#define _gettc		getc
#define _gettchar	getchar
#define _getts		gets
#define _puttc		putc
#define _puttchar	putchar
#define _putts		puts
#define _ungettc	ungetc


/* String conversion functions */

#define _tcstod		strtod
#define _tcstol		strtol
#define _tcstoul	strtoul

#define _itot		itoa
#define _ltot		ltoa
#define _ultot		ultoa
#define _ttoi		atoi
#define _ttol		atol


/* String functions */

#define _tcscat		strcat
#define _tcscpy		strcpy
#define _tcslen		strlen
#define _tcsxfrm	strxfrm
#define _tcsdup		strdup


/* Execute functions */

#define _texecl		execl
#define _texecle	execle
#define _texeclp	execlp
#define _texeclpe	execlpe
#define _texecv		execv
#define _texecve	execve
#define _texecvp	execvp
#define _texecvpe	execvpe

#define _tspawnl	spawnl
#define _tspawnle	spawnle
#define _tspawnlp	spawnlp
#define _tspawnlpe	spawnlpe
#define _tspawnv	spawnv
#define _tspawnve	spawnve
#define _tspawnvp	spawnvp
#define _tspawnvpe	spawnvpe

#define _tsystem	system


/* Time functions */

#define _tasctime	asctime
#define _tctime		ctime
#define _tstrdate	_strdate
#define _tstrtime	strtime
#define _tutime		utime
#define _tcsftime	strftime


/* Directory functions */

#define _tchdir		chdir
#define _tgetcwd	getcwd
#define _tgetdcwd	getdcwd
#define _tmkdir		mkdir
#define _trmdir		rmdir


/* Environment/Path functions */

#define _tfullpath	_fullpath
#define _tgetenv	getenv
#define _tmakepath	_makepath
#define _tputenv	putenv
#define _tsearchenv	searchenv
#define _tsplitpath	_splitpath


/* Stdio functions */

#define _tfdopen	fdopen
#define _tfsopen	fsopen
#define _tfopen		fopen
#define _tfreopen	freopen
#define _tperror	perror
#define _ttmpnam	tmpnam


/* Io functions */

#define _taccess	access
#define _tchmod		chmod
#define _tcreat		creat
#define _tfindfirst	_dos_findfirst
#define _tfindnext	_dos_findnext
#define _topen		open
#define _tremove	remove
#define _trename	rename
#define _tsopen		sopen
#define _tunlink	unlink

#define _tfinddata_t	find_t


/* ctype functions */

#define _istascii	isascii
#define _istcntrl	iscntrl
#define _istxdigit	isxdigit


/* Stat functions */

#define _tstat		stat


/* Setlocale functions */

#define _tsetlocale	setlocale


#ifdef _MBCS
// **************************** MBCS *****************************************

#include <mbstring.h>

#ifndef __TCHAR_DEFINED
typedef char		_TCHAR;
typedef signed char	_TSCHAR;
typedef unsigned char	_TUCHAR;
typedef unsigned char	_TXCHAR;
typedef unsigned int	_TINT;
#define __TCHAR_DEFINED
#endif

#ifndef _TCHAR_DEFINED
#if !defined(NO_EXT_KEYS) /* extensions enabled */
typedef char		TCHAR;
#endif
#define _TCHAR_DEFINED
#endif

#ifdef _MB_MAP_DIRECT

/* use mb functions directly - types must match */

/* String functions */

#define _tcschr     _mbschr
#define _tcscspn    _mbscspn
#define _tcsncat    _mbsnbcat
#define _tcsncpy    _mbsnbcpy
#define _tcspbrk    _mbspbrk
#define _tcsrchr    _mbsrchr
#define _tcsspn     _mbsspn
#define _tcsstr     _mbsstr
#define _tcstok     _mbstok

#define _tcsnset    _mbsnbset
#define _tcsrev     _mbsrev
#define _tcsset     _mbsset

#define _tcscmp		_mbscmp
#define _tcsicmp	_mbsicmp
#define _tcsnccmp	_mbsncmp
#define _tcsncmp	_mbsnbcmp
#define _tcsncicmp	_mbsnicmp
#define _tcsnicmp	_mbsnbicmp

#define _tcscoll	_mbscoll
#define _tcsicoll	_mbsicoll
#define _tcsnccoll	_mbsncoll
#define _tcsncoll	_mbsnbcoll
#define _tcsncicoll	_mbsnicoll
#define _tcsnicoll	_mbsnbicoll


/* "logical-character" mappings */

#define _tcsclen    _mbslen
#define _tcsnccat   _mbsncat
#define _tcsnccpy   _mbsncpy
#define _tcsncset   _mbsnset


/* MBCS-specific mappings */

#define	_tcsdec     _mbsdec
#define	_tcsinc     _mbsinc
#define	_tcsnbcnt   _mbsnbcnt
#define	_tcsnccnt   _mbsnccnt
#define	_tcsnextc   _mbsnextc
#define	_tcsninc    _mbsninc
#define	_tcsspnp    _mbsspnp

#define _tcslwr     _mbslwr
#define _tcsupr     _mbsupr

#define _tclen      _mbclen
//#define _tccpy      _mbccpy

//#define _tccmp	    _mbccmp

#else

#if defined(__cplusplus) && defined(__INLINE_FUNCTIONS__)

/* the default: use type-safe inline function thunks */

/* String functions */

inline char *_tcschr( const char *__p, unsigned int __c ) {
    return( (char *)_mbschr( (const unsigned char *)__p, __c ) );
}
inline size_t _tcscspn( const char *__p1, const char *__p2 ) {
    return( _mbscspn( (const unsigned char *)__p1, 
    		      (const unsigned char *)__p2 ) );
}
inline char *_tcsncat( char *__p1, const char *__p2, size_t __l ) {
    return( (char *)_mbsnbcat( (unsigned char *)__p1,
    			       (const unsigned char *)__p2, __l ) );
}
inline char *_tcsncpy( char *__p1, const char *__p2, size_t __l ) {
    return( (char *)_mbsnbcpy( (unsigned char *)__p1,
    			       (const unsigned char *)__p2, __l ) );
}
inline char *_tcspbrk( const char *__p1, const char *__p2 ) {
    return( (char *)_mbspbrk( (const unsigned char *)__p1,
			      (const unsigned char *)__p2 ) );
}
inline char *_tcsrchr( const char *__p, unsigned int __c ) {
    return( (char *)_mbsrchr( (const unsigned char *)__p, __c ) );
}
inline size_t _tcsspn( const char *__p1, const char *__p2 ) {
    return( _mbsspn( (const unsigned char *)__p1,
    		     (const unsigned char *)__p2 ) );
}
inline char *_tcsstr( const char *__p1, const char *__p2 ) {
    return( (char *)_mbsstr( (const unsigned char *)__p1,
			     (const unsigned char *)__p2 ) );
}
inline char *_tcstok( char *__p1, const char *__p2 ) {
    return( (char *)_mbstok( (unsigned char *)__p1,
    			     (const unsigned char *)__p2 ) );
}

inline char *_tcsnset( char *__p, unsigned int __c, size_t __l ) {
    return( (char *)_mbsnbset( (unsigned char *)__p, __c, __l ) );
}
inline char *_tcsrev( char *__p ) {
    return( (char *)_mbsrev( (unsigned char *)__p ) );
}
inline char *_tcsset(char *__p, unsigned int __c ) {
    return( (char *)_mbsset( (unsigned char *)__p, __c ) );
}

inline int _tcscmp( const char *__p1, const char *__p2 ) {
    return( _mbscmp( (const unsigned char *)__p1, 
    		     (const unsigned char *)__p2 ) );
}
inline int _tcsicmp( const char *__p1, const char *__p2 ) {
    return( _mbsicmp( (const unsigned char *)__p1, 
    		      (const unsigned char *)__p2 ) );
}
inline int _tcsnccmp( const char *__p1, const char *__p2, size_t __l ) {
    return( _mbsncmp( (const unsigned char *)__p1, 
    		      (const unsigned char *)__p2, __l ) );
}
inline int _tcsncmp( const char *__p1, const char *__p2, size_t __l ) {
    return( _mbsnbcmp( (const unsigned char *)__p1, 
    		       (const unsigned char *)__p2, __l ) );
}
inline int _tcsncicmp( const char *__p1, const char *__p2, size_t __l ) {
    return( _mbsnicmp( (const unsigned char *)__p1, 
    		       (const unsigned char *)__p2, __l ) );
}
inline int _tcsnicmp( const char *__p1, const char *__p2, size_t __l ) {
    return( _mbsnbicmp( (const unsigned char *)__p1, 
		        (const unsigned char *)__p2, __l ) );
}

inline int _tcscoll( const char *__p1, const char *__p2 ) {
    return( _mbscoll( (const unsigned char *)__p1, 
		      (const unsigned char *)__p2 ) );
}
inline int _tcsicoll( const char *__p1, const char *__p2 ) {
    return( _mbsicoll( (const unsigned char *)__p1, 
		       (const unsigned char *)__p2 ) );
}
inline int _tcsnccoll( const char *__p1, const char *__p2, size_t __l ) {
    return( _mbsncoll( (const unsigned char *)__p1, 
		       (const unsigned char *)__p2, __l ) );
}
inline int _tcsncoll( const char *__p1, const char *__p2, size_t __l ) {
    return( _mbsnbcoll( (const unsigned char *)__p1, 
			(const unsigned char *)__p2, __l ) );
}
inline int _tcsncicoll( const char *__p1, const char *__p2, size_t __l ) {
    return( _mbsnicoll( (const unsigned char *)__p1, 
		        (const unsigned char *)__p2, __l ) );
}
inline int _tcsnicoll( const char *__p1, const char *__p2, size_t __l ) {
    return( _mbsnbicoll( (const unsigned char *)__p1, 
			 (const unsigned char *)__p2, __l ) );
}


/* "logical-character" mappings */

inline size_t _tcsclen( const char *__p ) {
    return( _mbslen( (const unsigned char *)__p ) );
}
inline char *_tcsnccat( char *__p1, const char *__p2, size_t __l ) {
    return( (char *)_mbsncat( (unsigned char *)__p1, 
			      (const unsigned char *)__p2, __l ) );
}
inline char *_tcsnccpy( char *__p1, const char *__p2, size_t __l ) {
    return( (char *)_mbsncpy( (unsigned char *)__p1, 
			      (const unsigned char *)__p2, __l ) );
}
inline char *_tcsncset( char *__p, unsigned int __c, size_t __l ) {
    return( (char *)_mbsnset( (unsigned char *)__p, __c, __l ) );
}


/* MBCS-specific mappings */

inline char *_tcsdec( const char *__p1, const char *__p2 ) {
    return( (char *)_mbsdec( (const unsigned char *)__p1, 
			     (const unsigned char *)__p2 ) );
}
inline char *_tcsinc( const char *__p ) {
    return( (char *)_mbsinc( (const unsigned char *)__p ) );
}
inline size_t _tcsnbcnt( const char *__p, size_t __l ) {
    return( _mbsnbcnt( (const unsigned char *)__p, __l ) );
}
inline size_t _tcsnccnt( const char *__p, size_t __l ) {
    return( _mbsnccnt( (const unsigned char *)__p, __l ) );
}
inline char *_tcsninc( const char *__p, size_t __l ) {
    return( (char *)_mbsninc( (const unsigned char *)__p, __l ) );
}
inline char *_tcsspnp( const char *__p1, const char *__p2 ) {
    return( (char *)_mbsspnp( (const unsigned char *)__p1, 
			      (const unsigned char *)__p2 ) );
}
inline char *_tcslwr( char *__p ) {
    return( (char *)_mbslwr( (unsigned char *)__p ) );
}
inline char *_tcsupr( char *__p ) {
    return( (char *)_mbsupr( (unsigned char *)__p ) );
}

inline size_t _tclen( const char *__p ) {
    return( _mbclen( (const unsigned char *)__p ) );
}

//inline void _tccpy( char *__p1, const char *__p2 ) {
//    _mbccpy( (unsigned char *)__p1, (const unsigned char *)__p2 );
//    return;
//}

//inline int _tccmp( const char *__p1, const char *__p2 ) {
//    return _mbccmp( (const unsigned char *)__p1, (const unsigned char *)__p2 );
//}


/* inline helper */
inline unsigned int _tcsnextc( const char *__p ) {
    unsigned int __l=0; 
    if( _mbislead((unsigned int)*(unsigned char *)__p) )  {
	__l=((unsigned int)*__p++)<<8; 
    }
    __l+=(unsigned int)*__p; return(__l);
}

#else

/* use type-safe linked-in function thunks */

/* String functions */

char *_tcschr(const char *, unsigned int);
size_t _tcscspn(const char *, const char *);
char *_tcsncat(char *, const char *, size_t);
char *_tcsncpy(char *, const char *, size_t);
char *_tcspbrk(const char *, const char *);
char *_tcsrchr(const char *, int);
size_t _tcsspn(const char *, const char *);
char *_tcsstr(const char *, const char *);
char *_tcstok(char *, const char *);

char *_tcsnset(char *, unsigned int, size_t);
char *_tcsrev(char *);
char *_tcsset(char *, unsigned int);

int _tcscmp(const char *, const char *);
int _tcsicmp(const char *, const char *);
int _tcsnccmp(const char *, const char *, size_t);
int _tcsncmp(const char *, const char *, size_t);
int _tcsncicmp(const char *, const char *, size_t);
int _tcsnicmp(const char *, const char *, size_t);

int _tcscoll(const char *, const char *);
int _tcsicoll(const char *, const char *);
int _tcsnccoll(const char *, const char *, size_t);
int _tcsncoll(const char *, const char *, size_t);
int _tcsncicoll(const char *, const char *, size_t);
int _tcsnicoll(const char *, const char *, size_t);


/* "logical-character" mappings */

size_t _tcsclen(const char *);
char *_tcsnccat(char *, const char *, size_t);
char *_tcsnccpy(char *, const char *, size_t);
char *_tcsncset(char *, unsigned int, size_t);


/* MBCS-specific mappings */

char *_tcsdec(const char *, const char *);
char *_tcsinc(const char *);
size_t _tcsnbcnt(const char *, size_t);
size_t _tcsnccnt(const char *, size_t);
unsigned int _tcsnextc (const char *);
char *_tcsninc(const char *, size_t);
char *_tcsspnp(const char *, const char *);

char *_tcslwr(char *);
char *_tcsupr(char *);

size_t _tclen(const char *);
void _tccpy(char *, const char *);
int _tccmp(const char *, const char *);

#endif

#endif


/* ctype functions */

#define _istalnum   _ismbcalnum
#define _istalpha   _ismbcalpha
#define _istdigit   _ismbcdigit
#define _istgraph   _ismbcgraph
#define _istlegal   _ismbclegal
#define _istlower   _ismbclower
#define _istprint   _ismbcprint
#define _istpunct   _ismbcpunct
#define _istspace   _ismbcspace
#define _istupper   _ismbcupper

#define _totupper   _mbctoupper
#define _totlower   _mbctolower

#define _istlead    _mbislead
#define _istleadbyte    isleadbyte


#else
// **************************** SBCS *****************************************

#ifndef __TCHAR_DEFINED
typedef char		_TCHAR;
typedef signed char	_TSCHAR;
typedef unsigned char	_TUCHAR;
typedef char		_TXCHAR;
typedef int		_TINT;
#define __TCHAR_DEFINED
#endif

#ifndef _TCHAR_DEFINED
#if !defined(NO_EXT_KEYS) /* extensions enabled */
typedef char		TCHAR;
#endif
#define _TCHAR_DEFINED
#endif

/* String functions */

#define _tcschr		strchr
#define _tcscspn	strcspn
#define _tcsncat	strncat
#define _tcsncpy	strncpy
#define _tcspbrk	strpbrk
#define _tcsrchr	strrchr
#define _tcsspn		strspn
#define _tcsstr		strstr
#define _tcstok		strtok

#define _tcsnset	strnset
#define _tcsrev		strrev
#define _tcsset		strset

#define _tcscmp		strcmp
#define _tcsicmp	stricmp
#define _tcsnccmp	strncmp
#define _tcsncmp	strncmp
#define _tcsncicmp	strnicmp
#define _tcsnicmp	strnicmp

#define _tcscoll	strcoll
#define _tcsicoll	stricoll
#define _tcsnccoll	strncoll
#define _tcsncoll	strncoll
#define _tcsncicoll	strnicoll
#define _tcsnicoll	strnicoll


/* "logical-character" mappings */

#define _tcsclen	strlen
#define _tcsnccat	strncat
#define _tcsnccpy	strncpy
#define _tcsncset	strnset

#define	_tcsdec		_strdec
#define	_tcsinc		_strinc
#define	_tcsnbcnt	_strncnt
#define	_tcsnccnt	_strncnt
#define	_tcsnextc	_strnextc
#define	_tcsninc	_strninc
#define	_tcsspnp	_strspnp

#define _tcslwr		strlwr
#define _tcsupr		strupr
#define _tcsxfrm	strxfrm

#if defined(__cplusplus) && defined(__INLINE_FUNCTIONS__)
inline size_t _tclen( const char * ) {
    return( 1 );
}
inline void _tccpy( char *__p1, const char *__p2 ) {
    *__p1 = *__p2;
}
inline int _tccmp( const char *__p1, const char *__p2 ) {
    return( (int) (((unsigned char)*__p1)-((unsigned char)*__p2)) );
}
#else
#define _tclen(__p)	(1)
#define _tccpy(__p1,__p2) ((*(__p1) = *(__p2)))
#define _tccmp(__p1,__p2) (((unsigned char)*(__p1))-((unsigned char)*(__p2)))
#endif


/* ctype functions */

#define _istalnum	isalnum
#define _istalpha	isalpha
#define _istdigit	isdigit
#define _istgraph	isgraph
#define _istlower	islower
#define _istprint	isprint
#define _istpunct	ispunct
#define _istspace	isspace
#define _istupper	isupper

#define _totupper	toupper
#define _totlower	tolower

#define _istlegal(_c)   (1)
#define _istlead(_c)    (0)
#define _istleadbyte(_c)    (0)

#if defined(__cplusplus) && defined(__INLINE_FUNCTIONS__)
inline char *_strdec( const char *, const char *__p ) {
    return( (char *)(__p-1) );
}
inline char *_strinc( const char *__p ) {
    return( (char *)(__p+1) );
}
inline unsigned int _strnextc( const char *__p ) {
    return( (unsigned int)*__p );
}
inline char *_strninc( const char *__p, size_t __l ) {
    return( (char *)(__p+__l) );
}
inline size_t _strncnt( const char *__p, size_t __l ) {
    size_t __i;
    __i = strlen(__p);
    return( (__i>__l) ? __l : __i );
}
inline char *_strspnp( const char *__p1, const char *__p2 ) {
    return( (*(__p1 += strspn(__p1,__p2))!='\0') ? (char*)__p1 : NULL );
}
#else
#define _strdec(__p1, __p2)  ((__p2)-1)
#define _strinc(__p)	     ((__p)+1)
#define _strnextc(__p)	     ((unsigned int) *(__p))
#define _strninc(__p, __l)   (((__p)+(__l)))
#define _strncnt(__p, __l)   ((strlen(__p)>__l) ? __l : strlen(__p))
#define _strspnp(__p1, __p2) ((*((__p1)+strspn(__p1,__p2))) ? ((__p1)+strspn(__p1,__p2)) : NULL)
#endif


#endif
#endif

#define _T(x)		__T(x)
#define _TEXT(x)	__T(x)

#define _TCHAR_H_INCLUDED
#ifdef __cplusplus
};
#endif
#endif
