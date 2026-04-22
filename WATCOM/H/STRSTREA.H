//
//  strstrea.h	String streams
//
//  Copyright by WATCOM International Corp. 1988-1996.  All rights reserved.
//
#ifndef _STRSTREAM_H_INCLUDED

#ifndef __cplusplus
#error strstrea.h is for use with C++
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

#include <iostream.h>

// **************************** STRSTREAMBUF *********************************
#if __WATCOMC__ > 1000
#pragma pack(__push,1);
#else
#pragma pack(1);
#endif
class strstreambuf : public streambuf {
public:
    strstreambuf();
    strstreambuf( int __allocation_size );
    strstreambuf( void *(*__alloc_fn)( long ), void (*__free_fn)( void * ) );
    strstreambuf( char *__str, int __size, char *__pstart = NULL );
    ~strstreambuf();

    int   alloc_size_increment( int __increment );
    void  freeze( int __freeze = 1 );
    char *str();

    virtual int        overflow( int = EOF );
    virtual int        underflow();
    virtual streambuf *setbuf( char *__ignored, int __allocation_size );
    virtual streampos  seekoff( streamoff     __offset,
                                ios::seekdir  __direction,
                                ios::openmode __mode );
    virtual int        sync();

protected:
    virtual int doallocate();

private:
    void  __strstreambuf( char *, int, char * );

    void *(*__alloc_fn)( long );
    void  (*__free_fn)( void * );
    int   __allocation_size;
    int   __minbuf_size;
    unsigned  __frozen    : 1;
    unsigned  __dynamic   : 1;
    unsigned  __unlimited : 1;
};
#if __WATCOMC__ > 1000
#pragma pack(__pop);
#else
#pragma pack();
#endif

inline strstreambuf::strstreambuf() {
    __strstreambuf( NULL, 0, NULL );
}

inline strstreambuf::strstreambuf( char *__ptr, int __size, char *__pstart ) {
    __strstreambuf( __ptr, __size, __pstart );
}

inline int strstreambuf::alloc_size_increment( int __increment ) {
    __lock_it( __b_lock );
    int __old_allocation_size = __allocation_size;
    __allocation_size        += __increment;
    return( __old_allocation_size );
}

inline void strstreambuf::freeze( int __freeze ) {
    __frozen = (char)(__freeze ? 1 : 0);
}

// **************************** STRSTREAMBASE ********************************
#if __WATCOMC__ > 1000
#pragma pack(__push,1);
#else
#pragma pack(1);
#endif
class strstreambase : public virtual ios {
public:
    strstreambuf *rdbuf() const;

protected:
    strstreambase();
    strstreambase( char *__str, int __size, char *__pstart = NULL );
    ~strstreambase();

private:
    strstreambuf __strstrmbuf;
};
#if __WATCOMC__ > 1000
#pragma pack(__pop);
#else
#pragma pack();
#endif

inline strstreambuf *strstreambase::rdbuf() const {
    return( (strstreambuf *) ios::rdbuf() );
}

// ***************************** ISTRSTREAM **********************************
#if __WATCOMC__ > 1000
#pragma pack(__push,1);
#else
#pragma pack(1);
#endif
class istrstream : public strstreambase, public istream {
public:
    istrstream(          char *__str );
    istrstream(   signed char *__str );
    istrstream( unsigned char *__str );
    istrstream(          char *__str, int __size );
    istrstream(   signed char *__str, int __size );
    istrstream( unsigned char *__str, int __size );
    ~istrstream();
};
#if __WATCOMC__ > 1000
#pragma pack(__pop);
#else
#pragma pack();
#endif

// ***************************** OSTRSTREAM **********************************
#if __WATCOMC__ > 1000
#pragma pack(__push,1);
#else
#pragma pack(1);
#endif
class ostrstream : public strstreambase, public ostream {
public:
    ostrstream();
    ostrstream( char          *__str,
                int            __size,
		ios::openmode  __mode = ios::out );
    ostrstream( signed char   *__str,
                int            __size,
		ios::openmode  __mode = ios::out );
    ostrstream( unsigned char *__str,
                int            __size,
		ios::openmode  __mode = ios::out );
    ~ostrstream();

    int   pcount() const;
    char *str();
};
#if __WATCOMC__ > 1000
#pragma pack(__pop);
#else
#pragma pack();
#endif

inline char *ostrstream::str() {
    __lock_it( __i_lock );
    return( rdbuf()->str() );
}

inline int ostrstream::pcount() const {
    __lock_it( __i_lock );
    return( rdbuf()->out_waiting() );
}

// ***************************** STRSTREAM ***********************************
#if __WATCOMC__ > 1000
#pragma pack(__push,1);
#else
#pragma pack(1);
#endif
class strstream : public strstreambase, public iostream {
public:
    strstream();
    strstream( char          *__str,
               int            __size,
	       ios::openmode  __mode = ios::in|ios::out );
    strstream( signed char   *__str,
               int            __size,
	       ios::openmode  __mode = ios::in|ios::out );
    strstream( unsigned char *__str,
               int            __size,
	       ios::openmode  __mode = ios::in|ios::out );
    ~strstream();

    char *str();
};
#if __WATCOMC__ > 1000
#pragma pack(__pop);
#else
#pragma pack();
#endif

inline char *strstream::str() {
    __lock_it( __i_lock );
    return( rdbuf()->str() );
}

#define _STRSTREAM_H_INCLUDED
#endif
