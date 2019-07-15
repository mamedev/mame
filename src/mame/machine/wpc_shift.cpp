// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "wpc_shift.h"

DEFINE_DEVICE_TYPE(WPC_SHIFT, wpc_shift_device, "wpc_shift", "Windows Pinball Controller Shifter")

wpc_shift_device::wpc_shift_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, WPC_SHIFT, tag, owner, clock)
{
}

wpc_shift_device::~wpc_shift_device()
{
}

void wpc_shift_device::registers(address_map &map)
{
	map(0, 0).rw(FUNC(wpc_shift_device::adrh_r), FUNC(wpc_shift_device::adrh_w));
	map(1, 1).rw(FUNC(wpc_shift_device::adrl_r), FUNC(wpc_shift_device::adrl_w));
	map(2, 2).rw(FUNC(wpc_shift_device::val1_r), FUNC(wpc_shift_device::val1_w));
	map(3, 3).rw(FUNC(wpc_shift_device::val2_r), FUNC(wpc_shift_device::val2_w));
}


READ8_MEMBER(wpc_shift_device::adrh_r)
{
	return (adr + (val1 >> 3)) >> 8;
}

WRITE8_MEMBER(wpc_shift_device::adrh_w)
{
	adr = (adr & 0xff) | (data << 8);
}

READ8_MEMBER(wpc_shift_device::adrl_r)
{
	return adr + (val1 >> 3);
}

WRITE8_MEMBER(wpc_shift_device::adrl_w)
{
	adr = (adr & 0xff00) | data;
}

READ8_MEMBER(wpc_shift_device::val1_r)
{
	return 1 << (val1 & 7);
}

WRITE8_MEMBER(wpc_shift_device::val1_w)
{
	val1 = data;
}

READ8_MEMBER(wpc_shift_device::val2_r)
{
	return 1 << (val2 & 7);
}

WRITE8_MEMBER(wpc_shift_device::val2_w)
{
	val2 = data;
}

void wpc_shift_device::device_start()
{
	save_item(NAME(adr));
	save_item(NAME(val1));
	save_item(NAME(val2));
}

void wpc_shift_device::device_reset()
{
	adr = 0x0000;
	val1 = 0x00;
	val2 = 0x00;
}
