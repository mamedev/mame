// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "emu.h"
#include "ym2612.h"


DEFINE_DEVICE_TYPE(YM2612, ym2612_device, "ym2612", "YM2612 OPN2")
DEFINE_DEVICE_TYPE(YM3438, ym3438_device, "ym3438", "YM3438 OPN2C")
DEFINE_DEVICE_TYPE(YMF276, ymf276_device, "ymf276", "YMF276 OPN2L")


//*********************************************************
//  YM2612 DEVICE
//*********************************************************

//-------------------------------------------------
//  ym2612_device - constructor
//-------------------------------------------------

ym2612_device::ym2612_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymfm_device_base<ymfm::ym2612>(mconfig, tag, owner, clock, YM2612)
{
}



//*********************************************************
//  YM3438 DEVICE
//*********************************************************

//-------------------------------------------------
//  ym3438_device - constructor
//-------------------------------------------------

ym3438_device::ym3438_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymfm_device_base<ymfm::ym3438>(mconfig, tag, owner, clock, YM3438)
{
}



//*********************************************************
//  YMF276 DEVICE
//*********************************************************

//-------------------------------------------------
//  ymf276_device - constructor
//-------------------------------------------------

ymf276_device::ymf276_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymfm_device_base<ymfm::ymf276>(mconfig, tag, owner, clock, YMF276)
{
}

