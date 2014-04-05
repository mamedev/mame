// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"

const device_type DEVCB2_LINE_DISPATCH_2 = &device_creator<devcb2_line_dispatch_device<2> >;
const device_type DEVCB2_LINE_DISPATCH_3 = &device_creator<devcb2_line_dispatch_device<3> >;
const device_type DEVCB2_LINE_DISPATCH_4 = &device_creator<devcb2_line_dispatch_device<4> >;
const device_type DEVCB2_LINE_DISPATCH_5 = &device_creator<devcb2_line_dispatch_device<5> >;
const device_type DEVCB2_LINE_DISPATCH_6 = &device_creator<devcb2_line_dispatch_device<6> >;

template<> devcb2_line_dispatch_device<2>::devcb2_line_dispatch_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, DEVCB2_LINE_DISPATCH_2, "Line dispatcher (2 slots)", tag, owner, clock, "devcb2_line_dispatch", __FILE__)
{
	init_fwd();
}

template<> devcb2_line_dispatch_device<3>::devcb2_line_dispatch_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, DEVCB2_LINE_DISPATCH_3, "Line dispatcher (3 slots)", tag, owner, clock, "devcb2_line_dispatch", __FILE__)
{
	init_fwd();
}

template<> devcb2_line_dispatch_device<4>::devcb2_line_dispatch_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, DEVCB2_LINE_DISPATCH_4, "Line dispatcher (4 slots)", tag, owner, clock, "devcb2_line_dispatch", __FILE__)
{
	init_fwd();
}

template<> devcb2_line_dispatch_device<5>::devcb2_line_dispatch_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, DEVCB2_LINE_DISPATCH_5, "Line dispatcher (5 slots)", tag, owner, clock, "devcb2_line_dispatch", __FILE__)
{
	init_fwd();
}

template<> devcb2_line_dispatch_device<6>::devcb2_line_dispatch_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, DEVCB2_LINE_DISPATCH_6, "Line dispatcher (6 slots)", tag, owner, clock, "devcb2_line_dispatch", __FILE__)
{
	init_fwd();
}
