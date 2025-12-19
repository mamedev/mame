// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
    Spectrum Next Layer2
**********************************************************************/

#include "emu.h"
#include "specnext_layer2.h"

#include "screen.h"

specnext_layer2_device::specnext_layer2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SPECNEXT_LAYER2, tag, owner, clock)
	, device_gfx_interface(mconfig, *this)
{
}

specnext_layer2_device &specnext_layer2_device::set_palette(const char *tag, u16 base_offset, u16 alt_offset)
{
	device_gfx_interface::set_palette(tag);
	m_palette_base_offset = base_offset,
	m_palette_alt_offset = alt_offset;

	return *this;
}

void specnext_layer2_device::blend(u8 &prio, u32 &target, const rgb_t pen, bool is_transparent, bool is_prio_color, u8 pcode, u8 priority_mask, u8 mixer)
{
	if (is_transparent && !mixer)
		return;

	if (mixer)
	{
		if (!prio)
			mixer = 0;
		else if (!(prio & priority_mask) || (prio & ~priority_mask))
			return;

		if (is_transparent)
			mixer = 3;

		switch (mixer)
		{
		case 0:
			target = pen;
			break;
		case 1:
			target = rgb_t(target) + pen;
			break;
		case 2:
		{
			const u8 five = pal3bit(5);
			const rgb_t t = rgb_t(target);
			target = rgb_t(0, rgb_t::clamp(t.r() + pen.r() - five)
				, rgb_t::clamp(t.g() + pen.g() - five)
				, rgb_t::clamp(t.b() + pen.b() - five));
		}
			break;
		default:
			target = palette().pen_color(0x800);
			break;
		}
	}
	else if (is_prio_color || !(prio & priority_mask))
	{
		target = pen;
		prio |= is_prio_color ? 8 : pcode;
	}
}

void specnext_layer2_device::draw_mix(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, u8 mixer)
{
	draw(screen, bitmap, cliprect, 0, 1, mixer + 1);
}

void specnext_layer2_device::draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, u8 pcode, u8 priority_mask, u8 mixer)
{
	if (!m_layer2_en)
		return;

	if (BIT(m_resolution, 1))
		draw_16(screen, bitmap, cliprect, pcode, priority_mask, mixer);
	else
		draw_256(screen, bitmap, cliprect, pcode, priority_mask, mixer);
}

void specnext_layer2_device::draw_256(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, u8 pcode, u8 priority_mask, u8 mixer)
{
	const u8 res = m_resolution + 1;
	const u16 (&info)[5] = LAYER2_INFO[m_resolution];

	rectangle clip = rectangle{ m_clip_x1 << res, (std::min<u16>(m_clip_x2 + 1, info[0]) << res) - 1, m_clip_y1, std::min<u8>(m_clip_y2, info[1] - 1) };
	u16 offset_h = m_offset_h - (info[2] << 1);
	u16 offset_v = m_offset_v - info[2];
	clip.offset(offset_h, offset_v);
	clip &= cliprect;
	clip.setx(clip.left() & ~1, clip.right() | 1);

	if (clip.empty())
		return;

	const rgb_t gt0 = rgbexpand<3,3,3>((m_global_transparent << 1) | 0, 6, 3, 0);
	const rgb_t gt1 = rgbexpand<3,3,3>((m_global_transparent << 1) | 1, 6, 3, 0);
	const u16 pen_base = (m_layer2_palette_select ? m_palette_alt_offset : m_palette_base_offset) | (m_palette_offset << 4);
	const u16 x_min = (((clip.left() - offset_h) >> 1) + m_scroll_x) % info[0];
	const bool x_overscan = m_scroll_x >= info[0] && info[3] == 256;
	for (u16 vpos = clip.top(); vpos <= clip.bottom(); vpos++)
	{
		const u16 y = (vpos - offset_v + m_scroll_y) % info[1];
		u16 x = x_min;
		const u8 *scr = m_host_ram_ptr + (m_layer2_active_bank << 14) + (y * info[4]) + (x * info[3]);
		u32 *pix = &(bitmap.pix(vpos, clip.left()));
		u8 *prio = &(screen.priority().pix(vpos, clip.left()));
		for (u16 hpos = clip.left(); hpos <= clip.right(); hpos += 2, pix += 2, prio += 2)
		{
			const u16 idx = pen_base + (*scr);
			const rgb_t pen = palette().pen_color(idx);
			const bool is_transparent = (pen == gt0) || (pen == gt1);
			const bool is_prio_color = m_pen_priority[idx];
			blend(prio[0], pix[0], pen, is_transparent, is_prio_color, pcode, priority_mask, mixer);
			blend(prio[1], pix[1], pen, is_transparent, is_prio_color, pcode, priority_mask, mixer);

			++x %= info[0];
			if (x == 0 && !x_overscan)
				scr = m_host_ram_ptr + (m_layer2_active_bank << 14) + (y * info[4]);
			else
				scr += info[3];
		}
	}
}

