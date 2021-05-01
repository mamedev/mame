// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "emu.h"
#include "ym3526.h"


DEFINE_DEVICE_TYPE(YM3526, ym3526_device, "ym3526", "YM3526 OPL")


//*********************************************************
//  YM3526 DEVICE
//*********************************************************

//-------------------------------------------------
//  ym3526_device - constructor
//-------------------------------------------------

ym3526_device::ym3526_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymfm_device_base<ymfm::ym3526>(mconfig, tag, owner, clock, YM3526)
{
}
