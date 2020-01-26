// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_legacy.h
 *
 * All of the devices below needs to disappear at some time .....
 *
 *
 */

#ifndef NLD_LEGACY_H_
#define NLD_LEGACY_H_

#include "netlist/nl_setup.h"

// ----------------------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------------------

#define NETDEV_RSFF(name)                                                       \
		NET_REGISTER_DEV(NETDEV_RSFF, name)

#define NETDEV_DELAY(name)                                                      \
		NET_REGISTER_DEV(NETDEV_DELAY, name)

// ----------------------------------------------------------------------------------------
// Devices ...
// ----------------------------------------------------------------------------------------

namespace netlist
{
	namespace devices
	{
	} //namespace devices
} // namespace netlist

#endif /* NLD_LEGACY_H_ */
