// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#include "emu.h"
#include "mixer.h"


//**************************************************************************
//  MIXER DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(MIXER, mixer_device, "mixer", "Generic Audio Mixer")

//-------------------------------------------------
//  mixer_device - constructor
//-------------------------------------------------

mixer_device::mixer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MIXER, tag, owner, clock),
	device_mixer_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mixer_device::device_start()
{
}
