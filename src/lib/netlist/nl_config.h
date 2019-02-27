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
 *
 */
#define USE_MEMPOOL                 (1)

/*! Store input values in logic_terminal_t.
 *
 * Set to 1 to store values in logic_terminal_t instead of
 * accessing them indirectly by pointer from logic_net_t.
 * This approach is stricter and should identify bugs in
 * the netlist core faster.
 * By default it is disabled since it is not as fast as
 * the default approach. It is up to 5% slower.
 *
 */
#define USE_COPY_INSTEAD_OF_REFERENCE (0)

/*
 * FIXME: Using truthtable is a lot slower than the explicit device
 *        in breakout. Performance drops by 20%. This can be fixed by
 *        setting param USE_DEACTIVATE for the device.
 */

#define USE_TRUTHTABLE_7448 (0)

// How many times do we try to resolve links (connections)
#define NL_MAX_LINK_RESOLVE_LOOPS   (100)

//============================================================
//  DEBUGGING
//============================================================

#define NL_DEBUG                    (false)
#define NL_KEEP_STATISTICS          (0)
//#define NL_DEBUG                    (true)
//#define NL_KEEP_STATISTICS          (1)

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

static constexpr const auto NETLIST_INTERNAL_RES = 1000000000;
static constexpr const auto NETLIST_CLOCK = NETLIST_INTERNAL_RES;

//#define NETLIST_INTERNAL_RES        (UINT64_C(1000000000))
//#define NETLIST_CLOCK               (NETLIST_INTERNAL_RES)
//#define NETLIST_INTERNAL_RES      (UINT64_C(1000000000000))
//#define NETLIST_CLOCK               (UINT64_C(1000000000))


//#define nl_double float
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
