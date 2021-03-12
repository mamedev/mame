// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * nld_74161.cpp
 *
 */

#include "nl_base.h"
#include "nl_factory.h"

#include "nld_9316_base.hxx"

// FIXME: All detail can be found in nld_9316_base.hxx
//        At least gcc-7.x needs the implementations of the base device
//        in different compilation units. If created in the same file
//        performance degrades horrible.

// FIXME: this file could be created programmatically

namespace netlist::devices {

	NETLIB_DEVICE_IMPL(74161,     "TTL_74161",     "+CLK,+ENP,+ENT,+CLRQ,+LOADQ,+A,+B,+C,+D,@VCC,@GND")
	// FIXME: This happens on copy/paste
	NETLIB_DEVICE_IMPL(74161_fixme, "TTL_74161_FIXME", "+A,+B,+C,+D,+CLRQ,+LOADQ,+CLK,+ENP,+ENT,@VCC,@GND")

} // namespace netlist::devices
