// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
    Spectrum Next Lores (Radastan)

    LoRes video mode
    128 x 96, 8-bit colour
    $4000-$57FF for top half
    $6000-$77FF for bottom half

    LoRes radastan (original mode from zx uno <http://zxuno.speccy.org/>)
    128 x 96, 4-bit colour
    $4000 - $57ff if timex dfile 0 active
    $6000 - $77ff if timex dfile 1 active

**********************************************************************/

#include "emu.h"
#include "specnext_lores.h"

#include "screen.h"

specnext_lores_device::specnext_lores_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SPECNEXT_LORES, tag, owner, clock)
	, device_gfx_interface(mconfig, *this)
{
}

specnext_lores_device &specnext_lores_device::set_palette(const char *tag, u16 base_offset, u16 alt_offset)
{
	device_gfx_interface::set_palette(tag);
	m_palette_base_offset = base_offset,
	m_palette_alt_offset = alt_offset;

	return *this;
}

void specnext_lores_device::draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, u8 pcode)
{
	rectangle clip = { m_clip_x1 << 1, (m_clip_x2 << 1) | 1, m_clip_y1, m_clip_y2 };
	clip.offset(m_offset_h, m_offset_v);
	clip &= cliprect;
	clip.setx(clip.left() & ~3, clip.right() | 3);

	if (clip.empty())
		return;

	const rgb_t gt0 = rgbexpand<3,3,3>((m_global_transparent << 1) | 0, 6, 3, 0);
	const rgb_t gt1 = rgbexpand<3,3,3>((m_global_transparent << 1) | 1, 6, 3, 0);
	const u16 pen_base = (m_lores_palette_select ? m_palette_alt_offset : m_palette_base_offset) | (m_lores_palette_offset << 4);
	const u8 *screen_location = m_host_ram_ptr + (5 << 14);

	const u16 x_min = ((((clip.left() - m_offset_h) >> 1) + m_scroll_x) % SCREEN_AREA.width()) >> 1;
	for (u16 vpos = clip.top(); vpos <= clip.bottom(); ++vpos)
	{
		const u16 y = ((vpos - m_offset_v + m_scroll_y) % SCREEN_AREA.height()) >> 1;
		u16 x = x_min ;
		const u8 *scr = &screen_location[x + ((y < 48) ?  128 * y : (128 * (y - 48) + 0x2000))];

		u32 *pix = &(bitmap.pix(vpos, clip.left()));
		u8 *prio = &(screen.priority().pix(vpos, clip.left()));
		for (u16 hpos = clip.left(); hpos <= clip.right(); hpos += 4, pix += 4, prio += 4)
		{
			const rgb_t pen = palette().pen_color(pen_base + *scr);
			if ((pen != gt0) && (pen != gt1))
			{
				*(pix + 0) = pen;
				*(pix + 1) = pen;
				*(pix + 2) = pen;
				*(pix + 3) = pen;
				*(prio + 0) |= pcode;
				*(prio + 1) |= pcode;
				*(prio + 2) |= pcode;
				*(prio + 3) |= pcode;
			}

			++x %= SCREEN_AREA.width() >> 1;
			if (x == 0)
				scr = &screen_location[(y < 48) ?  128 * y : (128 * (y - 48) + 0x2000)];
			else
				++scr;
		}
	}
}

void specnext_lores_device::device_add_mconfig(machine_config &config)
{
	m_offset_h = 0;
	m_offset_v = 0;
}

void specnext_lores_device::device_start()
{
	save_item(NAME(m_global_transparent));
	save_item(NAME(m_lores_palette_select));

	save_item(NAME(m_mode));
	save_item(NAME(m_dfile));
	save_item(NAME(m_ulap_en));

	save_item(NAME(m_lores_palette_offset));
	save_item(NAME(m_clip_x1));
	save_item(NAME(m_clip_x2));
	save_item(NAME(m_clip_y1));
	save_item(NAME(m_clip_y2));
	save_item(NAME(m_scroll_x));
	save_item(NAME(m_scroll_y));
}

// device type definition
DEFINE_DEVICE_TYPE(SPECNEXT_LORES, specnext_lores_device, "lores", "Spectrum Next Lores")
