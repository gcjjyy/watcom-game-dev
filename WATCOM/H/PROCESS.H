/*
 *  process.h	Process spawning and related routines
 *
 *  Copyright by WATCOM International Corp. 1988-1996.  All rights reserved.
 */
#ifndef _PROCESS_H_INCLUDED
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


/*
 *  POSIX 1003.1 Prototypes
 */
extern int execl( const char *__path, const char *__arg0, ... );
extern int execle( const char *__path, const char *__arg0, ... );
extern int execlp( const char *__file, const char *__arg0, ... );
extern int execv( const char *__path, const char *const __argv[] );
extern int execve( const char *__path, const char *const __argv[], const char *const __envp[] );
extern int execvp( const char *__file, const char *const __argv[] );


/* mode flags for spawnxxx routines */
extern int __p_overlay;

#define P_WAIT	    0
#define P_NOWAIT    1
#define P_OVERLAY   __p_overlay
#define P_NOWAITO   3

/*
 *  Prototypes for non-POSIX functions
 */

extern int execlpe( const char *__file, const char *__arg0, ... );
extern int execvpe( const char *__file, const char *const __argv[], const char *const __envp[] );

#ifdef __386__
extern int _beginthread( register void (*__start_address)(void *),
     void *__stack_bottom, unsigned __stack_size, void *__arglist );
extern void _endthread();
#ifdef __NT__
extern unsigned long _beginthreadex( void *__security, unsigned __stack_size,
    register unsigned (__stdcall *__start_address)(void *),
    void *__arglist, unsigned __initflag, unsigned *__thrdaddr );
extern void _endthreadex( unsigned __retval );
#endif
#else
extern int __far _beginthread( register void (__far *__start_address)(void __far *),
     void __far *__stack_bottom, unsigned __stack_size,void __far *__arglist );
extern void __far _endthread();
#ifdef __SW_ZU
  #define _beginthread( __sa, __sb, __ss, __al ) _beginthread( __sa, __sb, __ss, __al )
#else
  void __far *__chkstack( void __far * );
  #define _beginthread( __sa, __sb, __ss, __al ) _beginthread( __sa, __chkstack(__sb), __ss, __al )
#endif
#endif

#if defined(__OS2__) || defined(__NT__)
/* values for __action_code used with cwait() */

#define WAIT_CHILD 0
#define WAIT_GRANDCHILD 1

extern int cwait(int *__status,int __process_id,int __action_code);
#endif
extern void abort(void);
extern void exit(int __status);
extern void _exit(int __status);
extern char *getcmd(char *__buffer);
extern int _bgetcmd(char *__buffer, int __len);
extern char *_cmdname(char *__name);
extern char *getenv(const char *__name);
extern int putenv(const char *__string);
extern int spawnl(int __mode,const char *__path, const char *__arg0,...);
extern int spawnle(int __mode,const char *__path, const char *__arg0,...);
extern int spawnlp(int __mode,const char *__path, const char *__arg0,...);
extern int spawnlpe(int __mode,const char *__path, const char *__arg0,...);
extern int spawnv(int __mode,const char *__path,const char * const *__argv);
extern int spawnve(int __mode,const char *__path,const char * const *__argv,const char * const *__envp);
extern int spawnvp(int __mode,const char *__path,const char * const *__argv);
extern int spawnvpe(int __mode,const char *__path,const char * const *__argv,const char * const *__envp);
extern int system(const char *__cmd);

#if defined(__NT__)
 // For use when access to the DLL startup is required.
 // this function pointer will be called after the C library startup
 // and before the C++ library startup.
 #if defined(_WINDOWS_)
  extern BOOL (WINAPI *_pRawDllMain)(HANDLE, DWORD, LPVOID);
 #else
  extern int (__stdcall *_pRawDllMain)(void *, unsigned, void *);
 #endif
#endif
extern int getpid(void);
extern int wait(int *__status);


#define _PROCESS_H_INCLUDED
#ifdef __cplusplus
};
#endif
#endif
