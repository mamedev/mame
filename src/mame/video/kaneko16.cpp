// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
/***************************************************************************

                            -= Kaneko 16 Bit Games =-

                    driver by   Luca Elia (l.elia@tin.it)

**************************************************************************/

#include "emu.h"
#include "includes/kaneko16.h"
#include "screen.h"


void kaneko16_state::display_enable_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_disp_enable);
}

void kaneko16_state::video_start()
{
	m_disp_enable = 1;  // default enabled for games not using it
	save_item(NAME(m_disp_enable));
}


/* Fill the bitmap with a single colour. This is wrong, but will work most of
   the times. To do it right, each pixel should be drawn with pen 0
   of the bottomost tile that covers it (which is pretty tricky to do) */
template<class _BitmapClass>
void kaneko16_state::fill_bitmap(_BitmapClass &bitmap, const rectangle &cliprect)
{
	int pen = 0;

	if (m_kaneko_spr.found())
	{
		if (m_kaneko_spr->get_sprite_type() == 1)
		{
			pen = 0x7f00;
		}
	}

	typename _BitmapClass::pixel_t *dest;
	(void)dest; // shut up Visual Studio
	if (sizeof(*dest) == 2)
	{
		bitmap.fill(pen, cliprect);
	}
	else
	{
		const pen_t *pal = m_palette->pens();
		bitmap.fill(pal[pen], cliprect);
	}
}


template<class _BitmapClass>
u32 kaneko16_state::screen_update_common(screen_device &screen, _BitmapClass &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	if (m_view2[0].found()) m_view2[0]->prepare(bitmap, cliprect);
	if (m_view2[1].found()) m_view2[1]->prepare(bitmap, cliprect);

	for (int i = 0; i < 8; i++)
	{
		if (m_view2[0].found())
			m_view2[0]->render_tilemap(screen,bitmap,cliprect,i);
		if (m_view2[1].found())
			m_view2[1]->render_tilemap_alt(screen,bitmap,cliprect,i, m_VIEW2_2_pri);
	}

	return 0;
}


u32 kaneko16_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	fill_bitmap(bitmap,cliprect);

	// if the display is disabled, do nothing?
	if (!m_disp_enable) return 0;

	screen_update_common(screen, bitmap, cliprect);
	m_kaneko_spr->render_sprites(bitmap,cliprect, screen.priority(), m_spriteram->buffer(), m_spriteram->bytes());
	return 0;
}


/* berlwall and have an additional hi-color layers */

void kaneko16_berlwall_state::video_start()
{
	m_bg15_bright = 0xff;
	u8 *RAM  =   memregion("gfx3")->base();

	/* Render the hi-color static backgrounds held in the ROMs */

	for (auto & elem : m_bg15_bitmap)
		elem.allocate(256, 256);

/*
    8aba is used as background color
    8aba/2 = 455d = 10001 01010 11101 = $11 $0a $1d (G5R5B5)
*/

	for (int screen = 0; screen < 32; screen++)
	{
		for (int x = 0; x < 256; x++)
		{
			for (int y = 0; y < 256; y++)
			{
				int addr  = screen * (256 * 256) + x + y * 256;
				int data = RAM[addr * 2 + 0] * 256 + RAM[addr * 2 + 1];

				int r = (data & 0x07c0) >>  6;
				int g = (data & 0xf800) >> 11;
				int b = (data & 0x003e) >>  1;

				/* apply a simple decryption */
				r ^= 0x09;

				if (~g & 0x08) g ^= 0x10;
				g = (g - 1) & 0x1f;     /* decrease with wraparound */

				b ^= 0x03;
				if (~b & 0x08) b ^= 0x10;
				b = (b + 2) & 0x1f;     /* increase with wraparound */

				/* kludge to fix the rollercoaster picture */
				if ((r & 0x10) && (b & 0x10))
				g = (g - 1) & 0x1f;     /* decrease with wraparound */

				m_bg15_bitmap[screen].pix16(y, x) = ((g << 10) | (r << 5) | b) & 0x7fff;
			}
		}
	}

	kaneko16_state::video_start();
	save_item(NAME(m_bg15_select));
	save_item(NAME(m_bg15_bright));
}


/* Select the high color background image (out of 32 in the ROMs) */
u8 kaneko16_berlwall_state::bg15_select_r()
{
	return m_bg15_select;
}
void kaneko16_berlwall_state::bg15_select_w(u8 data)
{
	m_bg15_select = data;
}

/* Brightness (00-ff) */
u8 kaneko16_berlwall_state::bg15_bright_r()
{
	return m_bg15_bright;
}
void kaneko16_berlwall_state::bg15_bright_w(u8 data)
{
	if (m_bg15_bright != data)
	{
		m_bg15_bright = data;
		double brt1 = m_bg15_bright & 0xff;
		brt1 = brt1 / 255.0;

		for (int i = 0; i < 0x8000; i++)
			m_bgpalette->set_pen_contrast(i, brt1);
	}
}


void kaneko16_berlwall_state::render_15bpp_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!m_bg15_bitmap[0].valid())
		return;

	int select  =   m_bg15_select;
	int scroll  =   m_bg15_scroll[ 0 ];

	int screen  =   select & 0x1f;
	int flip    =   select & 0x20;

	int scrollx =   (scroll >> 0) & 0xff;
	int scrolly =   (scroll >> 8) & 0xff;

	if (!flip)
	{
		scrollx -= 0x80;
		scrolly -= 0x08;
	}
	else
	{
		scrollx -= 0xff - 0x80;
		scrolly -= 0xff - 0x08;
	}

	const pen_t *pal = m_bgpalette->pens();
	u16* srcbitmap;
	u32* dstbitmap;

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		if (!flip)  srcbitmap = &m_bg15_bitmap[screen].pix16(        (y - scrolly) & 0xff  );
		else        srcbitmap = &m_bg15_bitmap[screen].pix16( 255 - ((y - scrolly) & 0xff) );

		dstbitmap = &bitmap.pix32(y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			u16 pix;

			if (!flip)  pix = srcbitmap[        (x - scrollx) & 0xff  ];
			else        pix = srcbitmap[ 255 - ((x - scrollx) & 0xff) ];

			dstbitmap[x] = pal[pix];
		}
	}
}

u32 kaneko16_berlwall_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// berlwall uses a 15bpp bitmap as a bg, not a solid fill
	render_15bpp_bitmap(bitmap,cliprect);

	// if the display is disabled, do nothing?
	if (!m_disp_enable) return 0;

	screen_update_common(screen, bitmap, cliprect);
	m_kaneko_spr->render_sprites(bitmap,cliprect, screen.priority(), m_spriteram->buffer(), m_spriteram->bytes()/2);
	return 0;
}
