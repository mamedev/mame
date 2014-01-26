// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_legacy.h
 *
 * All of the devices below needs to disappear at some time .....
 *
 *
 */

#pragma once

#ifndef NLD_LEGACY_H_
#define NLD_LEGACY_H_

#include "../nl_base.h"

// ----------------------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------------------

#define NETDEV_RSFF(_name)                                                          \
		NET_REGISTER_DEV(nicRSFF, _name)

// ----------------------------------------------------------------------------------------
// Devices ...
// ----------------------------------------------------------------------------------------

NETLIB_DEVICE(nicRSFF,
	netlist_ttl_input_t m_S;
	netlist_ttl_input_t m_R;

	netlist_ttl_output_t m_Q;
	netlist_ttl_output_t m_QQ;
);


#endif /* NLD_LEGACY_H_ */
