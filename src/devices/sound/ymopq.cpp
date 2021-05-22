// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "emu.h"
#include "ymopq.h"


DEFINE_DEVICE_TYPE(YM3806, ym3806_device, "ym3806", "YM3806 OPN")


//*********************************************************
//  YM3806 DEVICE
//*********************************************************

//-------------------------------------------------
//  ym3806_device - constructor
//-------------------------------------------------

ym3806_device::ym3806_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymfm_device_base<ymfm::ym3806>(mconfig, tag, owner, clock, YM3806)
{
}
