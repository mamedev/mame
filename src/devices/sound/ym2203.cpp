// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "emu.h"
#include "ym2203.h"


DEFINE_DEVICE_TYPE(YM2203, ym2203_device, "ym2203", "YM2203 OPN")


//*********************************************************
//  YM2203 DEVICE
//*********************************************************

//-------------------------------------------------
//  ym2203_device - constructor
//-------------------------------------------------

ym2203_device::ym2203_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymfm_ssg_device_base<ymfm::ym2203>(mconfig, tag, owner, clock, YM2203)
{
}
