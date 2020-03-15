// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

******************************************************************************/

#include "emu.h"
#include "video/ppu2c0x_sh6578.h"

#define LOG_PPU_EXTRA       (1U << 0)

//#define VERBOSE             (LOG_PPU_EXTRA)
#define VERBOSE             (0)

#include "logmacro.h"

// devices
DEFINE_DEVICE_TYPE(PPU_SH6578, ppu_sh6578_device, "ppu_sh6578", "SH6578 PPU (NTSC)")
DEFINE_DEVICE_TYPE(PPU_SH6578PAL, ppu_sh6578pal_device, "ppu_sh6578pal", "SH6578 PPU (PAL)")

ppu_sh6578_device::ppu_sh6578_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock) :
	ppu2c0x_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(ppu_sh6578_device::ppu_internal_map), this))
{
	m_paletteram_in_ppuspace = false;
	m_line_write_increment_large = 64;
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


// Palette RAM is _NOT_ mapped to PPU space here
void ppu_sh6578_device::ppu_internal_map(address_map& map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x27ff).ram();
	map(0x2800, 0x7fff).nopr();

	map(0x8000, 0xffff).ram(); // machine specific?
}

void ppu_sh6578_device::device_start()
{
	ppu2c0x_device::device_start();

	m_palette_ram.resize(0x40);

	for (int i = 0; i < 0x40; i++)
		m_palette_ram[i] = 0x00;
}

void ppu_sh6578_device::device_reset()
{
	ppu2c0x_device::device_reset();
	m_colsel_pntstart = 0x00;
}

void ppu_sh6578_device::draw_background(uint8_t* line_priority)
{

}

void ppu_sh6578_device::draw_sprites(uint8_t* line_priority)
{

}

void ppu_sh6578_device::write(offs_t offset, uint8_t data)
{
	if (offset < 0x8)
	{
		ppu2c0x_device::write(offset, data);
	}
	else if (offset == 0x8)
	{

	}
	else
	{
		LOGMASKED(LOG_PPU_EXTRA, "%s: ppu_sh6578_device::write : Color Select & PNT Start Address : %02x\n", machine().describe_context(), data);
		m_colsel_pntstart = data;
	}
}


uint8_t ppu_sh6578_device::read(offs_t offset)
{
	if (offset < 0x8)
	{
		return ppu2c0x_device::read(offset);
	}
	else if (offset == 0x8)
	{
		LOGMASKED(LOG_PPU_EXTRA, "%s: ppu_sh6578_device::read : Color Select & PNT Start Address\n", machine().describe_context());
		return m_colsel_pntstart;
	}
	else
	{
		return 0x00;
	}
}

void ppu_sh6578_device::palette_write(offs_t offset, uint8_t data)
{
	m_palette_ram[offset] = data;
}

uint8_t ppu_sh6578_device::palette_read(offs_t offset)
{
	return m_palette_ram[offset];
}
