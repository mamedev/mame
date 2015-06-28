// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nlconfig.h
 *
 */

#ifndef NLCONFIG_H_
#define NLCONFIG_H_

#include "plib/pconfig.h"

//============================================================
//  SETUP
//============================================================

/*
 * The following options determine how object::update is called.
 * NL_PMF_TYPE_VIRTUAL
 *      Use stock virtual call
 *
 * NL_PMF_TYPE_GNUC_PMF
 *      Use standard pointer to member function syntax
 *
 *  NL_PMF_TYPE_GNUC_PMF_CONV
 *      Use gnu extension and convert the pmf to a function pointer.
 *      This is not standard compliant and needs
 *      -Wno-pmf-conversions to compile.
 *
 *  NL_PMF_TYPE_INTERNAL
 *      Use the same approach as MAME for deriving the function pointer.
 *      This is compiler-dependant as well
 *
 *  Benchmarks for ./nltool -c run -f src/mame/drivers/nl_pong.c -t 10 -n pong_fast
 *
 *  NL_PMF_TYPE_INTERNAL:       215%
 *  NL_PMF_TYPE_GNUC_PMF:       163%
 *  NL_PMF_TYPE_GNUC_PMF_CONV:  215%
 *  NL_PMF_TYPE_VIRTUAL:        213%
 *
 *  The whole exercise was done to avoid virtual calls. In prior versions of
 *  netlist, the INTERNAL and GNUC_PMF_CONV approach provided significant improvement.
 *  Since than, ATTR_COLD was removed from functions declared as virtual.
 *  This may explain that the recent benchmarks show no difference at all.
 *
 *  Disappointing is the GNUC_PMF performance.
 */

// This will be autodetected
//#define NL_PMF_TYPE 3

#define NL_PMF_TYPE_VIRTUAL         0
#define NL_PMF_TYPE_GNUC_PMF        1
#define NL_PMF_TYPE_GNUC_PMF_CONV   2
#define NL_PMF_TYPE_INTERNAL        3

#ifndef NL_PMF_TYPE
	#if PHAS_PMF_INTERNAL
		#define NL_PMF_TYPE NL_PMF_TYPE_INTERNAL
	#else
		#define NL_PMF_TYPE NL_PMF_TYPE_VIRTUAL
	#endif
#endif

#if (NL_PMF_TYPE == NL_PMF_TYPE_GNUC_PMF_CONV)
#pragma GCC diagnostic ignored "-Wpmf-conversions"
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
	#define NL_VERBOSE_OUT(x)       netlist().log x
#else
	#define NL_VERBOSE_OUT(x)       do { if(0) netlist().log x ; } while (0)
#endif

//============================================================
//  General Macros
//============================================================

#if defined(_OPENMP)
#define HAS_OPENMP ( _OPENMP >= 200805 )
#else
#define HAS_OPENMP (0)
#endif

//============================================================
//  Performance tracking
//============================================================

#if NL_KEEP_STATISTICS
#include "eminline.h"
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
