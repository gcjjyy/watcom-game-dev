/*
 *  sys/timeb.h     timeb structure used with ftime()
 *
 *  Copyright by WATCOM International Corp. 1988-1996.  All rights reserved.
 */
#ifndef __TIMEB_H_INCLUDED
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

#ifndef _TIME_T_DEFINED_
 #define _TIME_T_DEFINED_
 typedef unsigned long time_t;	 /* time value */
#endif

#if __WATCOMC__ > 1000
#pragma pack(__push,1);
#else
#pragma pack(1);
#endif
struct timeb {
    time_t		time;		/* seconds since Jan 1, 1970 UTC */
    unsigned short 	millitm;	/* milliseconds */
    short		timezone;	/* difference in minutes from UTC */
    short		dstflag;	/* nonzero if daylight savings time */
};
#if __WATCOMC__ > 1000
#pragma pack(__pop);
#else
#pragma pack();
#endif

extern int ftime( struct timeb *__timeptr );

#define __TIMEB_H_INCLUDED
#ifdef __cplusplus
};
#endif
#endif
