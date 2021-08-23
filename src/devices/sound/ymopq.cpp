// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "emu.h"
#include "ymopq.h"


//*********************************************************
//  YM3806 DEVICE
//*********************************************************

DEFINE_DEVICE_TYPE(YM3806, ym3806_device, "ym3806", "YM3806 OPQ")

//-------------------------------------------------
//  ym3806_device - constructor
//-------------------------------------------------

ym3806_device::ym3806_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymfm_device_base<ymfm::ym3806>(mconfig, tag, owner, clock, YM3806)
{
}



//*********************************************************
//  YM3533 DEVICE
//*********************************************************

DEFINE_DEVICE_TYPE(YM3533, ym3533_device, "ym3533", "YM3533 OPQ")

//-------------------------------------------------
//  ym3533_device - constructor
//-------------------------------------------------

ym3533_device::ym3533_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymfm_device_base<ymfm::ym3533>(mconfig, tag, owner, clock, YM3533)
{
}
