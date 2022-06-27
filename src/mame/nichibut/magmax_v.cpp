// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/***************************************************************************

Video Hardware for MAGMAX.

Driver by Takahiro Nogi (nogi@kt.rim.or.jp) 1999/11/05 -
Additional tweaking by Jarek Burczynski

***************************************************************************/

#include "emu.h"
#include "includes/magmax.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Mag Max has three 256x4 palette PROMs (one per gun), connected to the
  RGB output this way:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/
void magmax_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	// create a lookup table for the palette
	for (int i = 0; i < 0x100; i++)
	{
		int const r = pal4bit(color_prom[i + 0x000]);
		int const g = pal4bit(color_prom[i + 0x100]);
		int const b = pal4bit(color_prom[i + 0x200]);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x300;

	// characters use colors 0-0x0f
	for (int i = 0; i < 0x10; i++)
		palette.set_pen_indirect(i, i);

	// sprites use colors 0x10-0x1f, color 0x1f being transparent
	for (int i = 0; i < 0x100; i++)
	{
		uint8_t const ctabentry = (color_prom[i] & 0x0f) | 0x10;
		palette.set_pen_indirect(i + 0x10, ctabentry);
	}

	// background uses all colors (no lookup table)
	for (int i = 0; i < 0x100; i++)
		palette.set_pen_indirect(i + 0x110, i);

}

void magmax_state::video_start()
{
	uint8_t * prom14D = memregion("user2")->base();

	// Set up save state
	save_item(NAME(m_flipscreen));

	m_prom_tab = std::make_unique<uint32_t[]>(256);

	m_screen->register_screen_bitmap(m_bitmap);

	// Allocate temporary bitmap
	for (int i = 0; i < 256; i++)
	{
		int v = (prom14D[i] << 4) + prom14D[i + 0x100];
		m_prom_tab[i] = ((v&0x1f)<<8) | ((v&0x10)<<10) | ((v&0xe0)>>1); /*convert data into more useful format*/
	}
}



uint32_t magmax_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// bit 2 flip screen
	m_flipscreen = *m_vreg & 0x04;

	// copy the background graphics
	if (*m_vreg & 0x40)     // background disable
		bitmap.fill(0, cliprect);
	else
	{
		uint32_t scroll_h = (*m_scroll_x) & 0x3fff;
		uint32_t scroll_v = (*m_scroll_y) & 0xff;

		// clear background-over-sprites bitmap
		m_bitmap.fill(0);

		for (int v = 2*8; v < 30*8; v++) // only for visible area
		{
			uint16_t line_data[256];

			uint32_t map_v_scr_100 =   (scroll_v + v) & 0x100;
			uint32_t rom18D_addr   =  ((scroll_v + v) & 0xf8)     + (map_v_scr_100<<5);
			uint32_t rom15F_addr   = (((scroll_v + v) & 0x07)<<2) + (map_v_scr_100<<5);
			uint32_t map_v_scr_1fe_6 =((scroll_v + v) & 0x1fe)<<6;

			pen_t pen_base = 0x110 + 0x20 + (map_v_scr_100>>1);

			for (int h = 0; h < 0x100; h++)
			{
				uint32_t LS283 = scroll_h + h;

				if (!map_v_scr_100)
				{
					if (h & 0x80)
						LS283 = LS283 + (m_rom18B[ map_v_scr_1fe_6 + (h ^ 0xff) ] ^ 0xff);
					else
						LS283 = LS283 + m_rom18B[ map_v_scr_1fe_6 + h ] + 0xff01;
				}

				uint32_t prom_data = m_prom_tab[ (LS283 >> 6) & 0xff ];

				rom18D_addr &= 0x20f8;
				rom18D_addr += (prom_data & 0x1f00) + ((LS283 & 0x38) >>3);

				rom15F_addr &= 0x201c;
				rom15F_addr += (m_rom18B[0x4000 + rom18D_addr ]<<5) + ((LS283 & 0x6)>>1);
				rom15F_addr += (prom_data & 0x4000);

				uint32_t graph_color = (prom_data & 0x0070);

				uint32_t graph_data = m_rom18B[0x8000 + rom15F_addr];
				if ((LS283 & 1))
					graph_data >>= 4;
				graph_data &= 0x0f;

				line_data[h] = pen_base + graph_color + graph_data;

				// priority: background over sprites
				if (map_v_scr_100 && ((graph_data & 0x0c)==0x0c))
					m_bitmap.pix(v, h) = line_data[h];
			}

			if (m_flipscreen)
			{
				uint16_t line_data_flip_x[256];
				for (int i = 0; i < 256; i++)
					line_data_flip_x[i] = line_data[255-i];
				draw_scanline16(bitmap, 0, 255-v, 256, line_data_flip_x, nullptr);
			}
			else
				draw_scanline16(bitmap, 0, v, 256, line_data, nullptr);
		}
	}

	// draw the sprites
	for (int offs = 0; offs < m_spriteram.bytes()/2; offs += 4)
	{
		int sy = m_spriteram[offs] & 0xff;

		if (sy)
		{
			int code = m_spriteram[offs + 1] & 0xff;
			int attr = m_spriteram[offs + 2] & 0xff;
			int color = (attr & 0xf0) >> 4;
			int flipx = attr & 0x04;
			int flipy = attr & 0x08;

			int sx = (m_spriteram[offs + 3] & 0xff) - 0x80 + 0x100 * (attr & 0x01);
			sy = 239 - sy;

			if (m_flipscreen)
			{
				sx = 255-16 - sx;
				sy = 239 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			if (code & 0x80)    // sprite bankswitch
				code += (*m_vreg & 0x30) * 0x8;

			m_gfxdecode->gfx(1)->transmask(bitmap, cliprect,
					code,
					color,
					flipx, flipy,
					sx, sy,
					m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 0x1f));
		}
	}

	if (!(*m_vreg & 0x40))      // background disable
		copybitmap_trans(bitmap, m_bitmap, m_flipscreen, m_flipscreen, 0, 0, cliprect, 0);

	// draw the foreground characters
	for (int offs = 32*32-1; offs >= 0; offs -= 1)
	{
		//int page = (*m_vreg>>3) & 0x1;
		int code = m_videoram[offs /*+ page*/] & 0xff;

		if (code)
		{
			int sx = (offs % 32);
			int sy = (offs / 32);

			if (m_flipscreen)
			{
				sx = 31 - sx;
				sy = 31 - sy;
			}

			m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
					code,
					0,
					m_flipscreen, m_flipscreen,
					8 * sx, 8 * sy, 0x0f);
		}
	}
	return 0;
}
