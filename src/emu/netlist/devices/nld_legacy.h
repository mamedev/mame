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

#define NETDEV_RSFF(_name, _S, _R)                                                  \
		NET_REGISTER_DEV(nicRSFF, _name)                                            \
		NET_CONNECT(_name, S, _S)                                                   \
		NET_CONNECT(_name, R, _R)

#define NETDEV_SWITCH2(_name, _i1, _i2)                                             \
		NET_REGISTER_DEV(nicMultiSwitch, _name)                                     \
		NET_CONNECT(_name, i1, _i1)                                                 \
		NET_CONNECT(_name, i2, _i2)

// ----------------------------------------------------------------------------------------
// Devices ...
// ----------------------------------------------------------------------------------------

NETLIB_DEVICE(nicRSFF,
	netlist_ttl_input_t m_S;
	netlist_ttl_input_t m_R;

	netlist_ttl_output_t m_Q;
	netlist_ttl_output_t m_QQ;
);

NETLIB_DEVICE_WITH_PARAMS(nicMultiSwitch,
	netlist_analog_input_t m_I[8];

	netlist_analog_output_t m_Q;
	netlist_analog_output_t m_low;

	netlist_param_int_t m_POS;

	int m_position;
);




#endif /* NLD_LEGACY_H_ */
