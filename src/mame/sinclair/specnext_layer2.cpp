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

void specnext_layer2_device::draw_mix(screen_device &screen, bitmap_rgb32 &bitmap, bitmap_rgb32 &blendprio, const rectangle &cliprect, u8 mixer)
{
	if (!m_layer2_en)
		return;

	const rgb_t gt0 = rgbexpand<3,3,3>((m_global_transparent << 1) | 0, 6, 3, 0);
	const rgb_t gt1 = rgbexpand<3,3,3>((m_global_transparent << 1) | 1, 6, 3, 0);
	const rgb_t fb = palette().pen_color(0x800);
	auto blend_op = [gt0, gt1, fb, mixer](u8 &prio, u32 &target, u32 &priotarget, const rgb_t pen, bool is_prio_color)
	{
		if ((pen == gt0) || (pen == gt1))
		{
			target = fb;
		}
		else
		{
			u32 &to = is_prio_color ? priotarget : target;
			if (!prio)
			{
				to = pen;
			}
			else if (mixer == 0)
			{
				to = rgb_t(target) + pen;
			}
			else if (mixer == 1)
			{
				const u8 five = pal3bit(5);
				const rgb_t t = rgb_t(target);
				to = rgb_t(0, rgb_t::clamp(t.r() + pen.r() - five)
					, rgb_t::clamp(t.g() + pen.g() - five)
					, rgb_t::clamp(t.b() + pen.b() - five));
			}
			prio |= 8 * is_prio_color;
		}

	};

	if (BIT(m_resolution, 1))
		draw_16(screen, bitmap, blendprio, cliprect, blend_op);
	else
		draw_256(screen, bitmap, blendprio, cliprect, blend_op);
}

void specnext_layer2_device::copyprio(screen_device &screen, bitmap_rgb32 &bitmap, bitmap_rgb32 &blendprio, const rectangle &cliprect)
{
	if (!m_layer2_en)
		return;

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		u8 *prio = &screen.priority().pix(y, cliprect.min_x);
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++, prio++)
		{
			if (prio[0] & 8)
				bitmap.pix(y, x) = blendprio.pix(y, x);
		}
	}
}

void specnext_layer2_device::draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, u8 pcode, u8 priority_mask)
{
	if (!m_layer2_en)
		return;

	const rgb_t gt0 = rgbexpand<3,3,3>((m_global_transparent << 1) | 0, 6, 3, 0);
	const rgb_t gt1 = rgbexpand<3,3,3>((m_global_transparent << 1) | 1, 6, 3, 0);
	auto blend_op = [gt0, gt1, pcode, priority_mask](u8 &prio, u32 &target, u32 &priotarget_ignore, const rgb_t pen, bool is_prio_color)
	{
		if ((prio & priority_mask) && !is_prio_color)
			return;

		const bool is_transparent = (pen == gt0) || (pen == gt1);
		if (!is_transparent)
		{
			target = pen;
			prio |= is_prio_color ? 8 : pcode;
		}
	};

	if (BIT(m_resolution, 1))
		draw_16(screen, bitmap, bitmap, cliprect, blend_op);
	else
		draw_256(screen, bitmap, bitmap, cliprect, blend_op);
}

template <typename FunctionClass>
void specnext_layer2_device::draw_256(screen_device &screen, bitmap_rgb32 &bitmap, bitmap_rgb32 &blendprio, const rectangle &cliprect, FunctionClass blend_op)
{
	const u8 res = m_resolution + 1;
	const u16 (&info)[5] = LAYER2_INFO[m_resolution];

	rectangle clip = rectangle{ m_clip_x1 << res, (std::min<u16>(m_clip_x2 + 1, info[0]) << res) - 1, m_clip_y1, std::min<u8>(m_clip_y2, info[1] - 1) };
	const u16 offset_h = m_offset_h - (info[2] << 1);
	const u16 offset_v = m_offset_v - info[2];
	clip.offset(offset_h, offset_v);
	clip &= cliprect;
	clip.setx(clip.left() & ~1, clip.right() | 1);

	do_draw(
			screen, bitmap, blendprio, clip, info, offset_h, offset_v,
			[this, &blend_op] (u16 pen_base, const u8 *scr, u32 *pix, u8 *prio, u32 *bprio, u16 &hpos, u16 &vpos)
			{
				const u16 idx = pen_base + (*scr);
				const rgb_t pen = palette().pen_color(idx);
				const bool is_prio_color = m_pen_priority[idx];
				blend_op(prio[0], pix[0], bprio[0], pen, is_prio_color);
				blend_op(prio[1], pix[1], bprio[1], pen, is_prio_color);
			});
}

template <typename FunctionClass>
void specnext_layer2_device::draw_16(screen_device &screen, bitmap_rgb32 &bitmap, bitmap_rgb32 &blendprio, const rectangle &cliprect, FunctionClass blend_op)
{
	const u16 (&info)[5] = LAYER2_INFO[1];

	rectangle clip = rectangle{ m_clip_x1 << 2, (std::min<u16>(m_clip_x2 + 1, info[0]) << 2) - 1, m_clip_y1, std::min<u8>(m_clip_y2, info[1] - 1) };

	const u16 offset_h = m_offset_h - (info[2] << 1);
	const u16 offset_v = m_offset_v - info[2];
	clip.offset(offset_h, offset_v);
	clip &= cliprect;

	do_draw(
			screen, bitmap, blendprio, clip, info, offset_h, offset_v,
			[this, &blend_op] (u16 pen_base, const u8 *scr, u32 *pix, u8 *prio, u32 *bprio, u16 &hpos, u16 &vpos)
			{
				if (hpos & 1)
					hpos ^= 1;
				else
				{
					const u16 idx = pen_base + (*scr >> 4);
					const rgb_t pen = palette().pen_color(idx);
					const bool is_prio_color = m_pen_priority[idx];
					blend_op(prio[0], pix[0], bprio[0], pen, is_prio_color);
				}

				{
					const u16 idx = pen_base + (*scr & 0x0f);
					const rgb_t pen = palette().pen_color(idx);
					const bool is_prio_color = m_pen_priority[idx];
					blend_op(prio[1], pix[1], bprio[1], pen, is_prio_color);
				}
			});
}

template <typename FunctionClass>
void specnext_layer2_device::do_draw(screen_device &screen, bitmap_rgb32 &bitmap, bitmap_rgb32 &blendprio, const rectangle &clip, const u16 (&info)[5], u16 offset_h, u16 offset_v, FunctionClass plot_op)
{
	if (clip.empty())
		return;

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
		u32 *bprio = &(blendprio.pix(vpos, clip.left()));
		for (u16 hpos = clip.left(); hpos <= clip.right(); hpos += 2, pix += 2, prio += 2, bprio += 2)
		{
			plot_op(pen_base, scr, pix, prio, bprio, hpos, vpos);

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
	save_item(NAME(m_offset_h));
	save_item(NAME(m_offset_v));
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
