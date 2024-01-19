// license:BSD-3-Clause
/**********************************************************************

	Spectrum Next Layer2

TODO:
* clipping
* scroll
* resolution != 0

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

void specnext_layer2_device::draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!m_layer2_en || m_resolution != 0b00)
		return;

	const rgb_t gt0 = rgbexpand<3,3,3>((m_global_transparent << 1) | 0, 6, 3, 0);
	const rgb_t gt1 = rgbexpand<3,3,3>((m_global_transparent << 1) | 1, 6, 3, 0);
	const u16 pen_base = (m_layer2_palette_select ? m_palette_alt_offset : m_palette_base_offset) | (m_palette_offset << 4);
	for (u16 vpos = cliprect.top(); vpos <= cliprect.bottom(); vpos++)
	{
		const u8 y = vpos - m_offset_v;
		const u8 x = cliprect.left() - m_offset_h;
		const u8 *scr = m_host_ram_ptr + (m_layer2_active_bank << 14) + (y * 256) + x;
		u16 *pix = &(bitmap.pix(vpos, cliprect.left()));
		for (u16 hpos = cliprect.left(); hpos <= cliprect.right(); hpos++, pix++, scr++)
		{
			const u16 pen = pen_base + *scr;
			if (palette().pen_color(pen) != gt0 && palette().pen_color(pen) != gt1)
				*pix = pen;
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
