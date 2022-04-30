// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "emu.h"
#include "ymopz.h"


DEFINE_DEVICE_TYPE(YM2414, ym2414_device, "ym2414", "YM2414 OPZ")


//*********************************************************
//  YM2414 DEVICE
//*********************************************************

//-------------------------------------------------
//  ym2414_device - constructor
//-------------------------------------------------

ym2414_device::ym2414_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymfm_device_base<ymfm::ym2414>(mconfig, tag, owner, clock, YM2414)
{
}
