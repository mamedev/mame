// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/zaccaria.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.


Here's the hookup from the proms (82s131) to the r-g-b-outputs

     Prom 9F        74LS374
    -----------   ____________
       12         |  3   2   |---680 ohm----| blue out
       11         |  4   5   |---1k ohm-----| (+ 470 ohm pulldown)
       10         |  7   6   |---820 ohm-------|
        9         |  8   9   |---1k ohm--------| green out
     Prom 9G      |          |                 | (+ 390 ohm pulldown)
       12         |  13  12  |---1.2k ohm------|
       11         |  14  15  |---820 ohm----------|
       10         |  17  16  |---1k ohm-----------| red out
        9         |  18  19  |---1.2k ohm---------| (+ 390 ohm pulldown)
                  |__________|


***************************************************************************/
PALETTE_INIT_MEMBER(zaccaria_state, zaccaria)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i, j, k;
	static const int resistances_rg[] = { 1200, 1000, 820 };
	static const int resistances_b[]  = { 1000, 820 };

	double weights_rg[3], weights_b[2];

	compute_resistor_weights(0, 0xff, -1.0,
								3, resistances_rg, weights_rg, 390, 0,
								2, resistances_b,  weights_b,  470, 0,
								0, 0, 0, 0, 0);

	for (i = 0; i < 0x200; i++)
	{
		/*
		  TODO: I'm not sure, but I think that pen 0 must always be black, otherwise
		  there's some junk brown background in Jack Rabbit.
		  From the schematics it seems that the background color can be changed, but
		  I'm not sure where it would be taken from; I think the high bits of
		  attributesram, but they are always 0 in these games so they would turn out
		  black anyway.
		 */
		if (((i % 64) / 8) == 0)
			palette.set_indirect_color(i, rgb_t::black);
		else
		{
			int bit0, bit1, bit2;
			int r, g, b;

			/* red component */
			bit0 = (color_prom[i + 0x000] >> 3) & 0x01;
			bit1 = (color_prom[i + 0x000] >> 2) & 0x01;
			bit2 = (color_prom[i + 0x000] >> 1) & 0x01;
			r = combine_3_weights(weights_rg, bit0, bit1, bit2);

			/* green component */
			bit0 = (color_prom[i + 0x000] >> 0) & 0x01;
			bit1 = (color_prom[i + 0x200] >> 3) & 0x01;
			bit2 = (color_prom[i + 0x200] >> 2) & 0x01;
			g = combine_3_weights(weights_rg, bit0, bit1, bit2);

			/* blue component */
			bit0 = (color_prom[i + 0x200] >> 1) & 0x01;
			bit1 = (color_prom[i + 0x200] >> 0) & 0x01;
			b = combine_2_weights(weights_b, bit0, bit1);

			palette.set_indirect_color(i, rgb_t(r, g, b));
		}
	}

	/* There are 512 unique colors, which seem to be organized in 8 blocks */
	/* of 64. In each block, colors are not in the usual sequential order */
	/* but in interleaved order, like Phoenix. Additionally, colors for */
	/* background and sprites are interleaved. */
	for (i = 0;i < 8;i++)
		for (j = 0;j < 4;j++)
			for (k = 0;k < 8;k++)
				/* swap j and k to make the colors sequential */
				palette.set_pen_indirect(0 + 32 * i + 8 * j + k, 64 * i + 8 * k + 2*j);

	for (i = 0;i < 8;i++)
		for (j = 0;j < 4;j++)
			for (k = 0;k < 8;k++)
				/* swap j and k to make the colors sequential */
				palette.set_pen_indirect(256 + 32 * i + 8 * j + k, 64 * i + 8 * k + 2*j+1);
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(zaccaria_state::get_tile_info)
{
	UINT8 attr = m_videoram[tile_index + 0x400];
	SET_TILE_INFO_MEMBER(0,
			m_videoram[tile_index] + ((attr & 0x03) << 8),
			((attr & 0x0c) >> 2) + ((m_attributesram[2 * (tile_index % 32) + 1] & 0x07) << 2),
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void zaccaria_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(zaccaria_state::get_tile_info),this),TILEMAP_SCAN_ROWS,8,8,32,32);

	m_bg_tilemap->set_scroll_cols(32);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(zaccaria_state::videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(zaccaria_state::attributes_w)
{
	if (offset & 1)
	{
		if (m_attributesram[offset] != data)
		{
			int i;

			for (i = offset / 2;i < 0x400;i += 32)
				m_bg_tilemap->mark_tile_dirty(i);
		}
	}
	else
		m_bg_tilemap->set_scrolly(offset / 2,data);

	m_attributesram[offset] = data;
}

WRITE8_MEMBER(zaccaria_state::flip_screen_x_w)
{
	flip_screen_x_set(data & 1);
}

WRITE8_MEMBER(zaccaria_state::flip_screen_y_w)
{
	flip_screen_y_set(data & 1);
}



/***************************************************************************

  Display refresh

***************************************************************************/

/* sprite format:

  76543210
0 xxxxxxxx x
1 x....... flipy
  .x...... flipx
  ..xxxxxx code low
2 xx...... code high
  ..xxx... ?
  .....xxx color
3 xxxxxxxx y

offsets 1 and 2 are swapped if accessed from spriteram2

*/
void zaccaria_state::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect,UINT8 *spriteram,int color,int section)
{
	int offs,o1 = 1,o2 = 2;

	if (section)
	{
		o1 = 2;
		o2 = 1;
	}

	for (offs = 0;offs < 0x20;offs += 4)
	{
		int sx = spriteram[offs + 3] + 1;
		int sy = 242 - spriteram[offs];
		int flipx = spriteram[offs + o1] & 0x40;
		int flipy = spriteram[offs + o1] & 0x80;

		if (sx == 1) continue;

		if (flip_screen_x())
		{
			sx = 240 - sx;
			flipx = !flipx;
		}
		if (flip_screen_y())
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				(spriteram[offs + o1] & 0x3f) + (spriteram[offs + o2] & 0xc0),
				((spriteram[offs + o2] & 0x07) << 2) | color,
				flipx,flipy,sx,sy,0);
	}
}

UINT32 zaccaria_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);

	// 3 layers of sprites, each with their own palette and priorities
	// Not perfect yet, does spriteram(1) layer have a priority bit somewhere?
	draw_sprites(bitmap,cliprect,m_spriteram2,2,1);
	draw_sprites(bitmap,cliprect,m_spriteram,1,0);
	draw_sprites(bitmap,cliprect,m_spriteram2+0x20,0,1);

	return 0;
}
