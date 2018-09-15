// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Sun cgthree color video controller

***************************************************************************/

#include "emu.h"
#include "cgthree.h"
#include "screen.h"

DEFINE_DEVICE_TYPE(SBUS_CGTHREE, sbus_cgthree_device, "cgthree", "Sun cgthree SBus Video")

void sbus_cgthree_device::mem_map(address_map &map)
{
	map(0x00000000, 0x01ffffff).rw(FUNC(sbus_cgthree_device::unknown_r), FUNC(sbus_cgthree_device::unknown_w));
	map(0x00000000, 0x000007ff).r(FUNC(sbus_cgthree_device::rom_r));
	map(0x00400000, 0x00400007).w(FUNC(sbus_cgthree_device::palette_w));
	map(0x007ff800, 0x007ff81f).rw(FUNC(sbus_cgthree_device::regs_r), FUNC(sbus_cgthree_device::regs_w));
	map(0x00800000, 0x008fffff).rw(FUNC(sbus_cgthree_device::vram_r), FUNC(sbus_cgthree_device::vram_w));
	map(0x00bff800, 0x00cff7ff).rw(FUNC(sbus_cgthree_device::vram_r), FUNC(sbus_cgthree_device::vram_w));
}

ROM_START( sbus_cgthree )
	ROM_REGION32_BE(0x800, "prom", ROMREGION_ERASEFF)
	ROM_LOAD( "sunw,501-1415.bin", 0x0000, 0x0800, CRC(d1eb6f4d) SHA1(9bef98b2784b6e70167337bb27cd07952b348b5a))
ROM_END

const tiny_rom_entry *sbus_cgthree_device::device_rom_region() const
{
	return ROM_NAME( sbus_cgthree );
}

void sbus_cgthree_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(sbus_cgthree_device::screen_update));
	screen.set_size(1152, 900);
	screen.set_visarea(0, 1152-1, 0, 900-1);
	screen.set_refresh_hz(72);

	PALETTE(config, m_palette, 256).set_init(DEVICE_SELF, FUNC(sbus_cgthree_device::palette_init));
}

sbus_cgthree_device::sbus_cgthree_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SBUS_CGTHREE, tag, owner, clock)
	, device_sbus_card_interface(mconfig, *this)
	, m_rom(*this, "prom")
	, m_screen(*this, "screen")
	, m_palette(*this, "palette")
{
}

void sbus_cgthree_device::device_start()
{
	m_vram = std::make_unique<uint32_t[]>(0x100000/4);

	save_item(NAME(m_palette_entry));
	save_item(NAME(m_palette_r));
	save_item(NAME(m_palette_g));
	save_item(NAME(m_palette_b));
	save_item(NAME(m_palette_step));
}

void sbus_cgthree_device::device_reset()
{
	m_palette_entry = 0;
	m_palette_r = 0;
	m_palette_g = 0;
	m_palette_b = 0;
	m_palette_step = 0;
}

void sbus_cgthree_device::install_device()
{
	m_sbus->install_device(m_base, m_base + 0x1ffffff, *this, &sbus_cgthree_device::mem_map);
}

void sbus_cgthree_device::palette_init(palette_device &palette)
{
	for (int i = 0; i < 256; i++)
	{
		const uint8_t reversed = 255 - i;
		palette.set_pen_color(i, rgb_t(reversed, reversed, reversed));
	}
}

uint32_t sbus_cgthree_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *pens = m_palette->pens();
	uint8_t *vram = (uint8_t *)&m_vram[0];

	for (int y = 0; y < 900; y++)
	{
		uint32_t *scanline = &bitmap.pix32(y);
		for (int x = 0; x < 1152; x++)
		{
			const uint8_t pixel = vram[y * 1152 + BYTE4_XOR_BE(x)];
			*scanline++ = pens[pixel];
		}
	}

	return 0;
}

READ32_MEMBER(sbus_cgthree_device::unknown_r)
{
	logerror("%s: unknown_r: %08x & %08x\n", machine().describe_context(), offset << 2, mem_mask);
	return 0;
}

WRITE32_MEMBER(sbus_cgthree_device::unknown_w)
{
	logerror("%s: unknown_w: %08x = %08x & %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
}

WRITE32_MEMBER(sbus_cgthree_device::palette_w)
{
	if (offset == 0)
	{
		m_palette_entry = data >> 24;
		m_palette_step = 0;
	}
	else
	{
		switch (m_palette_step)
		{
			case 0:
				m_palette_r = data >> 24;
				m_palette_step++;
				break;
			case 1:
				m_palette_g = data >> 24;
				m_palette_step++;
				break;
			case 2:
				m_palette_b = data >> 24;
				m_palette->set_pen_color(m_palette_entry, rgb_t(m_palette_r, m_palette_g, m_palette_b));
				m_palette_step = 0;
				m_palette_entry++;
				break;
		}
	}
}

READ8_MEMBER(sbus_cgthree_device::regs_r)
{
	logerror("%s: regs_r: Unimplemented: %08x\n", machine().describe_context(), 0x7ff800 + offset);
	return 0;
}

WRITE8_MEMBER(sbus_cgthree_device::regs_w)
{
	logerror("%s: regs_w: Unimplemented: %08x = %02x\n", machine().describe_context(), 0x7ff800 + offset, data);
}

READ32_MEMBER(sbus_cgthree_device::rom_r)
{
	return ((uint32_t*)m_rom->base())[offset];
}

READ32_MEMBER(sbus_cgthree_device::vram_r)
{
	return m_vram[offset];
}

WRITE32_MEMBER(sbus_cgthree_device::vram_w)
{
	COMBINE_DATA(&m_vram[offset]);
}
