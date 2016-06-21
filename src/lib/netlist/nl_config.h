// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nlconfig.h
 *
 */

#ifndef NLCONFIG_H_
#define NLCONFIG_H_

#include <cstdint>

#include "plib/pconfig.h"
#include "plib/pchrono.h"

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
 *  Since than, was removed from functions declared as virtual.
 *  This may explain that the recent benchmarks show no difference at all.
 *
 *  Disappointing is the GNUC_PMF performance.
 */

// This will be autodetected
//#define NL_PMF_TYPE 0

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



//============================================================
//  GENERAL
//============================================================

#define NL_USE_MEMPOOL				(0)
#define USE_TRUTHTABLE          	(1)

//============================================================
//  Solver defines
//============================================================

#define USE_MATRIX_GS 				(0)
#define USE_GABS 					(1)
// savings are eaten up by effort
// FIXME: Convert into solver parameter
#define USE_LINEAR_PREDICTION 		(0)
#define NETLIST_GMIN_DEFAULT     (1e-9)



//============================================================
//  DEBUGGING
//============================================================

#define NL_DEBUG                    (false)
#define NL_KEEP_STATISTICS          (0)

//============================================================
//  General Macros
//============================================================

#if defined(OPENMP)
#define HAS_OPENMP ( OPENMP >= 200805 )
#else
#define HAS_OPENMP (0)
#endif

//============================================================
//  Performance tracking
//============================================================

namespace netlist
{
#if NL_KEEP_STATISTICS
using nperftime_t = plib::timer<plib::exact_ticks, true>;
using nperfcount_t = plib::counter<true>;
#else
using nperftime_t = plib::chrono::timer<plib::chrono::exact_ticks, false>;
using nperfcount_t = plib::chrono::counter<false>;
#endif
}
//============================================================
//  General
//============================================================

// this macro passes an item followed by a string version of itself as two consecutive parameters
//#define NLNAME(x) x, #x

#define NOEXCEPT noexcept

// The following adds about 10% performance ...

#if !defined(USE_OPENMP)
#define USE_OPENMP              (0)
#endif // !defined(USE_OPENMP)

// Use nano-second resolution - Sufficient for now
#define NETLIST_INTERNAL_RES        (UINT64_C(1000000000))
//#define NETLIST_INTERNAL_RES      (UINT64_C(1000000000000))

#define NETLIST_CLOCK               (NETLIST_INTERNAL_RES)

//#define nl_double float
//#define NL_FCONST(x) (x ## f)

//#define nl_double double
#define NL_FCONST(x) x
using nl_double = double;


//============================================================
//  WARNINGS
//============================================================

#if (USE_OPENMP)
#if (!(HAS_OPENMP))
#error To use openmp compile and link with "-fopenmp"
#endif
#endif


#endif /* NLCONFIG_H_ */
