// license:BSD-3-Clause
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

static const u16 layer2_info[3][5] =
{
	//width  height  border  x-inc  y-inc
	{   256,    192,      0,     1,   256 },
	{   320,    256,     32,   256,     1 },
};

void specnext_layer2_device::draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!m_layer2_en)
		return;

	const u16 (&info)[5] = layer2_info[std::min<u8>(m_resolution, 1)];
	const u8 scale = (m_resolution >= 2) ? 0 : 1; // hires

	u16 offset_h = m_offset_h - (info[2] << 1);
	u16 offset_v = m_offset_v - info[2];
	rectangle clip = rectangle{ m_clip_x1 << 1, (std::min<u16>(m_clip_x2, info[0] - 1) << 1) | 1, m_clip_y1, std::min<u8>(m_clip_y2, info[1] - 1) };
	if (info[2])
		clip.setx(clip.left() << 1, (clip.right() << 1) | 1);

	clip.offset(offset_h, offset_v);
	clip &= cliprect;
	//clip.setx(clip.left() + (clip.left() & 1), clip.right() | 1);
	clip.setx(clip.left() & ~1, clip.right() | 1);

	if (clip.empty())
		return;

	const rgb_t gt0 = rgbexpand<3,3,3>((m_global_transparent << 1) | 0, 6, 3, 0);
	const rgb_t gt1 = rgbexpand<3,3,3>((m_global_transparent << 1) | 1, 6, 3, 0);
	const u16 pen_base = (m_layer2_palette_select ? m_palette_alt_offset : m_palette_base_offset) | (m_palette_offset << 4);
	const u16 x_min = (((clip.left() - offset_h) >> 1) + info[0] + m_scroll_x) % info[0];
	for (u16 vpos = clip.top(); vpos <= clip.bottom(); vpos++)
	{
		const u16 y = (vpos - offset_v + info[1] + m_scroll_y) % info[1];
		u16 x = x_min ;
		const u8 *scr = m_host_ram_ptr + (m_layer2_active_bank << 14) + (y * info[4]) + (x * info[3]);
		u32 *pix = &(bitmap.pix(vpos, clip.left()));
		for (u16 hpos = clip.left(); hpos <= clip.right(); hpos += 2, pix += 2)
		{
			if (scale)
			{
				const rgb_t pen = palette().pen_color(pen_base + (*scr));
				if (pen != gt0 && pen != gt1)
				{
					*pix = pen;
					*(pix + 1) = pen;
				}
			}
			else
			{
				rgb_t pen = palette().pen_color(pen_base + (*scr >> 4));
				if (pen != gt0 && pen != gt1)
					*pix = pen;
				pen = palette().pen_color(pen_base + (*scr & 0x0f));
				if (pen != gt0 && pen != gt1)
					*(pix + 1) = pen;
			}

			++x %= info[0];
			if (x == 0)
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
	save_item(NAME(m_layer2_palette_select));

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

// device type definition
DEFINE_DEVICE_TYPE(SPECNEXT_LAYER2, specnext_layer2_device, "layer2", "Spectrum Next Layer2")
