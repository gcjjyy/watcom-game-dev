/*
 *  time.h	Time functions
 *
 *  Copyright by WATCOM International Corp. 1988-1996.  All rights reserved.
 */
#ifndef _TIME_H_INCLUDED
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

#ifndef _TIME_T_DEFINED_
 #define _TIME_T_DEFINED_
 typedef unsigned long time_t;	 /* time value */
#endif

#define CLK_TCK 	100
#define CLOCKS_PER_SEC	100

#ifndef _CLOCK_T_DEFINED
 #define _CLOCK_T_DEFINED
 typedef unsigned long	clock_t;
#endif

struct	tm {
	int  tm_sec;	/* seconds after the minute -- [0,61] */
	int  tm_min;	/* minutes after the hour   -- [0,59] */
	int  tm_hour;	/* hours after midnight     -- [0,23] */
	int  tm_mday;	/* day of the month	    -- [1,31] */
	int  tm_mon;	/* months since January     -- [0,11] */
	int  tm_year;	/* years since 1900		      */
	int  tm_wday;	/* days since Sunday	    -- [0,6]  */
	int  tm_yday;	/* days since January 1     -- [0,365]*/
	int  tm_isdst;	/* Daylight Savings Time flag */
};

extern char *asctime( const struct tm *__timeptr );
extern clock_t clock( void );
extern char *ctime( const time_t *__timer );
extern double difftime( time_t __t1, time_t __t0 );
extern struct tm *gmtime( const time_t *__timer );
extern struct tm *localtime( const time_t *__timer );
extern time_t mktime( struct tm *__timeptr );
extern size_t strftime( char *__s, size_t __maxsiz, const char *__fmt,
		 const struct tm *__tp );
extern time_t time( time_t *__timer );

#ifndef __cplusplus
#define difftime(t1,t0) ((double)(t1)-(double)(t0))
#endif

#if !defined(NO_EXT_KEYS) /* extensions enabled */
extern char *_asctime( const struct tm *__timeptr, char *__buf );
extern char *_ctime( const time_t *__timer, char *__buf );
extern struct tm *_gmtime( const time_t *__timer, struct tm *__tmbuf );
extern struct tm *_localtime( const time_t *__timer, struct tm *__tmbuf );
extern char *_strdate( char *__buf );
extern char *_strtime( char *__buf );
#if defined(__NT__) && (defined(__UNICODE__) || defined(_UNICODE))
/*
 * Unicode prototypes
 */
extern _unichar *_uasctime( const struct tm * );
extern _unichar *_uctime( const time_t * );
extern _unichar *_ustrdate( _unichar *__buf );
extern _unichar *_ustrtime( _unichar *__buf );
extern size_t _ustrftime( _unichar *, size_t, const _unichar *, const struct tm * );
extern size_t _ustrftime_ms( _unichar *, size_t, const char *, const struct tm * );
extern _unichar *_u_ctime( const time_t *, _unichar * );
extern _unichar *_u_asctime( const struct tm *, _unichar * );
#endif
#endif

extern void tzset( void );
extern int __CacheOSTZ( int __new_flag );

 extern char	*tzname[2]; /*	time zone names */
 extern long	timezone;	/* # of seconds from GMT */
 extern int	daylight;	/* d.s.t. indicator */

#if __WATCOMC__ > 1000
#pragma pack(__pop);
#else
#pragma pack();
#endif

#define _TIME_H_INCLUDED
#ifdef __cplusplus
};
#endif
#endif
