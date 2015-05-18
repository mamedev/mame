// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nlconfig.h
 *
 */

#ifndef NLCONFIG_H_
#define NLCONFIG_H_

/* FIXME: at some time, make it compile on its own */

#include "osdcore.h"
#include "corealloc.h"
#include "eminline.h"
#include <math.h>
#include <exception>
#include <typeinfo>

//============================================================
//  SETUP
//============================================================

/*
 * The next options needs -Wno-pmf-conversions to compile and gcc
 * There is quite some significant speed-up of up to 20% involved.
 * NO_USE_PMFCONVERSION is for illustrative purposes only. Using PMFs
 * has some overhead in comparison to calling a virtual function.
 *
 * To get a performance increase we need the GCC extension.
 *
 */

#ifndef USE_PMFDELEGATES
#if defined(__clang__)
	#define USE_PMFDELEGATES        	(0)
	#define NO_USE_PMFCONVERSION		(1)
#else
	#if defined(__GNUC__)
		#define USE_PMFDELEGATES        (1)
		#define NO_USE_PMFCONVERSION	(0)
		#pragma GCC diagnostic ignored "-Wpmf-conversions"
	#else
		#define USE_PMFDELEGATES        (0)
		#define NO_USE_PMFCONVERSION	(1)
	#endif
#endif
#endif

/*
 *  This increases performance in circuits with a lot of gates
 *  but is not guaranteed to be absolutely timing correct.
 *
 *  Performance increase about 10% (breakout) to 20% (pong)
 *
 */


// moved to parameter NETLIST.USE_DEACTIVATE
// #define USE_DEACTIVE_DEVICE     (0)

#define USE_TRUTHTABLE          (1)

// The following adds about 10% performance ...

#if !defined(USE_OPENMP)
#define USE_OPENMP              (0)
#endif // !defined(USE_OPENMP)

// Use nano-second resolution - Sufficient for now
#define NETLIST_INTERNAL_RES        (U64(1000000000))
//#define NETLIST_INTERNAL_RES      (U64(1000000000000))

#define NETLIST_CLOCK               (NETLIST_INTERNAL_RES)

#define NETLIST_GMIN_DEFAULT    (1e-9)


//#define nl_double float
//#define NL_FCONST(x) (x ## f)

#define nl_double double
#define NL_FCONST(x) x


//============================================================
//  Solver defines
//============================================================

#define USE_MATRIX_GS (0)
#define USE_PIVOT_SEARCH (0)
#define USE_GABS (1)
// savings are eaten up by effort
// FIXME: Convert into solver parameter
#define USE_LINEAR_PREDICTION (0)


//============================================================
//  DEBUGGING
//============================================================

//#define fatalerror xxbreakme

#define NL_VERBOSE                  (0)
#define NL_KEEP_STATISTICS          (0)

#if (NL_VERBOSE)
	#define NL_VERBOSE_OUT(x)       printf x
#else
	#define NL_VERBOSE_OUT(x)       do { } while (0)
#endif

//============================================================
//  General Macros
//============================================================

#if defined(_OPENMP)
#define HAS_OPENMP ( _OPENMP >= 200805 )
#else
#define HAS_OPENMP (0)
#endif

// prevent implicit copying
#define NETLIST_PREVENT_COPYING(_name)          \
	private:                                    \
		_name(const _name &);                   \
		_name &operator=(const _name &);

#if defined(__GNUC__) && (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3))
#if !defined(__ppc__) && !defined (__PPC__) && !defined(__ppc64__) && !defined(__PPC64__)
#define ATTR_ALIGN __attribute__ ((aligned(64)))
#else
#define ATTR_ALIGN
#endif
#else
#define ATTR_ALIGN
#endif

//============================================================
//  Performance tracking
//============================================================

