// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

******************************************************************************/

#include "emu.h"
#include "video/ppu2c0x_sh6578.h"

// devices
DEFINE_DEVICE_TYPE(PPU_SH6578, ppu_sh6578_device, "ppu_sh6578", "SH6578 PPU (NTSC)")
DEFINE_DEVICE_TYPE(PPU_SH6578PAL, ppu_sh6578pal_device, "ppu_sh6578pal", "SH6578 PPU (PAL)")

ppu_sh6578_device::ppu_sh6578_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock) :
	ppu2c0x_device(mconfig, type, tag, owner, clock)
{
}

ppu_sh6578_device::ppu_sh6578_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	ppu_sh6578_device(mconfig, PPU_SH6578, tag, owner, clock)
{
}

ppu_sh6578pal_device::ppu_sh6578pal_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	ppu_sh6578_device(mconfig, PPU_SH6578PAL, tag, owner, clock)
{
	m_scanlines_per_frame = PAL_SCANLINES_PER_FRAME;
	m_vblank_first_scanline = VBLANK_FIRST_SCANLINE_PALC;
}

void ppu_sh6578_device::device_start()
{
	ppu2c0x_device::device_start();
}

void ppu_sh6578_device::device_reset()
{
	ppu2c0x_device::device_reset();
}
