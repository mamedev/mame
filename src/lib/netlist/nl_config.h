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
#ifndef USE_MEMPOOL
#define USE_MEMPOOL                 (1)
#endif

/*! Enable queue statistics.
 *
 * Queue statistics come at a performance cost. Although
 * the cost is low, we disable them here since they are
 * only needed during development.
 *
 */
#ifndef USE_QUEUE_STATS
#define USE_QUEUE_STATS             (0)
#endif

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
#ifndef USE_COPY_INSTEAD_OF_REFERENCE
#define USE_COPY_INSTEAD_OF_REFERENCE (0)
#endif

/*
 * FIXME: Using truthtable is a lot slower than the explicit device
 *        in breakout. Performance drops by 20%. This can be fixed by
 *        setting param USE_DEACTIVATE for the device.
 */

#define USE_TRUTHTABLE_7448 (0)

/*
 * FIXME: The truthtable implementation of 74107 (JK-Flipflop)
 *        is included for educational purposes to demonstrate how
 *        to implement state holding devices as truthtables.
 *        It will completely nuke performance for pong.
 */

#define USE_TRUTHTABLE_74107 (0)

//============================================================
//  DEBUGGING
//============================================================

#ifndef NL_DEBUG
#define NL_DEBUG                    (false)
//#define NL_DEBUG                    (true)
#endif

//============================================================
// Time resolution
//============================================================

// Use nano-second resolution - Sufficient for now

static constexpr const auto NETLIST_INTERNAL_RES = 1000000000;
//static constexpr const auto NETLIST_INTERNAL_RES = 1000000000000;
static constexpr const auto NETLIST_CLOCK = NETLIST_INTERNAL_RES;

//#define NETLIST_INTERNAL_RES        (UINT64_C(1000000000))
//#define NETLIST_CLOCK               (NETLIST_INTERNAL_RES)
//#define NETLIST_INTERNAL_RES      (UINT64_C(1000000000000))
//#define NETLIST_CLOCK               (UINT64_C(1000000000))


//#define nl_double float
using nl_double = double;

#endif /* NLCONFIG_H_ */
