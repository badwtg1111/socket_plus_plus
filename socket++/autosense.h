#ifndef _autosense_h_
#define _autosense_h_

// ---------------------------------------------------------------------------
//  This section attempts to auto detect the operating system. It will set
//  up PAC specific defines that are used by the rest of the code.
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

#if defined(_AIX)
#define OS_AIX
#define OS_UNIX
#elif defined(_SEQUENT_)
#define OS_PTX
#define OS_UNIX
#elif defined(_HP_UX) || defined(__hpux) || defined(_HPUX_SOURCE)
#define OS_HPUX
#define OS_UNIX
#elif defined(SOLARIS) || defined(__SVR4)
#define OS_SOLARIS
#define OS_UNIX
#elif defined(__SCO_VERSION__) || defined(UnixWare)
#define OS_UNIXWARE
#define OS_UNIX
#elif defined(__linux__)
#define OS_LINUX
#define OS_UNIX
#elif defined(__FreeBSD__)
#define OS_BSD
#define OS_LINUX
#define OS_UNIX
#elif defined(__OpenBSD__)
#define OS_BSD
#define OS_LINUX
#define OS_UNIX
#elif defined(__ONetBSD__)
#define OS_BSD
#define OS_LINUX
#define OS_UNIX
#elif defined(IRIX) || defined(__sgi)
#define OS_IRIX
#define OS_UNIX
#elif defined(__MVS__)
#define OS_OS390
#define OS_UNIX
#elif defined(EXM_OS390)
#define OS_OS390
#define OS_UNIX
#elif defined(__OS400__)
#define OS_AS400
#define OS_UNIX
#elif defined(__OS2__)
#define OS_OS2
#elif defined(__TANDEM)
#define OS_TANDEM
#define OS_UNIX
#define OS_CSET
#elif defined(_WIN32) || defined(WIN32)
#define OS_WIN32
#ifndef WIN32
#define WIN32
#endif
#define endian
#undef  ENDIAN
#pragma warning(disable: 4267)
// C4267: conversion de 'size_t' en 'std::streamsize', perte possible de données
#elif defined(__WINDOWS__)
// IBM VisualAge special handling
#if defined(__32BIT__)
#define OS_WIN32
#else
#define OS_WIN16
#endif
#elif defined(__MSDOS__)
#define OS_DOS

#elif defined(macintosh)
#define OS_MACOS
#elif defined(__APPLE__) && defined(__MACH__)
#define OS_MACOSX
#elif defined(__alpha) && defined(__osf__)
#define OS_TRU64
#else
#error Code requires port to host OS!
#endif


// ---------------------------------------------------------------------------
//  This section attempts to autodetect the compiler being used. It will set
//  up PAC specific defines that can be used by the rest of the code.
// ---------------------------------------------------------------------------
#if defined(__BORLANDC__)
#define CPP_BORLAND
#elif defined(_MSC_VER)
#define CPP_VISUALCPP
#define CPP_HAS_PLACEMENT_DELETE	1
#elif defined(__xlC__)
#define CPP_CSET
#elif defined(CPP_SOLARIS)
#if defined(__SUNPRO_CC) & __SUNPRO_CC >=0x500
#define CPP_SUNCC5
#elif defined(__SUNPRO_CC) & __SUNPRO_CC <0x500
#define CPP_SUNCC
#elif defined(_EDG_RUNTIME_USES_NAMESPACES)
#define CPP_SOLARIS_KAICC
#elif defined(__GNUG__)
#define CPP_GCC
#else
#error "Which compiler???"
#endif
#elif defined(OS_UNIXWARE)
#if defined(__USLC__)
#define CPP_CC
#elif defined(__GNUC__)
#define CPP_GCC
#else
#error "Must defined a compiler"
#endif
#elif defined (__GNUG__) || defined(__linux__)
#define CPP_GCC
#elif defined(CPP_HPUX)
#if defined(EXM_HPUX)
#define CPP_HPUX_KAICC
#elif (__cplusplus == 1)
#define CPP_HPUX_CC
#elif (__cplusplus == 199707 || __cplusplus == 199711)
#define CPP_HPUX_aCC
#endif
#elif defined(CPP_IRIX)
#define CPP_MIPSPRO_CC
#elif defined(CPP_PTX)
#define CPP_PTX_CC
#elif defined(CPP_TANDEM)
#define CPP_TANDEMCC
#elif defined(__MVS__) && defined(__cplusplus)
#define CPP_MVSCPP
#elif defined(EXM_OS390) && defined(__cplusplus)
#define CPP_MVSCPP
#elif defined(__IBMC__) || defined(__IBMCPP__)
#if defined(CPP_WIN32)
#define CPP_IBMVAW32
#elif defined(CPP_OS2)
#define CPP_IBMVAOS2
#if (__IBMC__ >= 400 || __IBMCPP__ >= 400)
#define CPP_IBMVA4_OS2
#endif
#endif
#elif defined(CPP_TRU64) && defined(__DECCXX)
#define CPP_DECCXX
#elif defined(__MWERKS__)
#define CPP_METROWERKS
#elif defined(__OS400__)
#else
#error Code requires port to current development environment
#endif

#endif
