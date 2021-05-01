// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "emu.h"
#include "ymf262.h"


DEFINE_DEVICE_TYPE(YMF262, ymf262_device, "ymf262", "YMF262 OPL3")


//*********************************************************
//  YMF262 DEVICE
//*********************************************************

//-------------------------------------------------
//  ymf262_device - constructor
//-------------------------------------------------

ymf262_device::ymf262_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymfm_device_base<ymfm::ymf262>(mconfig, tag, owner, clock, YMF262)
{
}
