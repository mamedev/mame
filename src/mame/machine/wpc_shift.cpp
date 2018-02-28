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

ADDRESS_MAP_START(wpc_shift_device::registers)
	AM_RANGE(0, 0) AM_READWRITE(adrh_r, adrh_w)
	AM_RANGE(1, 1) AM_READWRITE(adrl_r, adrl_w)
	AM_RANGE(2, 2) AM_READWRITE(val1_r, val1_w)
	AM_RANGE(3, 3) AM_READWRITE(val2_r, val2_w)
ADDRESS_MAP_END


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
