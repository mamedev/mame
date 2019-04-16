// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_mosfet.h
 *
 */

#ifndef NLD_MOSFET_H_
#define NLD_MOSFET_H_

#include "netlist/nl_setup.h"

// -----------------------------------------------------------------------------
// Macros
// -----------------------------------------------------------------------------

#define MOSFET(name, model)                                                   \
		NET_REGISTER_DEV(MOSFET, name)                                        \
		NETDEV_PARAMI(name,  MODEL, model)

#endif /* NLD_MOSFET_H_ */
