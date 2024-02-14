// license:BSD-3-Clause
/**********************************************************************

	Spectrum Next ULA

**********************************************************************/

#include "emu.h"
#include "specnext_ula.h"

#include "screen.h"


specnext_ula_device::specnext_ula_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SPECNEXT_ULA, tag, owner, clock)
	, device_gfx_interface(mconfig, *this)
{
}

specnext_ula_device &specnext_ula_device::set_palette(const char *tag, u16 base_offset, u16 alt_offset)
{
	device_gfx_interface::set_palette(tag);
	m_palette_base_offset = base_offset,
	m_palette_alt_offset = alt_offset;
	return *this;
}

void specnext_ula_device::draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, bool flash)
{
	rectangle clip = { m_ula_clip_x1 << 1, (m_ula_clip_x2 << 1) | 1, m_ula_clip_y1, m_ula_clip_y2 };
	clip.offset(m_offset_h, m_offset_v);
	clip &= cliprect;
	clip.setx(clip.left() + (clip.left() & 1), clip.right() | 1);

	if (clip.empty())
		return;

	const rgb_t gt0 = rgbexpand<3,3,3>((m_global_transparent << 1) | 0, 6, 3, 0);
	const rgb_t gt1 = rgbexpand<3,3,3>((m_global_transparent << 1) | 1, 6, 3, 0);
	const u16 pal_base = m_ula_palette_select ? m_palette_alt_offset : m_palette_base_offset;
	const u8 *screen_location = m_host_ram_ptr + ((m_ula_shadow_en ? 7 : 5) << 14);

	u8 ink_mask = m_ulanext_format;
	u8 pap_shift;
	switch (m_ulanext_format)
	{
	case 0x01: pap_shift = 1; break;
	case 0x03: pap_shift = 2; break;
	case 0x07: pap_shift = 3; break;
	case 0x0f: pap_shift = 4; break;
	case 0x1f: pap_shift = 5; break;
	case 0x3f: pap_shift = 6; break;
	case 0x7f: pap_shift = 7; break;
	default:
		ink_mask = 0xff;
		pap_shift = ~0;
	}

	const u16 x_min = (((clip.left() - m_offset_h) >> 1) + m_ula_scroll_x) % SCREEN_AREA.width();
	for (u16 vpos = clip.top(); vpos <= clip.bottom(); vpos++)
	{
		u16 hpos = clip.left();
		u16 y = (vpos - m_offset_v + m_ula_scroll_y) % SCREEN_AREA.height();
		u16 x = x_min;
		const u8 *scr = &screen_location[((y & 7) << 8) | ((y & 0x38) << 2) | ((y & 0xc0) << 5) | (x >> 3)];
		const u8 *attr = &screen_location[0x1800 + (((y & 0xf8) << 2) | (x >> 3))];
		u16 *pix = &(bitmap.pix(vpos, hpos));
		while (hpos <= clip.right())
		{
			u8 ink, pap;
			if (m_ulanext_en)
			{
				ink = *attr & ink_mask;
				pap = pap_shift == ~0 ? (0x800 - pal_base) : ((*attr >> pap_shift) | 0x80);
				flash = 0;
			}
			else if (m_ulap_en)
			{
				ink = ((*attr >> 3) & 0x18) | (*attr & 0x07);
				pap = ((*attr >> 3) & 0x1f) | 0x20;
				flash = 0;
			}
			else
			{
				ink = ((*attr >> 3) & 0x08) | (*attr & 0x07);
				pap = ((*attr >> 3) & 0x0f) | 0x10;
			}

			const u8 pix8 = (flash && (*attr & 0x80)) ? ~*scr : *scr;
			for (u8 b = (0x80 >> (x & 7)); b && (hpos <= clip.right()); b >>= 1, x++, hpos += 2, pix += 2)
			{
				const u16 pen = pal_base + ((pix8 & b) ? ink : pap);
				if (palette().pen_color(pen) != gt0 && palette().pen_color(pen) != gt1)
				{
					*pix = pen;
					*(pix + 1) = pen;
				}
			}
			x %= SCREEN_AREA.width();
			if (x == 0)
			{
				scr = &screen_location[((y & 7) << 8) | ((y & 0x38) << 2) | ((y & 0xc0) << 5)];
				attr = &screen_location[0x1800 + (((y & 0xf8) << 2))];
			}
			else
			{
				++scr;
				++attr;
			}
		}
	}
}

void specnext_ula_device::device_add_mconfig(machine_config &config)
{
	m_offset_h = 0;
	m_offset_v = 0;
}

void specnext_ula_device::device_start()
{
	save_item(NAME(m_global_transparent));
	save_item(NAME(m_ula_palette_select));
	save_item(NAME(m_ulanext_en));
	save_item(NAME(m_ulanext_format));
	save_item(NAME(m_ulap_en));
	save_item(NAME(m_ula_shadow_en));
	save_item(NAME(m_ula_clip_x1));
	save_item(NAME(m_ula_clip_x2));
	save_item(NAME(m_ula_clip_y1));
	save_item(NAME(m_ula_clip_y2));
	save_item(NAME(m_ula_scroll_x));
	save_item(NAME(m_ula_scroll_y));
	save_item(NAME(m_ula_fine_scroll_x));
}

// device type definition
DEFINE_DEVICE_TYPE(SPECNEXT_ULA, specnext_ula_device, "ula", "Spectrum Next ULA")
