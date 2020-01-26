// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
 * nld_schmitt.h
 *
 */

#ifndef NLD_SCHMITT_H_
#define NLD_SCHMITT_H_

#include "netlist/nl_setup.h"

// ----------------------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------------------

#define SCHMITT_TRIGGER(name, model)                                           \
		NET_REGISTER_DEV(SCHMITT_TRIGGER, name)                                \
		NETDEV_PARAMI(name, MODEL, model)

#endif /* NLD_SCHMITT_H_ */
