// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

3DO Amy a.k.a. Brooktree Bt9103

TODO:
- fixed CLUT;
- Bump bitmap output to 2x2 pixels (interlace);
- HV bits;
- CLUT bypass;
- cornerstone interpolation;
- background pen replacement;
- Move scan timers from Clio to here;
- Is this chip PAL and NTSC compatible? The schematics shows the same chip reused in both
  FZ-1 USA / Europe variants;

**************************************************************************************************/

#include "emu.h"
#include "3do_amy.h"

// a.k.a. Brooktree Bt9103
DEFINE_DEVICE_TYPE(AMY, amy_device, "amy", "3DO DA9103KPJ-XN \"Amy\" Digital Color Encoder")

amy_device::amy_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, AMY, tag, owner, clock)
	, device_video_interface(mconfig, *this)
{
}

void amy_device::device_start()
{
	screen().register_screen_bitmap(m_bitmap);

	save_item(NAME(m_is_dac_enabled));
	save_item(STRUCT_MEMBER(m_custom_clut, r));
	save_item(STRUCT_MEMBER(m_custom_clut, g));
	save_item(STRUCT_MEMBER(m_custom_clut, b));
}

void amy_device::device_reset()
{
	m_is_dac_enabled = false;
}

void amy_device::clut_write(u32 data)
{
	const u8 reg = data >> 24;

	//printf("%02x %06x\n",reg, data & 0xffffff);

	// to color CLUT
	if (!BIT(reg, 7))
	{
		const u8 which = reg & 0x1f;

		switch(reg & 0x60)
		{
			case 0x00:
				m_custom_clut[which].r = (data >> 16) & 0xff;
				m_custom_clut[which].g = (data >> 8) & 0xff;
				m_custom_clut[which].b = (data >> 0) & 0xff;
				break;
			case 0x20:
				m_custom_clut[which].b = (data >> 0) & 0xff;
				break;
			case 0x40:
				m_custom_clut[which].g = (data >> 8) & 0xff;
				break;
			case 0x60:
				m_custom_clut[which].r = (data >> 16) & 0xff;
				break;
		}
	}
	else
	{
		// TODO: background color (0xe0) & control word (0xc0)
		// 0xe1: NULLOP
	}
}

void amy_device::pixel_xfer(int x, int y, u16 dot)
{
	u8 r, g, b;

	// TODO: modes thru functional fns
	r = m_custom_clut[(dot & 0x7c00) >> 10].r;
	g = m_custom_clut[(dot & 0x03e0) >> 5].g;
	b = m_custom_clut[(dot & 0x001f) >> 0].b;


	//r = (dot & 0x7c00) >> 10;
	//g = (dot & 0x03e0) >> 5;
	//b = (dot & 0x001f) >> 0;
	//r = (r << 3) | (r & 7);
	//g = (g << 3) | (g & 7);
	//b = (b << 3) | (b & 7);

	const u32 pix = r << 16 | g << 8 | b;

	for (int xi = 0; xi < 4; xi++)
		m_bitmap.pix(y, x * 4 + xi + 254) = pix;
}

void amy_device::blank_line(int y)
{
	const rectangle clip(0, y, 1592, y + 1);

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

