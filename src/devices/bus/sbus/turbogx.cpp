// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Sun TurboGX accelerated 8-bit color video controller

***************************************************************************/

#include "emu.h"
#include "turbogx.h"
#include "screen.h"

DEFINE_DEVICE_TYPE(SBUS_TURBOGX, sbus_turbogx_device, "turbogx", "Sun TurboGX SBus Video")

void sbus_turbogx_device::mem_map(address_map &map)
{
	map(0x00000000, 0x01ffffff).rw(FUNC(sbus_turbogx_device::unknown_r), FUNC(sbus_turbogx_device::unknown_w));
	map(0x00000000, 0x00007fff).r(FUNC(sbus_turbogx_device::rom_r));
	map(0x00200000, 0x00200007).w(FUNC(sbus_turbogx_device::palette_w));
	map(0x00800000, 0x008fffff).rw(FUNC(sbus_turbogx_device::vram_r), FUNC(sbus_turbogx_device::vram_w));
}

ROM_START( sbus_turbogx )
	ROM_REGION32_BE(0x8000, "prom", ROMREGION_ERASEFF)
	ROM_LOAD( "sunw,501-2325.bin", 0x0000, 0x8000, CRC(bbdc45f8) SHA1(e4a51d78e199cd57f2fcb9d45b25dfae2bd537e4))
ROM_END

const tiny_rom_entry *sbus_turbogx_device::device_rom_region() const
{
	return ROM_NAME( sbus_turbogx );
}

void sbus_turbogx_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(sbus_turbogx_device::screen_update));
	screen.set_size(1152, 900);
	screen.set_visarea(0, 1152-1, 0, 900-1);
	screen.set_refresh_hz(72);

	PALETTE(config, m_palette, 256).set_init(DEVICE_SELF, FUNC(sbus_turbogx_device::palette_init));
}


sbus_turbogx_device::sbus_turbogx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SBUS_TURBOGX, tag, owner, clock)
	, device_sbus_card_interface(mconfig, *this)
	, m_rom(*this, "prom")
	, m_screen(*this, "screen")
	, m_palette(*this, "palette")
{
}

void sbus_turbogx_device::device_start()
{
	m_vram = std::make_unique<uint32_t[]>(0x100000/4);

	save_item(NAME(m_palette_entry));
	save_item(NAME(m_palette_r));
	save_item(NAME(m_palette_g));
	save_item(NAME(m_palette_b));
	save_item(NAME(m_palette_step));
}

void sbus_turbogx_device::device_reset()
{
	m_palette_entry = 0;
	m_palette_r = 0;
	m_palette_g = 0;
	m_palette_b = 0;
	m_palette_step = 0;
}

void sbus_turbogx_device::install_device()
{
	m_sbus->install_device(m_base, m_base + 0x1ffffff, *this, &sbus_turbogx_device::mem_map);
}

void sbus_turbogx_device::palette_init(palette_device &palette)
{
	for (int i = 0; i < 256; i++)
	{
		const uint8_t reversed = 255 - i;
		palette.set_pen_color(i, rgb_t(reversed, reversed, reversed));
	}
}

uint32_t sbus_turbogx_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
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

READ32_MEMBER(sbus_turbogx_device::rom_r)
{
	return ((uint32_t*)m_rom->base())[offset];
}

READ32_MEMBER(sbus_turbogx_device::unknown_r)
{
	logerror("%s: unknown_r: %08x & %08x\n", machine().describe_context(), offset << 2, mem_mask);
	return 0;
}

WRITE32_MEMBER(sbus_turbogx_device::unknown_w)
{
	logerror("%s: unknown_w: %08x = %08x & %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
}

READ32_MEMBER(sbus_turbogx_device::vram_r)
{
	return m_vram[offset];
}

WRITE32_MEMBER(sbus_turbogx_device::vram_w)
{
	COMBINE_DATA(&m_vram[offset]);
}

WRITE32_MEMBER(sbus_turbogx_device::palette_w)
{
	if (offset == 0)
	{
		m_palette_entry = data >> 24;
		logerror("selecting palette entry %d\n", (uint32_t)m_palette_entry);
		m_palette_step = 0;
	}
	else if (offset == 1)
	{
		switch (m_palette_step)
		{
			case 0:
				logerror("palette entry %d red: %02x\n", (uint32_t)m_palette_entry, data);
				m_palette_r = data >> 24;
				m_palette_step++;
				break;
			case 1:
				logerror("palette entry %d green: %02x\n", (uint32_t)m_palette_entry, data);
				m_palette_g = data >> 24;
				m_palette_step++;
				break;
			case 2:
				logerror("palette entry %d blue: %02x\n", (uint32_t)m_palette_entry, data);
				m_palette_b = data >> 24;
				m_palette->set_pen_color(m_palette_entry, rgb_t(m_palette_r, m_palette_g, m_palette_b));
				m_palette_step = 0;
				m_palette_entry++;
				break;
		}
	}
}
