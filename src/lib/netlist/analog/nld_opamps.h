// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_opamps.h
 *
 */

//#pragma once

#ifndef NLD_OPAMPS_H_
#define NLD_OPAMPS_H_

#include "netlist/nl_setup.h"

// ----------------------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------------------

#define OPAMP(name, model)                                                     \
		NET_REGISTER_DEV(OPAMP, name)                                          \
		NETDEV_PARAMI(name, MODEL, model)


#endif /* NLD_OPAMPS_H_ */
