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

#ifndef NL_AUTO_DEVICES

#define SWITCH(name)                                                              \
		NET_REGISTER_DEV(SWITCH, name)

#define SWITCH2(name)                                                              \
		NET_REGISTER_DEV(SWITCH2, name)

#endif

#endif /* NLD_SWITCHES_H_ */
