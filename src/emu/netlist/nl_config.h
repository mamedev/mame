// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nlconfig.h
 *
 */

#ifndef NLCONFIG_H_
#define NLCONFIG_H_

/* FIXME: at some time, make it compile on it's own */

#include "emu.h"

//============================================================
//  SETUP
//============================================================

/*
 * The next options needs -Wno-pmf-conversions to compile and gcc
 * This is intended for non-mame usage.
 *
 */
#define USE_PMFDELEGATES        (0)

// This increases performance in circuits with a lot of gates
// but is not guaranteed to be absolutely timing correct.

#define USE_DEACTIVE_DEVICE     (0)

// The following adds about 10% performance ...

#define USE_ADD_REMOVE_LIST     (1)

#define USE_OPENMP              (0)

// Use nano-second resolution - Sufficient for now
#define NETLIST_INTERNAL_RES        (U64(1000000000))
//#define NETLIST_INTERNAL_RES      (U64(1000000000000))

#define NETLIST_CLOCK               (NETLIST_INTERNAL_RES)

#define NETLIST_GMIN_DEFAULT    (1e-9)

typedef UINT8 netlist_sig_t;

//============================================================
//  DEBUGGING
//============================================================

//#define fatalerror xxbreakme

#define NL_VERBOSE                  (0)
#define NL_KEEP_STATISTICS          (0)
#define FATAL_ERROR_AFTER_NS        (0) //(1000)

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
#define begin_timing(v)         do { (v) -= get_profile_ticks(); } while (0)
#define end_timing(v)           do { (v) += get_profile_ticks(); } while (0)
#else
#define add_to_stat(v,x)        do { } while (0)
#define inc_stat(v)             add_to_stat(v, 1)
#define begin_timing(v)         do { } while (0)
#define end_timing(v)           do { } while (0)
#endif


//============================================================
//  Performance tracking
//============================================================

// Compiling without mame ?

#ifndef ATTR_HOT
#warning ATTR_HOT not defined
#define ATTR_HOT
#endif

#ifndef ATTR_COLD
#define ATTR_COLD
#endif

//============================================================
//  WARNINGS
//============================================================

#if (USE_OPENMP)
#if (!(HAS_OPENMP))
#warning To use openmp compile and link with "-fopenmp"
#endif
#endif

#endif /* NLCONFIG_H_ */
