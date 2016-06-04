// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  sdlprefix.h - prefix file, included by ALL files
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

//============================================================
// System specific defines
//============================================================

/* Only problems ... */
#if defined(_WIN32)
#define SDLMAME_WIN32 1
#define _SDL_main_h
#endif


#ifdef __APPLE__
#define SDLMAME_DARWIN 1
#endif /* __APPLE__ */

#ifdef SDLMAME_UNIX

#if defined(__sun__) && defined(__svr4__)
#define SDLMAME_SOLARIS 1
#define NO_AFFINITY_NP 1
//#undef _XOPEN_SOURCE
//#undef _XOPEN_VERSION
//#undef _XOPEN_SOURCE_EXTENDED
//#undef _XPG6
//#undef _XPG5
//#undef _XPG4_2
//#define _XOPEN_SOURCE
//#define _XOPEN_VERSION 4
#elif defined(__irix__) || defined(__sgi)
#define SDLMAME_IRIX 1
/* Large file support on IRIX needs _SGI_SOURCE */
#undef _POSIX_SOURCE

#elif defined(__linux__) || defined(__FreeBSD_kernel__)
#define SDLMAME_LINUX 1

#elif defined(__FreeBSD__)
#define SDLMAME_FREEBSD 1
#define NO_AFFINITY_NP 1
#elif defined(__DragonFly__)
#define SDLMAME_DRAGONFLY 1
#elif defined(__OpenBSD__)
#define SDLMAME_OPENBSD 1
#elif defined(__NetBSD__)
#define SDLMAME_NETBSD 1
#endif

#if defined(__DragonFly__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#define SDLMAME_BSD 1
#endif

#if defined(__HAIKU__)
#define SDLMAME_HAIKU 1
#define SDLMAME_NO64BITIO 1
#endif

#if defined(EMSCRIPTEN)
#define SDLMAME_EMSCRIPTEN 1
#define SDLMAME_NO64BITIO 1
struct _IO_FILE {};  //_IO_FILE is an opaque type in the emscripten libc which makes clang cranky
#endif

#if defined(__ANDROID__)
#define SDLMAME_ANDROID 1
#endif

// fix for Ubuntu 8.10
#ifdef _FORTIFY_SOURCE
#undef _FORTIFY_SOURCE
#endif

#endif /* SDLMAME_UNIX */
