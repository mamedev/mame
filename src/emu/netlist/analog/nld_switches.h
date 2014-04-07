// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_switches.h
 *
 */

#pragma once

#ifndef NLD_SWITCHES_H_
#define NLD_SWITCHES_H_

#include "../nl_base.h"
#include "nld_twoterm.h"

// ----------------------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------------------

#define SWITCH2(_name)                                                              \
		NET_REGISTER_DEV(switch2, _name)

// ----------------------------------------------------------------------------------------
// Devices ...
// ----------------------------------------------------------------------------------------

NETLIB_DEVICE_WITH_PARAMS(switch2,
	NETLIB_NAME(R_base) m_R[2];

	netlist_param_int_t m_POS;
);




#endif /* NLD_SWITCHES_H_ */
