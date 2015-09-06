// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_switches.h
 *
 */

#pragma once

#ifndef NLD_SWITCHES_H_
#define NLD_SWITCHES_H_

#include "nl_base.h"
#include "nld_twoterm.h"

// ----------------------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------------------

#define SWITCH(_name)                                                              \
		NET_REGISTER_DEV(SWITCH, _name)

#define SWITCH2(_name)                                                              \
		NET_REGISTER_DEV(SWITCH2, _name)

// ----------------------------------------------------------------------------------------
// Devices ...
// ----------------------------------------------------------------------------------------

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_DEVICE_WITH_PARAMS(switch1,
	NETLIB_NAME(R_base) m_R;

	param_int_t m_POS;
);

NETLIB_DEVICE_WITH_PARAMS(switch2,
	NETLIB_NAME(R_base) m_R[2];

	param_int_t m_POS;
);

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_SWITCHES_H_ */
