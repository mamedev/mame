// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

    Seta (or Allumer) X1-012 (+ X1-011) tile layer generator

    Each layer consists of 2 tilemaps: only one can be displayed
    at any given time.

***************************************************************************/

#include "emu.h"
#include "x1_012.h"


DEFINE_DEVICE_TYPE(X1_012, x1_012_device, "x1_012", "Seta X1-012 Tile Layer")

x1_012_device::x1_012_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, X1_012, tag, owner, clock)
	, device_gfx_interface(mconfig, *this)
	, m_tile_offset_callback(*this)
	, m_vram(*this, DEVICE_SELF)
	, m_tilemap(nullptr)
	, m_xoffsets{0, 0}
	, m_rambank(0)
{
	std::fill(std::begin(m_vctrl), std::end(m_vctrl), 0);
}

TILE_GET_INFO_MEMBER(x1_012_device::get_tile_info)
{
	const u16 *const vram = &m_vram[m_rambank ? 0x1000 : 0];
	u16 code = vram[tile_index] & 0x3fff;
	const u16 flip = TILE_FLIPXY((vram[tile_index] & 0xc000) >> 14);
	const u16 attr = vram[tile_index + 0x800];

	int gfx = (m_vctrl[2] & 0x10) >> 4;
	if (gfx == 1 && this->gfx(1) == nullptr)
	{
		popmessage("Missing Color Mode = 1 for Layer = %s. Contact MAMETesters.", tag());
		gfx = 0;
	}

	if (!m_tile_offset_callback.isnull())
		code = m_tile_offset_callback(code);

	tileinfo.set(gfx, code, attr & 0x1f, flip);
}

void x1_012_device::device_resolve_objects()
{
	m_tile_offset_callback.resolve();
}

void x1_012_device::device_start()
{
	m_tilemap = &machine().tilemap().create(
			*this, tilemap_get_info_delegate(*this, FUNC(x1_012_device::get_tile_info)), TILEMAP_SCAN_ROWS,
			16, 16, 64, 32);
	m_tilemap->set_transparent_pen(0);

	save_item(NAME(m_vctrl));
	save_item(NAME(m_rambank));
}

void x1_012_device::vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vram[offset]);
	if (m_rambank == ((offset >> 12) & 1))
		m_tilemap->mark_tile_dirty(offset & 0x7ff);
}

u16 x1_012_device::vctrl_r(offs_t offset, u16 mem_mask)
{
	return m_vctrl[offset];
}

void x1_012_device::vctrl_w(offs_t offset, u16 data, u16 mem_mask)
{
	// Select tilemap bank. Only one tilemap bank per layer is enabled.
	if (offset == 2 && ACCESSING_BITS_0_7)
	{
		if ((data & 0x0008) != (m_vctrl[2] & 0x0008))
		{
			m_rambank = (data & 0x0008) ? 1 : 0;
			m_tilemap->mark_all_dirty();
		}

		// Check tilemap color mode
		if ((data & 0x0010) != (m_vctrl[2] & 0x0010))
			m_tilemap->mark_all_dirty();
	}

	COMBINE_DATA(&m_vctrl[offset]);
}


void x1_012_device::update_scroll(int vis_dimy, bool flip)
{
	int x = m_vctrl[0];
	int y = m_vctrl[1];

	/* the hardware wants different scroll values when flipped */

	/*  bg x scroll      flip
	    metafox     0000 025d = 0, $400-$1a3 = $400 - $190 - $13
	    eightfrc    ffe8 0272
	                fff0 0260 = -$10, $400-$190 -$10
	                ffe8 0272 = -$18, $400-$190 -$18 + $1a      */

	x += 0x10 - m_xoffsets[flip ? 1 : 0];
	y -= (256 - vis_dimy)/2;
	if (flip)
	{
		x = -x - 512;
		y = y - vis_dimy;
	}

	m_tilemap->set_scrollx(0, x);
	m_tilemap->set_scrolly(0, y);
}


void x1_012_device::draw(screen_device &screen, bitmap_ind16 &dest, const rectangle &cliprect, u32 flags, u8 priority, u8 priority_mask)
{
	m_tilemap->draw(screen, dest, cliprect, flags, priority, priority_mask);
}

void x1_012_device::draw_tilemap_palette_effect(bitmap_ind16 &bitmap, const rectangle &cliprect, bool flipscreen)
{
	const int scrollx = m_tilemap->scrollx(0);
	const int scrolly = m_tilemap->scrolly(0);
	const gfx_element *const gfx_tilemap = gfx((m_vctrl[2] & 0x10) >> 4);
	const bitmap_ind16 &src_bitmap = m_tilemap->pixmap();
	const int opaque_mask = gfx_tilemap->granularity() - 1;
	const int pixel_effect_mask = gfx_tilemap->colorbase() + (gfx_tilemap->colors() - 1) * gfx_tilemap->granularity();

	const int width_mask = src_bitmap.width() - 1;
	const int height_mask = src_bitmap.height() - 1;

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		u16 *const dest = &bitmap.pix(y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			int p;
			if (!flipscreen)
			{
				p = src_bitmap.pix((y + scrolly) & height_mask, (x + scrollx) & width_mask);
			}
			else
			{
				p = src_bitmap.pix((y - scrolly - 256) & height_mask, (x - scrollx - 512) & width_mask);
			}

			// draw not transparent pixels
			if (p & opaque_mask)
			{
				// pixels with the last color are not drawn and the 2nd palette is added to the current bitmap color
				if ((p & pixel_effect_mask) == pixel_effect_mask)
				{
					dest[x] = palette().entries() / 2 + dest[x];
				}
				else
				{
					dest[x] = palette().pen(p);
				}
			}
		}
	}
}

