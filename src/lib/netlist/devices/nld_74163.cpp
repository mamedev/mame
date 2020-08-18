// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74163.cpp
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

		NETLIB_DEVICE_IMPL(74163,     "TTL_74163",     "+CLK,+ENP,+ENT,+CLRQ,+LOADQ,+A,+B,+C,+D,@VCC,@GND")

	} //namespace devices
} // namespace netlist