void specnext_layer2_device::draw_16(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, u8 pcode, u8 priority_mask, u8 mixer)
{
	const u16 (&info)[5] = LAYER2_INFO[1];

	rectangle clip = rectangle{ m_clip_x1 << 2, (std::min<u16>(m_clip_x2 + 1, info[0]) << 2) - 1, m_clip_y1, std::min<u8>(m_clip_y2, info[1] - 1) };

	u16 offset_h = m_offset_h - (info[2] << 1);
	u16 offset_v = m_offset_v - info[2];
	clip.offset(offset_h, offset_v);
	clip &= cliprect;

	if (clip.empty())
		return;

	const rgb_t gt0 = rgbexpand<3,3,3>((m_global_transparent << 1) | 0, 6, 3, 0);
	const rgb_t gt1 = rgbexpand<3,3,3>((m_global_transparent << 1) | 1, 6, 3, 0);
	const u16 pen_base = (m_layer2_palette_select ? m_palette_alt_offset : m_palette_base_offset) | (m_palette_offset << 4);
	const u16 x_min = (((clip.left() - offset_h) >> 1) + m_scroll_x) % info[0];
	const bool x_overscan = m_scroll_x >= info[0] && info[3] == 256;
	for (u16 vpos = clip.top(); vpos <= clip.bottom(); vpos++)
	{
		const u16 y = (vpos - offset_v + m_scroll_y) % info[1];
		u16 x = x_min;
		const u8 *scr = m_host_ram_ptr + (m_layer2_active_bank << 14) + (y * info[4]) + (x * info[3]);
		u32 *pix = &(bitmap.pix(vpos, clip.left()));
		u8 *prio = &(screen.priority().pix(vpos, clip.left()));
		for (u16 hpos = clip.left(); hpos <= clip.right(); hpos += 2, pix += 2, prio += 2)
		{
			if (hpos & 1)
				hpos ^= 1;
			else
			{
				const u16 idx = pen_base + (*scr >> 4);
				const rgb_t pen = palette().pen_color(idx);
				const bool is_transparent = (pen == gt0) || (pen == gt1);
				const bool is_prio_color = m_pen_priority[idx];
				blend(prio[0], pix[0], pen, is_transparent, is_prio_color, pcode, priority_mask, mixer);
			}

			{
				const u16 idx = pen_base + (*scr & 0x0f);
				const rgb_t pen = palette().pen_color(idx);
				const bool is_transparent = (pen == gt0) || (pen == gt1);
				const bool is_prio_color = m_pen_priority[idx];
				blend(prio[1], pix[1], pen, is_transparent, is_prio_color, pcode, priority_mask, mixer);
			}

			++x %= info[0];
			if (x == 0  && !x_overscan)
				scr = m_host_ram_ptr + (m_layer2_active_bank << 14) + (y * info[4]);
			else
				scr += info[3];
		}
	}
}

void specnext_layer2_device::device_add_mconfig(machine_config &config)
{
	m_offset_h = 0;
	m_offset_v = 0;
}

void specnext_layer2_device::device_start()
{
	save_item(NAME(m_global_transparent));
	save_item(NAME(m_layer2_palette_select));
	save_pointer(NAME(m_pen_priority), 512 * 4);

	save_item(NAME(m_layer2_en));
	save_item(NAME(m_resolution));
	save_item(NAME(m_palette_offset));
	save_item(NAME(m_layer2_active_bank));

	save_item(NAME(m_scroll_x));
	save_item(NAME(m_scroll_y));
	save_item(NAME(m_clip_x1));
	save_item(NAME(m_clip_x2));
	save_item(NAME(m_clip_y1));
	save_item(NAME(m_clip_y2));
}

void specnext_layer2_device::device_reset()
{
	memset(m_pen_priority, 0, 512 * 4);
}

// device type definition
DEFINE_DEVICE_TYPE(SPECNEXT_LAYER2, specnext_layer2_device, "layer2", "Spectrum Next Layer2")
