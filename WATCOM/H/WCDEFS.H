//
//  wcdefs.h   Definitions for the WATCOM Container Classes
//
//  Copyright by WATCOM International Corp. 1988-1996.  All rights reserved.
//

#ifndef _WCDEFS_H_INCLUDED

#ifndef __cplusplus
#error wcdefs.h is for use with C++
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

#include <except.h>

typedef int             WCbool;

#ifndef TRUE
    #define TRUE        ((WCbool)1)
    #define FALSE       ((WCbool)0)
#endif

#ifndef _SIZE_T_DEFINED_
#define _SIZE_T_DEFINED_
 typedef unsigned size_t;
#endif


#define _WCDEFS_H_INCLUDED
#endif
