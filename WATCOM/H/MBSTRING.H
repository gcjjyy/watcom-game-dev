/*
 *  mbstring.h	Multi-Byte Character Set (MBCS) functions
 *
 *  Copyright by WATCOM International Corp. 1988-1996.  All rights reserved.
 */
#ifndef _MBSTRING_H_INCLUDED
#ifdef __cplusplus
extern "C" {
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

#include <string.h>	// for LIFEBOAT code

#ifndef _MB_TABLE_FLAGS_DEFINED
    #define _MB_LEAD		0x01
    #define _MB_TABLE_FLAGS_DEFINED
#endif


extern unsigned char *	_mbsncpy( unsigned char *__s1, const unsigned char *__s2, size_t __n );
extern unsigned char *	_mbsnbcpy( unsigned char *__s1, const unsigned char *__s2, size_t __n );
extern unsigned char *	_mbscat( unsigned char *__s1, const unsigned char *__s2 );
extern unsigned char *	_mbsncat( unsigned char *__s1, const unsigned char *__s2, size_t __n );
extern unsigned char *	_mbsnbcat( unsigned char *__s1, const unsigned char *__s2, size_t __n );

extern unsigned char *	_mbschr( const unsigned char *__string, unsigned int __ch );
extern unsigned char *	_mbsrchr( const unsigned char *__string, unsigned int __ch );
extern unsigned char *	_mbsstr( const unsigned char *__string, const unsigned char *__substring );
extern unsigned char *	_mbspbrk( const unsigned char *__string, const unsigned char *__charset );
extern unsigned char *	_mbstok( unsigned char *__str, const unsigned char *__delim );

extern size_t		_mbsspn( const unsigned char *__string, const unsigned char *__charset );
extern size_t		_mbscspn( const unsigned char *__string, const unsigned char *__charset );
extern size_t		_mbslen( const unsigned char *__string );

extern unsigned char *	_mbsset( unsigned char *__string, unsigned int __ch );
extern unsigned char *	_mbsnset( unsigned char *__string, unsigned int __ch, size_t __n );
extern unsigned char *	_mbsnbset( unsigned char *__string, unsigned int __ch, size_t __n );
extern unsigned char *	_mbsrev( unsigned char *__string );

extern int		_mbscmp( const unsigned char *__s1, const unsigned char *__s2 );
extern int		_mbsicmp( const unsigned char *__s1, const unsigned char *__s2 );
extern int		_mbsncmp( const unsigned char *__s1, const unsigned char *__s2, size_t __n );
extern int		_mbsnicmp( const unsigned char *__s1, const unsigned char *__s2, size_t __n );
extern int		_mbsnbcmp( const unsigned char *__s1, const unsigned char *__s2, size_t __n );
extern int		_mbsnbicmp( const unsigned char *__s1, const unsigned char *__s2, size_t __n );
extern int		_mbccmp( const unsigned char *__s1, const unsigned char *__s2 );
extern int		_mbcicmp( const unsigned char *__c1, const unsigned char *__c2 );

extern unsigned char *	_mbsdec( const unsigned char *__stringStart, const unsigned char *__ch );
extern unsigned char *	_mbsinc( const unsigned char *__string );
extern void		_mbccpy( unsigned char *__dest, const unsigned char *__ch );
extern size_t		_mbclen( const unsigned char *__ch );

extern size_t		_mbsnbcnt( const unsigned char *__string, size_t __n );
extern size_t		_mbsnccnt( const unsigned char *__string, size_t __n );
extern unsigned int	_mbsnextc( const unsigned char *__string );
extern unsigned char *	_mbsninc( const unsigned char *__string, size_t __count );
extern unsigned char *	_mbsspnp( const unsigned char *__string, const unsigned char *__charset );

extern unsigned char *	_mbslwr( unsigned char *__string );
extern unsigned char *	_mbsupr( unsigned char *__string );
extern unsigned int	_mbctolower( unsigned int __ch );
extern unsigned int	_mbctoupper( unsigned int __ch );

extern int		wctomb( char *__s, wchar_t __wchar );
extern size_t		wcstombs( char *__s, const wchar_t *__pwcs, size_t __n );
extern int		mbtowc( wchar_t *__pwc, const char *__string, size_t __n );
extern size_t		mbstowcs( wchar_t *__pwcs, const char *__s, size_t __n );

extern unsigned int	_mbptov( const unsigned char *__ch );
extern unsigned char *	_mbvtop( unsigned int __ch, unsigned char *__addr );
extern unsigned char *	__mbvtop( unsigned int __ch );

extern size_t		__mbslen( const unsigned char *__string );
extern unsigned char *	__mbsninc( const unsigned char *__string, size_t __count );


/* Far versions (16 bit only) */
#ifndef __386__
extern unsigned char __far *	_fmbsncpy( unsigned char __far *__s1, const unsigned char __far *__s2, size_t __n );
extern unsigned char __far *	_fmbsnbcpy( unsigned char __far *__s1, const unsigned char __far *__s2, size_t __n );
extern unsigned char __far *	_fmbscat( unsigned char __far *__s1, const unsigned char __far *__s2 );
//extern unsigned char *	_mbsncat( unsigned char *__s1, const unsigned char *__s2, size_t __n );
//extern unsigned char *	_mbsnbcat( unsigned char *__s1, const unsigned char *__s2, size_t __n );

extern unsigned char __far *	_fmbschr( const unsigned char __far *__string, unsigned int __ch );
extern unsigned char __far *	_fmbsrchr( const unsigned char __far *__string, unsigned int __ch );
//extern unsigned char *	_mbsstr( const unsigned char *__string, const unsigned char *__substring );
extern unsigned char __far *	_fmbspbrk( const unsigned char __far *__string, const unsigned char __far *__charset );
extern unsigned char __far *	_fmbstok( unsigned char __far *__str, const unsigned char __far *__delim );

//extern size_t		_mbsspn( const unsigned char *__string, const unsigned char *__charset );
extern size_t			_fmbscspn( const unsigned char __far *__string, const unsigned char __far *__charset );
extern size_t			_fmbslen( const unsigned char __far *__string );

//extern unsigned char *	_mbsset( unsigned char *__string, unsigned int __ch );
//extern unsigned char *	_mbsnset( unsigned char *__string, unsigned int __ch, size_t __n );
//extern unsigned char *	_mbsnbset( unsigned char *__string, unsigned int __ch, size_t __n );
//extern unsigned char *	_mbsrev( unsigned char *__string );

//extern int		_mbscmp( const unsigned char *__s1, const unsigned char *__s2 );
//extern int		_mbsicmp( const unsigned char *__s1, const unsigned char *__s2 );
extern int			_fmbsncmp( const unsigned char __far *__s1, const unsigned char __far *__s2, size_t __n );
//extern int		_mbsnicmp( const unsigned char *__s1, const unsigned char *__s2, size_t __n );
extern int			_fmbsnbcmp( const unsigned char __far *__s1, const unsigned char __far *__s2, size_t __n );
//extern int		_mbsnbicmp( const unsigned char *__s1, const unsigned char *__s2, size_t __n );
extern int			_fmbccmp( const unsigned char __far *__s1, const unsigned char __far *__s2 );
//extern int		_mbcicmp( const unsigned char *__c1, const unsigned char *__c2 );

extern unsigned char __far *	_fmbsdec( const unsigned char __far *__stringStart, const unsigned char __far *__ch );
extern unsigned char __far *	_fmbsinc( const unsigned char __far *__string );
extern void			_fmbccpy( unsigned char __far *__dest, const unsigned char __far *__ch );
//extern size_t		_mbclen( const unsigned char *__ch );

extern size_t			_fmbsnbcnt( const unsigned char __far *__string, size_t __n );
extern size_t			_fmbsnccnt( const unsigned char __far *__string, size_t __n );
extern unsigned int		_fmbsnextc( const unsigned char __far *__string );
extern unsigned char __far *	_fmbsninc( const unsigned char __far *__string, size_t __count );
extern unsigned char __far *	_fmbsspnp( const unsigned char __far *__string, const unsigned char __far *__charset );

//extern unsigned char *	_mbslwr( unsigned char *__string );
//extern unsigned char *	_mbsupr( unsigned char *__string );
//extern unsigned int	_mbctolower( unsigned int __ch );
//extern unsigned int	_mbctoupper( unsigned int __ch );

//extern int		wctomb( char *__s, wchar_t __wchar );
//extern size_t		wcstombs( char *__s, const wchar_t *__pwcs, size_t __n );
//extern int		mbtowc( wchar_t *__pwc, const char *__string, size_t __n );
//extern size_t		mbstowcs( wchar_t *__pwcs, const char *__s, size_t __n );

extern unsigned int		_fmbptov( const unsigned char __far *__ch );
extern unsigned char __far *	_fmbvtop( unsigned int __ch, unsigned char __far *__addr );
extern unsigned char __far *	__fmbvtop( unsigned int __ch );
#endif


/* Global data */
extern unsigned char	__MBCSIsTable[257];
extern int		__IsDBCS;


/* Since only "C" locale supported, we can remap locale compare functions */
#define _mbscoll    	_mbscmp
#define _mbsicoll   	_mbsicmp
#define _mbsncoll   	_mbsncmp
#define _mbsnbcoll  	_mbsnbcmp
#define _mbsnicoll  	_mbsnicmp
#define _mbsnbicoll	_mbsnbicmp


#ifndef MB_CUR_MAX
    #define MB_CUR_MAX	2
#endif
#ifndef MB_LEN_MAX
    #define MB_LEN_MAX	2
#endif


/**
 * types used by LIFEBOAT code
 **/
typedef unsigned char	_UCHAR;	    /* Charactor is unsigned type for DBCS */
typedef _UCHAR *	_USTRING;   /* String is unsigned type for DBCS */
typedef _UCHAR __far *	_FUSTRING;  /* Far string is unsigned type for DBCS */
typedef unsigned int	DBCODE;	    /* äøéöÉRÅ[Éh(2byte code) */

/**
 * character byte types  
 **/
#define _MBC_SINGLE	0	/* valid single-byte char */
#define _MBC_LEAD	1	/* Kanji 1st byte (lead byte) */
#define _MBC_TRAIL	2	/* Kanji 2nd bytee (trailing byte) */
#define _MBC_ILLEGAL	(-1)	/* illegal charactor byte */

/**
 * prototypes used by LIFEBOAT code
 **/
_USTRING    _mbgetcode( _USTRING, DBCODE * );
_FUSTRING   _fmbgetcode( _FUSTRING, DBCODE * );
_USTRING    _mbputchar( _USTRING, DBCODE );
_FUSTRING   _fmbputchar( _FUSTRING, DBCODE );
int	    _mbsbtype( const _USTRING, size_t );
int	    _fmbsbtype( const _FUSTRING, size_t );
int	    _mbbtype( _UCHAR, int );

/**
 * distinguish character
 **/
int  _ismbcalpha(DBCODE);
int  _ismbcdigit(DBCODE);
int  _ismbclegal(DBCODE);
int  _ismbclower(DBCODE);
int  _ismbcprint(DBCODE);
int  _ismbcspace(DBCODE);
int  _ismbcupper(DBCODE);

int  _ismbddigit(DBCODE);
int  _ismbdlower(DBCODE);
int  _ismbdspace(DBCODE);
int  _ismbdupper(DBCODE);

/**
 *  distinguish and translate Japanese DBCS
 **/
int  _ismbchira(DBCODE);
int  _ismbckata(DBCODE);
int  _ismbcsymbol(DBCODE);
int  _ismbcl0(DBCODE);
int  _ismbcl1(DBCODE);
int  _ismbcl2(DBCODE);
DBCODE  _mbcjistojms(DBCODE);
DBCODE  _mbcjmstojis(DBCODE);
DBCODE  _mbctohira(DBCODE);
DBCODE  _mbctokata(DBCODE);
DBCODE  _mbctolower(DBCODE);
DBCODE  _mbctoupper(DBCODE);


/* Implement small functions as macros for speed */
#ifndef _mbislead
#define _mbislead(__b)		( __IsDBCS						\
				? ( __MBCSIsTable[(unsigned char)(__b)+1] & _MB_LEAD )  \
				: ( 0 ) )
#endif
			
#define _mbsinc(__s)		( (__s) + 1 + _mbislead(*(__s)) )
#define _mbclen(__c)		( 1 + _mbislead(*(__c)) )
#define _mbccmp(__c1,__c2)	( _mbsncmp(__c1,__c2,1) )
#define _mbcicmp(__c1,__c2)	( _mbsnicmp(__c1,__c2,1) )

#ifndef __386__
#define _fmbsinc(__s)		( (__s) + 1 + _mbislead(*(__s)) )
#define _fmbclen(__c)		( 1 + _mbislead(*(__c)) )
#define _fmbccmp(__c1,__c2)	( _fmbsncmp(__c1,__c2,1) )
#define _fmbcicmp(__c1,__c2)	( _fmbsnicmp(__c1,__c2,1) )
#endif


#define _MBSTRING_H_INCLUDED
#ifdef __cplusplus
};
#endif
#endif
