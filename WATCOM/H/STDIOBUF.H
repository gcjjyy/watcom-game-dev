//
//  stdiobuf.h	Standard I/O streams
//
//  Copyright by WATCOM International Corp. 1988-1996.  All rights reserved.
//
#ifndef _STDIOBUF_H_INCLUDED

#ifndef __cplusplus
#error stdiobuf.h is for use with C++
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

#include <stdio.h>
#include <iostream.h>

// **************************** STDIOBUF *************************************
#if __WATCOMC__ > 1000
#pragma pack(__push,1);
#else
#pragma pack(1);
#endif
class stdiobuf : public streambuf {
public:
    stdiobuf();
    stdiobuf( FILE *__fptr );
    ~stdiobuf();

    FILE *stdiofile();
    
    virtual int overflow( int = EOF );
    virtual int underflow();
    virtual int	sync();

private:
    FILE *__file_pointer;
    char __unbuffered_get_area[ DEFAULT_PUTBACK_SIZE+1 ];
    int : 0;
};
#if __WATCOMC__ > 1000
#pragma pack(__pop);
#else
#pragma pack();
#endif

inline FILE *stdiobuf::stdiofile() {
    return __file_pointer;
}

#define _STDIOBUF_H_INCLUDED
#endif