#if NL_KEEP_STATISTICS
#define add_to_stat(v,x)        do { v += (x); } while (0)
#define inc_stat(v)             add_to_stat(v, 1)
#define begin_timing(v)         do { v -= get_profile_ticks(); } while (0)
#define end_timing(v)           do { v += get_profile_ticks(); } while (0)
#else
#define add_to_stat(v,x)        do { } while (0)
#define inc_stat(v)             add_to_stat(v, 1)
#define begin_timing(v)         do { } while (0)
#define end_timing(v)           do { } while (0)
#endif

// this macro passes an item followed by a string version of itself as two consecutive parameters
#define NLNAME(x) x, #x

//============================================================
//  Exceptions
//============================================================

// emu_fatalerror is a generic fatal exception that provides an error string
class nl_fatalerror : public std::exception
{
public:
	nl_fatalerror(const char *format, ...) ATTR_PRINTF(2,3)
	{
		char text[1024];
		va_list ap;
		va_start(ap, format);
		vsprintf(text, format, ap);
		va_end(ap);
		osd_printf_error("%s\n", text);
	}
	nl_fatalerror(const char *format, va_list ap)
	{
		char text[1024];
		vsprintf(text, format, ap);
		osd_printf_error("%s\n", text);
	}
};

//============================================================
//  Memory allocation
//============================================================

#define nl_alloc(T, ...)        global_alloc(T(__VA_ARGS__))
#define nl_alloc_array(T, N)    global_alloc_array(T, N)

#define nl_free(_ptr)           global_free(_ptr)
#define nl_free_array(_ptr)     global_free_array(_ptr)


//============================================================
//  Asserts
//============================================================

#ifdef MAME_DEBUG
#define nl_assert(x)               do { if (!(x)) throw nl_fatalerror("assert: %s:%d: %s", __FILE__, __LINE__, #x); } while (0)
#else
#define nl_assert(x)               do { if (0) if (!(x)) throw nl_fatalerror("assert: %s:%d: %s", __FILE__, __LINE__, #x); } while (0)
//#define assert_always(x, msg)   do { if (!(x)) throw emu_fatalerror("Fatal error: %s (%s:%d)", msg, __FILE__, __LINE__); } while (0)
#endif
#define nl_assert_always(x, msg)    do { if (!(x)) throw nl_fatalerror("Fatal error: %s\nCaused by assert: %s:%d: %s", msg, __FILE__, __LINE__, #x); } while (0)

//============================================================
//  Compiling standalone
//============================================================

// Compiling without mame ?

#ifndef ATTR_HOT
#warning ATTR_HOT not defined

// standard C includes
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

// standard C++ includes
#include <exception>
#include <typeinfo>
#include <new>

#define ATTR_HOT
#define ATTR_COLD
#define ATTR_PRINTF(n1,n2)
#define RESTRICT
#define EXPECTED
#define UNEXPECTED
#define ATTR_UNUSED             __attribute__((__unused__))

/* 8-bit values */
typedef unsigned char                       UINT8;
typedef signed char                         INT8;

/* 16-bit values */
typedef unsigned short                      UINT16;
typedef signed short                        INT16;

/* 32-bit values */
#ifndef _WINDOWS_H
typedef unsigned int                        UINT32;
typedef signed int                          INT32;
#endif

/* 64-bit values */
#ifndef _WINDOWS_H
#ifdef _MSC_VER
typedef signed __int64                      INT64;
typedef unsigned __int64                    UINT64;
#else
__extension__ typedef unsigned long long    UINT64;
__extension__ typedef signed long long      INT64;
#endif
#endif

/* U64 and S64 are used to wrap long integer constants. */
#if defined(__GNUC__) || defined(_MSC_VER)
#define U64(val) val##ULL
#define S64(val) val##LL
#else
#define U64(val) val
#define S64(val) val
#endif

/* Standard MIN/MAX macros */
#ifndef MIN
#define MIN(x,y)            ((x) < (y) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x,y)            ((x) > (y) ? (x) : (y))
#endif


#endif

//============================================================
//  WARNINGS
//============================================================

#if (USE_OPENMP)
#if (!(HAS_OPENMP))
#error To use openmp compile and link with "-fopenmp"
#endif
#endif


#endif /* NLCONFIG_H_ */
