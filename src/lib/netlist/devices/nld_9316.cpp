// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_9316.cpp
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

namespace netlist
{
	namespace devices
	{

		NETLIB_DEVICE_IMPL(9316,     "TTL_9316",     "+CLK,+ENP,+ENT,+CLRQ,+LOADQ,+A,+B,+C,+D,@VCC,@GND")
		NETLIB_DEVICE_IMPL(9316_dip, "TTL_9316_DIP", "")

	} //namespace devices
} // namespace netlist
