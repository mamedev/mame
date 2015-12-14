// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "kvideodac.h"


const device_type KVIDEODAC = device_creator<kvideodac_device>;

kvideodac_device::kvideodac_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, KVIDEODAC, "KVIDEODAC Mixer", tag, owner, clock, "kvideodac", __FILE__)
{
	m_palette_tag = nullptr;
	m_palette = nullptr;
	memset(m_bitmaps, 0, sizeof(m_bitmaps));
}

kvideodac_device::~kvideodac_device()
{
	if(m_bitmaps[0])
		for(int i=0; i<2; i++)
			delete m_bitmaps[i];
}

void kvideodac_device::set_info(const char *tag, uint16_t shadow_mask, double shadow_level, uint16_t highlight_mask, double highlight_level)
{
	m_palette_tag = tag;
	m_shadow_mask = shadow_mask;
	m_shadow_level = shadow_level;
	m_highlight_mask = highlight_mask;
	m_highlight_level = highlight_level;
}

void kvideodac_device::device_start()
{
	m_palette = siblingdevice<palette_device>(m_palette_tag);

	m_init_cb.bind_relative_to(*owner());
	m_update_cb.bind_relative_to(*owner());

	save_item(NAME(m_shadow_level));
	save_item(NAME(m_highlight_level));
	save_item(NAME(m_force_shadow));
	save_item(NAME(m_force_highlight));

	generate_table(m_shadow_table, m_shadow_level);
	generate_table(m_highlight_table, m_highlight_level);
	generate_table(m_shadow_highlight_table, m_shadow_level*m_highlight_level);

	m_force_shadow = false;
	m_force_highlight = false;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void kvideodac_device::device_reset()
{
}

void kvideodac_device::set_force_shadow(bool force)
{
	m_force_shadow = force;
}

void kvideodac_device::set_force_highlight(bool force)
{
	m_force_highlight = force;
}

void kvideodac_device::set_shadow_level(double level)
{
	m_shadow_level = level;
	generate_table(m_shadow_table, m_shadow_level);
	generate_table(m_shadow_highlight_table, m_shadow_level*m_highlight_level);
}

void kvideodac_device::set_highlight_level(double level)
{
	m_highlight_level = level;
	generate_table(m_highlight_table, m_highlight_level);
	generate_table(m_shadow_highlight_table, m_shadow_level*m_highlight_level);
}

void kvideodac_device::generate_table(uint8_t *dest, double level)
{
	for(int i=0; i<256; i++) {
		int v = int(i*level+0.5);
		if(v > 255)
			v = 255;
		dest[i] = v;
	}
}

void kvideodac_device::bitmap_update(bitmap_rgb32 *bitmap, const rectangle &cliprect)
{
	if(!m_bitmaps[0] || m_bitmaps[0]->width() != bitmap->width() || m_bitmaps[0]->height() != bitmap->height()) {
		if(m_bitmaps[0])
			for(int i=0; i<2; i++)
				delete m_bitmaps[i];
		for(int i=0; i<2; i++)
			m_bitmaps[i] = new bitmap_ind16(bitmap->width(), bitmap->height());
		if(!m_init_cb.isnull())
			m_init_cb(m_bitmaps);
	}

	m_update_cb(m_bitmaps, cliprect);
	const uint32_t *pens = m_palette->pens();
	uint16_t mask = m_palette->entries() - 1;
	for(int y = cliprect.min_y; y <= cliprect.max_y; y++) {
		const uint16_t *bcolor = &m_bitmaps[BITMAP_COLOR]->pix16(y, cliprect.min_x);
		const uint16_t *battr  = &m_bitmaps[BITMAP_ATTRIBUTES]->pix16(y, cliprect.min_x);
		uint32_t *dest = &bitmap->pix32(y, cliprect.min_x);
		for(int x = cliprect.min_x; x <= cliprect.max_x; x++) {
			uint16_t bc = *bcolor++;
			uint16_t ba = *battr++;

			uint32_t col = pens[bc & mask];

			if(m_force_shadow || m_force_highlight || (ba & (m_shadow_mask|m_highlight_mask))) {
				const uint8_t *table;
				if(m_force_shadow || (ba & m_shadow_mask))
					if(m_force_highlight || (ba & m_highlight_mask))
						table = m_shadow_highlight_table;
					else
						table = m_shadow_table;
				else
					table = m_highlight_table;
				col = (table[(col >> 16) & 0xff] << 16) |  (table[(col >> 8) & 0xff] << 8) |  (table[col & 0xff]);
			}
			*dest++ = col;
		}
	}
}

uint32_t kvideodac_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap_update(&bitmap, cliprect);
	return 0;
}

