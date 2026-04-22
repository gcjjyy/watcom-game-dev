/*
 *  mbctype.h	Multi-Byte Character functions
 *
 *  Copyright by WATCOM International Corp. 1988-1996.  All rights reserved.
 */
#ifndef _MBCTYPE_H_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#ifndef _SIZE_T_DEFINED_
#define _SIZE_T_DEFINED_
 typedef unsigned size_t;
#endif

#ifndef NULL
 #if defined(__SMALL__) || defined(__MEDIUM__) || defined(__386__)
  #define NULL   0
 #else
  #define NULL   0L
 #endif
#endif

#include <ctype.h>	// for LIFEBOAT code

#ifndef _MB_TABLE_FLAGS_DEFINED
    #define _MB_LEAD		0x01
    #define _MB_TABLE_FLAGS_DEFINED
#endif


//extern int		_ismbcalnum( unsigned int __ch );
extern int		_ismbcalpha( unsigned int __ch );
//extern int		_ismbccntrl( unsigned int __ch );
extern int		_ismbcdigit( unsigned int __ch );
//extern int		_ismbcgraph( unsigned int __ch );
extern int		_ismbclower( unsigned int __ch );
extern int		_ismbcprint( unsigned int __ch );
//extern int		_ismbcpunct( unsigned int __ch );
extern int		_ismbcspace( unsigned int __ch );
extern int		_ismbcupper( unsigned int __ch );
//extern int		_ismbcxdigit( unsigned int __ch );


// /* For Microsoft compatability */
// #define _ismbbalnum	_ismbcalnum
// #define _ismbbalpha	_ismbcalpha
// #define _ismbbgraph	_ismbcgraph
// #define _ismbbkalnum	_ismbcalnum
// #define _ismbbkprint	_ismbcprint
// #define _ismbbkpunct	_ismbcpunct
// #define _ismbblead  	_mbislead
// #define _ismbbprint 	_ismbcprint
// #define _ismbbpunct 	_ismbcpunct
// //#define _ismbbkana	(unsupported)
// //#define _ismbbtrail 	(unsupported)


/* Global data */
extern unsigned char	__MBCSIsTable[257];
extern int		__IsDBCS;


/* Implement small functions as macros for speed */
#ifndef _mbislead
#define _mbislead(__b)	( __IsDBCS						\
			? ( __MBCSIsTable[(unsigned char)(__b)+1] & _MB_LEAD )	\
			: ( 0 ) )
#endif


/* _setmbcp parameter defines */
#define _MB_CP_SBCS     0
#define _MB_CP_OEM      (-2)
#define _MB_CP_ANSI     (-3)

/* MB control routines */
extern int _setmbcp(int);
extern int _getmbcp(void);


#ifndef MB_CUR_MAX
    #define MB_CUR_MAX	2
#endif
#ifndef MB_LEN_MAX
    #define MB_LEN_MAX	2
#endif


/**
 *  define bit masks for multi-byte characters type 
 **/
#define _M1ST    1	/* 漢字第 1 バイト	MBCS 1st byte */
#define _M2ND    2	/* 漢字第 2 バイト	MBCS 2nd byte */
#define _MSYMB   4	/* 1 バイト仮名文字	MBCS single-byte kana */
#define _MPUNCT  8	/* 仮名句切文字		MBCS single-byte kana punct */

/**
 * character byte types  
 **/
#define _MBC_SINGLE	0	/* valid single-byte char */
#define _MBC_LEAD	1	/* MBSC 1st byte(lead byte) */
#define _MBC_TRAIL	2	/* MBSC 2nd byte(trailing byte) */
#define _MBC_ILLEGAL	(-1)	/* illegal character byte type */

/**
 *  function prototypes for Multi-byte character set 
 **/
int  _ismbbkalpha( unsigned short );
int  _ismbbkana( unsigned short );
int  _ismbbkpunct( unsigned short );
int  _ismbbalpha( unsigned short );
int  _ismbbpunct( unsigned short );
int  _ismbbalnum( unsigned short );
int  _ismbbprint( unsigned short );
int  _ismbbgraph( unsigned short );
int  _ismbblead( unsigned short );
int  _ismbbtrail( unsigned short );

/**
 * function macros 
 **/
#define _ismbbkalpha(__c)	( __IsDBCS						\
				? ( __MBCSIsTable[(unsigned char)(__c)+1] & _MSYMB )	\
				: ( isalpha(__c) ) )

#define _ismbbkana(__c)		( __IsDBCS							\
				? ( __MBCSIsTable[(unsigned char)(__c)+1] & (_MSYMB|_MPUNCT) )	\
				: ( isalpha(__c) || ispunct(__c) ) )

#define _ismbbkpunct(__c)	( __IsDBCS						\
				? ( __MBCSIsTable[(unsigned char)(__c)+1] & _MPUNCT )	\
				: ( ispunct(__c) ) )

#define _ismbbalpha(__c)	(isalpha(__c) || _ismbbkana(__c))
#define _ismbbpunct(__c)	(ispunct(__c) || _ismbbkpunct(__c))
#define _ismbbalnum(__c)	(isalnum(__c) || _ismbbkana(__c))
#define _ismbbprint(__c)	(isprint(__c) || _ismbbkana(__c))
#define _ismbbgraph(__c)	(isgraph(__c) || _ismbbkana(__c))

#define _ismbblead(__c)		_mbislead(__c)
#define _ismbbtrail(__c)	( __IsDBCS						\
				? ( __MBCSIsTable[(unsigned char)(__c)+1] & _M2ND )	\
				: ( 0 ) )


#define _MBCTYPE_H_INCLUDED
#ifdef __cplusplus
};
#endif
#endif
