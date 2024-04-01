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

u8 specnext_ula_device::screen_mode()
{
	return m_ula_shadow_en ? 0b000 : (m_port_ff_reg & 7);
}

void specnext_ula_device::draw_border(bitmap_rgb32 &bitmap, const rectangle &cliprect, u8 border_color)
{
	rgb_t pap = m_ulanext_en
		? parse_attribute(border_color << m_pap_shift).first
		: parse_attribute(border_color << 3).first;

	const rgb_t gt0 = rgbexpand<3,3,3>((m_global_transparent << 1) | 0, 6, 3, 0);
	const rgb_t gt1 = rgbexpand<3,3,3>((m_global_transparent << 1) | 1, 6, 3, 0);
	if (pap == gt0 || pap == gt1)
		pap = palette().pen_color(UTM_FALLBACK_PEN);

	bitmap.fill(pap, cliprect);
}

void specnext_ula_device::draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, bool flash)
{
	if (screen_mode() == 6)
		draw_hires(bitmap, cliprect);
	else
		draw_ula(bitmap, cliprect, flash);
}

void specnext_ula_device::draw_ula(bitmap_rgb32 &bitmap, const rectangle &cliprect, bool flash)
{
	rectangle clip = { m_ula_clip_x1 << 1, (m_ula_clip_x2 << 1) | 1, m_ula_clip_y1, m_ula_clip_y2 };
	clip.offset(m_offset_h, m_offset_v);
	clip &= cliprect;
	clip.setx(clip.left() & ~1, clip.right() | 1);

	if (clip.empty())
		return;

	flash &= !m_ulanext_en && !m_ulap_en;
	const rgb_t gt0 = rgbexpand<3,3,3>((m_global_transparent << 1) | 0, 6, 3, 0);
	const rgb_t gt1 = rgbexpand<3,3,3>((m_global_transparent << 1) | 1, 6, 3, 0);
	const u8 *screen_location = m_host_ram_ptr + ((m_ula_shadow_en ? 7 : 5) << 14);

	const u16 x_min = (((clip.left() - m_offset_h) >> 1) + m_ula_scroll_x) % SCREEN_AREA.width();
	for (u16 vpos = clip.top(); vpos <= clip.bottom(); vpos++)
	{
		u16 hpos = clip.left();
		u16 y = (vpos - m_offset_v + m_ula_scroll_y) % SCREEN_AREA.height();
		u16 x = x_min;
		const u8 *scr = &screen_location[((y & 7) << 8) | ((y & 0x38) << 2) | ((y & 0xc0) << 5) | (x >> 3)];
		const u8 *attr = &screen_location[0x1800 + (((y & 0xf8) << 2) | (x >> 3))];
		u32 *pix = &(bitmap.pix(vpos, hpos));
		while (hpos <= clip.right())
		{
			const std::pair<rgb_t, rgb_t> pi = parse_attribute(*attr);
			const rgb_t pap = pi.first;
			const rgb_t ink = pi.second;

			const u8 pix8 = (flash && (*attr & 0x80)) ? ~*scr : *scr;
			for (u8 b = (0x80 >> (x & 7)); b && (hpos <= clip.right()); b >>= 1, x++, hpos += 2, pix += 2)
			{
				const rgb_t pen = (pix8 & b) ? ink : pap;
				if ((pen != gt0) && (pen != gt1))
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

void specnext_ula_device::draw_hires(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rectangle clip = { m_ula_clip_x1 << 1, (m_ula_clip_x2 << 1) | 1, m_ula_clip_y1, m_ula_clip_y2 };
	clip.offset(m_offset_h, m_offset_v);
	clip &= cliprect;
	clip.setx(clip.left() & ~1, clip.right() | 1);

	if (clip.empty())
		return;

	const rgb_t gt0 = rgbexpand<3,3,3>((m_global_transparent << 1) | 0, 6, 3, 0);
	const rgb_t gt1 = rgbexpand<3,3,3>((m_global_transparent << 1) | 1, 6, 3, 0);
	const u8 *screen_location = m_host_ram_ptr + (5 << 14);

	const u8 attr = 0x40 | (~m_port_ff_reg & 0x38) | (BIT(m_port_ff_reg, 3, 3));
	const std::pair<rgb_t, rgb_t> pi = parse_attribute(attr);
	const rgb_t pap = pi.first;
	const rgb_t ink = pi.second;

	const u16 x_min = ((clip.left() - m_offset_h) + (m_ula_scroll_x << 1)) % (SCREEN_AREA.width() << 1);
	for (u16 vpos = clip.top(); vpos <= clip.bottom(); vpos++)
	{
		u16 hpos = clip.left();
		u16 y = (vpos - m_offset_v + m_ula_scroll_y) % SCREEN_AREA.height();
		u16 x = x_min;
		const u8 *scr = &screen_location[((y & 7) << 8) | ((y & 0x38) << 2) | ((y & 0xc0) << 5) | ((x >> 3) >> 1)];
		u32 *pix = &(bitmap.pix(vpos, hpos));
		while (hpos <= clip.right())
		{

			const u8 pix8 = *(scr + 0x2000 * BIT(x, 3));
			for (u8 b = (0x80 >> (x & 7)); b && (hpos <= clip.right()); b >>= 1, ++x, ++hpos, ++pix)
			{
				const rgb_t pen = (pix8 & b) ? ink : pap;
				if (pen != gt0 && pen != gt1)
					*pix = pen;
			}
			x %= SCREEN_AREA.width() << 1;
			if (x == 0)
				scr = &screen_location[((y & 7) << 8) | ((y & 0x38) << 2) | ((y & 0xc0) << 5)];
			else if (BIT(~x, 3))
				++scr;
		}
	}
}

void specnext_ula_device::ulanext_format_w(u8 ulanext_format)
{
	m_ulanext_format = ulanext_format;

	m_ink_mask = m_ulanext_format;
	switch (m_ulanext_format)
	{
	case 0x01: m_pap_shift = 1; break;
	case 0x03: m_pap_shift = 2; break;
	case 0x07: m_pap_shift = 3; break;
	case 0x0f: m_pap_shift = 4; break;
	case 0x1f: m_pap_shift = 5; break;
	case 0x3f: m_pap_shift = 6; break;
	case 0x7f: m_pap_shift = 7; break;
	default:
		m_ink_mask = 0xff;
		m_pap_shift = 8;
	}
}

std::pair<rgb_t, rgb_t> specnext_ula_device::parse_attribute(u8 attr)
{
	const u16 pal_base = m_ula_palette_select ? m_palette_alt_offset : m_palette_base_offset;
	u16 pap, ink;
	if (m_ulanext_en)
	{
		ink = attr & m_ink_mask;
		pap = (m_ulanext_format == 0xff) ? (UTM_FALLBACK_PEN - pal_base) : ((attr >> m_pap_shift) | 0x80);
	}
	else if (m_ulap_en)
	{
		ink = 0xc0 | ((attr & 0xc0) >> 2) | BIT(attr, 0, 3);
		pap = 0xc8 | ((attr & 0xc0) >> 2) | BIT(attr, 3, 3);
	}
	else
	{
		ink = 0x00 | ((attr & 0x40) >> 3) | BIT(attr, 0, 3);
		pap = 0x10 | ((attr & 0x40) >> 3) | BIT(attr, 3, 3);
	}

	return { palette().pen_color(pal_base + pap), palette().pen_color(pal_base + ink) };
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
	save_item(NAME(m_ink_mask));
	save_item(NAME(m_pap_shift));
	save_item(NAME(m_ulap_en));
	save_item(NAME(m_port_ff_reg));
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
