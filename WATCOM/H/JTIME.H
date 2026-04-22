/*
 *  jtime.h     Japanese time functions
 *
 *  Copyright by WATCOM International Corp. 1988-1996.  All rights reserved.
 */
#ifndef _JTIME_H_INCLUDED

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

#ifndef _TIME_H_INCLUDED
#include <time.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
extern unsigned char   *jasctime( const struct tm *__timeptr );
extern unsigned char   *jctime( const time_t *__timer );
#ifdef __cplusplus
};
#endif

#define _JTIME_H_INCLUDED
#endif
