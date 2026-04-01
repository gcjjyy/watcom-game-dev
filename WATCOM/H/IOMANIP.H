//
//  iomanip.h	I/O streams manipulators
//
//  Copyright by WATCOM International Corp. 1988-1996.  All rights reserved.
//
#ifndef _IOMANIP_H_INCLUDED

#ifndef __cplusplus
#error iomanip.h is for use with C++
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


#if __WATCOMC__ > 1000
#pragma pack(__push,1);
#else
#pragma pack(1);
#endif
template<class T>
    class smanip;
template<class T>
    class sapp {
    public:
	sapp( ios &(*__f)( ios &, T ) ) : __fn( __f ) {};
	smanip<T> operator()( T __p ) { return smanip<T>( __fn, __p ); };
    private:
	ios &(*__fn)( ios &, T );
    };
template<class T>
    class smanip {
    public:
	smanip( ios &(*__f)( ios &, T ), T __p ) : __fn( __f ), __parm( __p ) {};
	friend istream &operator>>( istream &, const smanip<T> & );
	friend ostream &operator<<( ostream &, const smanip<T> & );
    private:
	ios &(*__fn)( ios &, T );
	T __parm;
    };
template<class T>
    istream &operator>>( istream &__is, const smanip<T> &__sm ) {
	__sm.__fn( __is, __sm.__parm );
	return( __is );
    }
template<class T>
    ostream &operator<<( ostream &__os, const smanip<T> &__sm ) {
	__sm.__fn( __os, __sm.__parm );
	return( __os );
    }

template<class T>
    class imanip;
template<class T>
    class iapp {
    public:
	iapp( istream &(*__f)( istream &, T ) ) : __fn( __f ) {};
	imanip<T> operator()( T __p ) { return imanip<T>( __fn, __p ) };
    private:
	istream &(*__fn)( istream &, T );
    };
template<class T>
    class imanip {
    public:
	imanip( istream &(*__f)( istream &, T ), T __p ) : __fn( __f ), __parm( __p ) {};
	friend istream &operator>>( istream &, const imanip<T> & );
    private:
	istream &(*__fn)( istream &, T );
	T __parm;
    };
template<class T>
    istream &operator>>( istream &__is, const imanip<T> &__im ) {
	__im.__fn( __is, __im.__parm );
	return( __is );
    }

template<class T>
    class omanip;
template<class T>
    class oapp {
    public:
	oapp( ostream &(*__f)( ostream &, T ) ) : __fn( __f ) {} ;
	omanip<T> operator()( T __p ) { return omanip<T>( __fn, __p ); };
    private:
	ostream &(*__fn)( ostream &, T );
    };
template<class T>
    class omanip {
    public:
	omanip( ostream &(*__f)( ostream &, T ), T __p ) : __fn( __f ), __parm( __p ) {};
	friend ostream &operator<<( ostream &, const omanip<T> & );
    private:
	ostream &(*__fn)( ostream &, T );
	T __parm;
    };
template<class T>
    ostream &operator<<( ostream &__os, const omanip<T> &__om ) {
	__om.__fn( __os, __om.__parm );
	return( __os );
    }

template<class T>
    class iomanip;
template<class T>
    class ioapp {
    public:
	ioapp( iostream &(*__f)( iostream &, T ) ) : __fn( __f ) {};
	iomanip<T> operator()( T __p ) { return iomanip<T>( __fn, __p ) };
    private:
	iostream &(*__fn)( iostream &, T );
    };
template<class T>
    class iomanip {
    public:
	iomanip( iostream &(*__f)( iostream &, T ), T __p ) : __fn( __f ), __parm( __p ) {};
	friend iostream &operator>>( iostream &, const iomanip<T> & );
    private:
	iostream &(*__fn)( iostream &, T );
	T __parm;
    };
template<class T>
    iostream &operator>>( iostream &__is, const iomanip<T> &__im ) {
	__im.__fn( __is, __im.__parm );
	return( __is );
    }
    
#if __WATCOMC__ > 1000
#pragma pack(__pop);
#else
#pragma pack();
#endif

// applicator objects
extern sapp<long> resetiosflags;
extern sapp<int>  setbase;
extern sapp<int>  setfill;
extern sapp<long> setiosflags;
extern sapp<int>  setprecision;
extern sapp<int>  setw;

// define some compatibility macros for legacy code
#define SMANIP(__Typ)	smanip<__Typ>
#define SAPP(__Typ)	sapp<__Typ>
#define IMANIP(__Typ)	imanip<__Typ>
#define IAPP(__Typ)	iapp<__Typ>
#define OMANIP(__Typ)	omanip<__Typ>
#define OAPP(__Typ)	oapp<__Typ>
#define IOMANIP(__Typ)	iomanip<__Typ>
#define IOAPP(__Typ)	ioapp<__Typ>

#define SMANIP_define(__Typ)
#define IOMANIPdeclare(__Typ)

#define _IOMANIP_H_INCLUDED
#endif
