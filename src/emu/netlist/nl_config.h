// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nlconfig.h
 *
 */

#ifndef NLCONFIG_H_
#define NLCONFIG_H_

#include "pconfig.h"

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
 * Todo: This doesn't work with current (4.8+) mingw 32bit builds.
 *       Therefore disabled for now for i386 builds.
 *
 */

#ifndef USE_PMFDELEGATES
#if defined(__clang__) || (defined(__GNUC__) && defined(__i386__))
	#define USE_PMFDELEGATES            (0)
	#define NO_USE_PMFCONVERSION        (1)
#else
	#if defined(__GNUC__)
		#define USE_PMFDELEGATES        (1)
		#define NO_USE_PMFCONVERSION    (0)
		#pragma GCC diagnostic ignored "-Wpmf-conversions"
	#else
		#define USE_PMFDELEGATES        (0)
		#define NO_USE_PMFCONVERSION    (1)
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
//  WARNINGS
//============================================================

#if (USE_OPENMP)
#if (!(HAS_OPENMP))
#error To use openmp compile and link with "-fopenmp"
#endif
#endif


#endif /* NLCONFIG_H_ */
