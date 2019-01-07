// license:GPL-2.0+
// copyright-holders:Couriersud
/*!
 *
 * \file nl_config.h
 *
 */

#ifndef NLCONFIG_H_
#define NLCONFIG_H_

#include "plib/pconfig.h"

//============================================================
//  SETUP
//============================================================


//============================================================
//  GENERAL
//============================================================

/*! Make use of a memory pool for performance related objects.
 *
 * Set to 1 to compile netlist with memory allocations from a
 * linear memory pool. This is based of the assumption that
 * due to enhanced locality there will be less cache misses.
 * Your mileage may vary.
 * This will cause crashes on OSX and thus is ignored on OSX.
 *
 */
#define USE_MEMPOOL                 (0)

/*
 * FIXME: Using truthtable is a lot slower than the explicit device
 *        in breakout. Performance drops by 20%. This can be fixed by
 *        setting param USE_DEACTIVATE for the device.
 */

#define USE_TRUTHTABLE_7448 (0)

// How many times do we try to resolve links (connections)
#define NL_MAX_LINK_RESOLVE_LOOPS   (100)

//============================================================
//  Solver defines
//============================================================

#define USE_GABS                    (1)
// savings are eaten up by effort
// FIXME: Convert into solver parameter
#define USE_LINEAR_PREDICTION       (0)
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
#elif defined(_OPENMP)
#define HAS_OPENMP ( _OPENMP >= 200805 )
#else
#define HAS_OPENMP (0)
#endif

//============================================================
//  General
//============================================================

/* The following adds about 10% to 20% performance for analog
 * netlists like kidniki.
 */

#if !defined(USE_OPENMP)
#define USE_OPENMP              (0)
#endif // !defined(USE_OPENMP)

// Use nano-second resolution - Sufficient for now
#define NETLIST_INTERNAL_RES        (UINT64_C(1000000000))
#define NETLIST_CLOCK               (NETLIST_INTERNAL_RES)
//#define NETLIST_INTERNAL_RES      (UINT64_C(1000000000000))
//#define NETLIST_CLOCK               (UINT64_C(1000000000))


//#define nl_double float
//#define NL_FCONST(x) (x ## f)

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
