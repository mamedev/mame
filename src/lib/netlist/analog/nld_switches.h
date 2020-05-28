// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_SWITCHES_H_
#define NLD_SWITCHES_H_

///
/// \file nld_switches.h
///

#include "netlist/nl_setup.h"

// ----------------------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------------------

#define SWITCH(name)                                                              \
		NET_REGISTER_DEV(SWITCH, name)

#define SWITCH2(name)                                                              \
		NET_REGISTER_DEV(SWITCH2, name)

#endif // NLD_SWITCHES_H_
