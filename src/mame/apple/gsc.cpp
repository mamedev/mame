// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Apple Gray Scale Controller (GSC) video, aka Chips & Technologies 65210
    Emulation by R. Belmont

    Reference:
    https://github.com/elliotnunn/supermario/blob/master/base/SuperMarioProj.1994-02-09/Internal/Asm/HardwarePrivateEqu.a#L1201
    https://github.com/elliotnunn/supermario/tree/master/base/SuperMarioProj.1994-02-09/DeclData/DeclVideo/DBLite

    panel 4 = 640x480 JeDI (PowerBook 150)
    panel 5 = 640x400 Dartanian (PowerBook 160/180)
    panel 6 = 640x400 DBLite (PowerBook Duo 210/230/250)
*/

#include "emu.h"
#include "gsc.h"

DEFINE_DEVICE_TYPE(GSC, gsc_device, "applegsc", "Apple Gray Scale Controller video")

static constexpr u8 GSC_GRAYSCALE               = 0x04;

static constexpr u8 GSC_GS_DISPLAY_ENABLED      = 5;
static constexpr u8 GSC_GS_MODE_MASK            = 0x03;

//-------------------------------------------------
//  ADDRESS_MAP
//-------------------------------------------------

void gsc_device::map(address_map &map)
{
	map(0x00f20000, 0x00f21fff).rw(FUNC(gsc_device::gsc_r), FUNC(gsc_device::gsc_w));
	map(0x10000000, 0x1001ffff).rw(FUNC(gsc_device::vram_r), FUNC(gsc_device::vram_w)).mirror(0x0ffe0000);
}

gsc_device::gsc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, GSC, tag, owner, clock),
	m_screen(*this, "screen"),
	m_palette(*this, "palette")
{
	std::fill(std::begin(m_gsc_regs), std::end(m_gsc_regs), 0);
}

void gsc_device::device_start()
{
	m_vram = std::make_unique<u32[]>(0x20000);

	save_item(NAME(m_gsc_regs));
	save_item(NAME(m_pmu_blank_display));
	save_pointer(NAME(m_vram), 0x20000);
}

void gsc_device::device_reset()
{
	if (m_gsc_panel_id == 4)
	{
		m_screen->set_raw(25175000, 800, 0, 640, 525, 0, 480);
	}
}

void gsc_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// these parameters are not real; we know the refresh rate is ~60.15 Hz and that's it
	m_screen->set_raw(21604953, 800, 0, 640, 449, 0, 400);

	m_screen->set_palette(m_palette);
	m_screen->set_screen_update(FUNC(gsc_device::screen_update_gsc));

	PALETTE(config, m_palette, FUNC(gsc_device::macgsc_palette), 16);
}

void gsc_device::macgsc_palette(palette_device &palette) const
{
	for (u8 i = 0; i < 16; i++)
	{
		palette.set_pen_color(i, ((15 - i) << 4) | 0xf, ((15 - i) << 4) | 0xf, ((15 - i) << 4) | 0xf);
	}
}

u32 gsc_device::screen_update_gsc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]);
	int height = (m_gsc_panel_id == 4) ? 480 : 400;

	// is the display enabled?
	if (!BIT(m_gsc_regs[GSC_GRAYSCALE], GSC_GS_DISPLAY_ENABLED) && !m_pmu_blank_display)
	{
		bitmap.fill(0xf, cliprect);
		return 0;
	}

	switch (m_gsc_regs[4] & GSC_GS_MODE_MASK)
	{
	case 0:
		for (int y = 0; y < height; y++)
		{
			u16 *line = &bitmap.pix(y);

			for (int x = 0; x < 640; x += 8)
			{
				u8 const pixels = vram8[(y * 640 / 8) + (x / 8)];
				*line++ = ((pixels >> 7) & 1) ? 0xf : 0;
				*line++ = ((pixels >> 6) & 1) ? 0xf : 0;
				*line++ = ((pixels >> 5) & 1) ? 0xf : 0;
				*line++ = ((pixels >> 4) & 1) ? 0xf : 0;
				*line++ = ((pixels >> 3) & 1) ? 0xf : 0;
				*line++ = ((pixels >> 2) & 1) ? 0xf : 0;
				*line++ = ((pixels >> 1) & 1) ? 0xf : 0;
				*line++ = (pixels & 1) ? 0xf : 0;
			}
		}
		break;

	case 1:
		for (int y = 0; y < height; y++)
		{
			u16 *line = &bitmap.pix(y);

			for (int x = 0; x < 640; x += 4)
			{
				u8 const pixels = vram8[(y * 640 / 4) + (x / 4)];
				*line++ = ((pixels >> 4) & 0xc);
				*line++ = ((pixels >> 2) & 0xc);
				*line++ = (pixels & 0xc);
				*line++ = ((pixels << 2) & 0xc);
			}
		}
		break;

	case 2:
		for (int y = 0; y < height; y++)
		{
			u16 *line = &bitmap.pix(y);

			for (int x = 0; x < 640; x += 2)
			{
				u8 const pixels = vram8[(y * 640 / 2) + (x / 2)];
				*line++ = pixels >> 4;
				*line++ = pixels & 0xf;
			}
		}
		break;
	}
	return 0;
}

void gsc_device::set_panel_id(int panel_id)
{
	m_gsc_panel_id = panel_id;
}

u8 gsc_device::gsc_r(offs_t offset)
{
	if (offset == 1)
	{
		return m_gsc_panel_id;
	}

	return m_gsc_regs[offset & 0x1f];
}

void gsc_device::gsc_w(offs_t offset, u8 data)
{
	m_gsc_regs[offset & 0x1f] = data;
}

u32 gsc_device::vram_r(offs_t offset)
{
	return m_vram[offset & 0x1ffff];
}

void gsc_device::vram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_vram[offset & 0x1ffff]);
}
