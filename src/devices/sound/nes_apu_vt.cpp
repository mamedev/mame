// license:BSD-3-Clause
// copyright-holders:David Haywood

// The XOP is the NES APU type found in VT platforms, it roughly doubles the functionality of the APU
// but in ways that means simply installing 2 APU devices isn't entirely correct (registers on one
// APU module having an effect on the other)

// TODO: everything

#include "emu.h"
#include "nes_apu_vt.h"

DEFINE_DEVICE_TYPE(NES_APU_VT, nes_apu_vt_device, "nes_apu_vt", "XOP APU")

nes_apu_vt_device::nes_apu_vt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nesapu_device(mconfig, NES_APU_VT, tag, owner, clock)
{
}

void nes_apu_vt_device::device_start()
{
	nesapu_device::device_start();
}

void nes_apu_vt_device::device_reset()
{
	nesapu_device::device_reset();
}
