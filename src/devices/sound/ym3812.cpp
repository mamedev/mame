// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "emu.h"
#include "ym3812.h"


DEFINE_DEVICE_TYPE(YM3812, ym3812_device, "ym3812", "YM3812 OPL2")


//*********************************************************
//  YM3812 DEVICE
//*********************************************************

//-------------------------------------------------
//  ym3812_device - constructor
//-------------------------------------------------

ym3812_device::ym3812_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymfm_device_base<ymfm::ym3812>(mconfig, tag, owner, clock, YM3812)
{
}
