// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

3DO Amy a.k.a. Brooktree Bt9103

TODO:
- Bump bitmap output to 2x2 pixels (interlace);
- Complete HV bits, shared with Madam;
- Fixed CLUT(s) LSB selectors (bits 12-11 of display-control)
- Cornerstone interpolation;
- Background pen replacement for SlipStream merging (should matter if/when VCD module is dumped);
- Move scan timers from Clio to here;
- Is this chip PAL and NTSC compatible? The schematics shows the same chip reused in both
  FZ-1 USA / Europe variants;

**************************************************************************************************/

#include "emu.h"
#include "3do_amy.h"

#define VERBOSE (0)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

// a.k.a. Brooktree Bt9103
DEFINE_DEVICE_TYPE(AMY, amy_device, "amy", "3DO DA9103KPJ-XN \"Amy\" Digital Color Encoder")

amy_device::amy_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, AMY, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_is_pal(false)
{
}

void amy_device::device_start()
{
	screen().register_screen_bitmap(m_bitmap);

	// we treat the CLUT as a single contiguous area for performance
	for (int i = 0; i < 32; i++)
	{
		// TODO: ramping for fixed CLUT lower part can also be fixed as 0 or be "random"
		// probably the best place to initialize this would be the display-control word itself,
		// where the RNG presumably acts at the start of the scanline ...
		const u8 color_ramp = pal5bit(i);
		m_clut[FIXED_CLUT_BASE + i].r = color_ramp;
		m_clut[FIXED_CLUT_BASE + i].g = color_ramp;
		m_clut[FIXED_CLUT_BASE + i].b = color_ramp;
	}

	m_display_hclocks = (m_is_pal ? 384 : 320) * 4;

	save_item(NAME(m_is_dac_enabled));
	save_item(STRUCT_MEMBER(m_clut, r));
	save_item(STRUCT_MEMBER(m_clut, g));
	save_item(STRUCT_MEMBER(m_clut, b));
	save_item(NAME(m_clut_mask));
}

void amy_device::device_reset()
{
	m_is_dac_enabled = false;
	// presumably starts in undefined state
	m_clut_mask = 0;
}

void amy_device::clut_write(u32 data)
{
	const u8 reg = data >> 24;

	// to custom CLUT
	if (!BIT(reg, 7))
	{
		const u8 which = reg & 0x1f;

		LOG("Color CLUT %d mode %02x %06x\n", which, reg & 0x60, data & 0xff'ffff);

		switch(reg & 0x60)
		{
			case 0x00:
				m_clut[which].r = (data >> 16) & 0xff;
				m_clut[which].g = (data >> 8) & 0xff;
				m_clut[which].b = (data >> 0) & 0xff;
				break;
			case 0x20:
				m_clut[which].b = (data >> 0) & 0xff;
				break;
			case 0x40:
				m_clut[which].g = (data >> 8) & 0xff;
				break;
			case 0x60:
				m_clut[which].r = (data >> 16) & 0xff;
				break;
		}
	}
	else
	{
		switch (reg)
		{
			// NULLOP
			case 0xe1: break;
			case 0xe0:
				LOG("0xe0: background color %06x\n", data & 0xff'ffff);
				break;
			// NOTE: 0xc1 / 0xc3 would go here too but bit 24: "not available to a user task"
			case 0xc0:
			case 0xc2:
			{
				LOG("0xc0: display-control word %07x\n", data & 0x03ff'ffff);

				const bool fixed_clut_enable = !!BIT(data, 25);
				m_clut_mask = fixed_clut_enable << 15;
				LOG("    fixed CLUT=%d\n", fixed_clut_enable);

				// TODO: everything else

				break;
			}
			default:
				LOG("%02x: unknown word set! %06x\n", reg, data & 0xff'ffff);
				break;
		}
	}
}

void amy_device::pixel_xfer(int x, int y, u16 dot)
{
	u8 r, g, b;

	// (res >> 15) << 5
	// - virtuoso separates output so that the resulting V bit selects the fixed CLUT.
	const u16 clut_select = (dot & m_clut_mask) >> (15 - 5);
	r = m_clut[clut_select | ((dot & 0x7c00) >> 10)].r;
	g = m_clut[clut_select | ((dot & 0x03e0) >>  5)].g;
	b = m_clut[clut_select | ((dot & 0x001f) >>  0)].b;

	const u32 pix = r << 16 | g << 8 | b;

	for (int xi = 0; xi < 4; xi++)
		m_bitmap.pix(y, x * 4 + xi + 254) = pix;
}

void amy_device::blank_line(int y)
{
	const rectangle clip(0, y, m_display_hclocks, y + 1);

	// TODO: vblank color really
	m_bitmap.fill(rgb_t::black(), clip);
}

void amy_device::dac_enable(bool enabled)
{
	m_is_dac_enabled = enabled;
}

u32 amy_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!m_is_dac_enabled)
	{
		// TODO: vblank or background color
		bitmap.fill(rgb_t::black(), cliprect);
		return 0;
	}

	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

