// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, Ernesto Corvi, Nicola Salmoria
/***************************************************************************

  gaplus.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/gaplus.h"


/***************************************************************************

  Convert the color PROMs.

  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

void gaplus_base_state::gaplus_palette(palette_device &palette) const
{
	const uint8_t *color_prom = m_proms_region->base();
	for (int i = 0; i < 256; i++)
	{
		int bit0, bit1, bit2, bit3;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		bit3 = BIT(color_prom[i], 3);
		int const r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		// green component
		bit0 = BIT(color_prom[i + 0x100], 0);
		bit1 = BIT(color_prom[i + 0x100], 1);
		bit2 = BIT(color_prom[i + 0x100], 2);
		bit3 = BIT(color_prom[i + 0x100], 3);
		int const g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		// blue component
		bit0 = BIT(color_prom[i + 0x200], 0);
		bit1 = BIT(color_prom[i + 0x200], 1);
		bit2 = BIT(color_prom[i + 0x200], 2);
		bit3 = BIT(color_prom[i + 0x200], 3);
		int const b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	color_prom += 0x300;
	// color_prom now points to the beginning of the lookup table

	// characters use colors 0xf0-0xff
	for (int i = 0; i < m_gfxdecode->gfx(0)->colors() * m_gfxdecode->gfx(0)->granularity(); i++)
		palette.set_pen_indirect(m_gfxdecode->gfx(0)->colorbase() + i, 0xf0 + (*color_prom++ & 0x0f));

	/* sprites */
	for (int i = 0; i < m_gfxdecode->gfx(1)->colors() * m_gfxdecode->gfx(1)->granularity(); i++)
	{
		palette.set_pen_indirect(m_gfxdecode->gfx(1)->colorbase() + i, (color_prom[0] & 0x0f) + ((color_prom[0x200] & 0x0f) << 4));
		color_prom++;
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/* convert from 32x32 to 36x28 */
TILEMAP_MAPPER_MEMBER(gaplus_base_state::tilemap_scan)
{
	row += 2;
	col -= 2;
	if (col & 0x20)
		return row + ((col & 0x1f) << 5);
	return col + (row << 5);
}

TILE_GET_INFO_MEMBER(gaplus_base_state::get_tile_info)
{
	const uint8_t attr = m_videoram[tile_index + 0x400];
	tileinfo.category = (attr & 0x40) >> 6;
	tileinfo.group = attr & 0x3f;
	SET_TILE_INFO_MEMBER(0,
			m_videoram[tile_index] + ((attr & 0x80) << 1),
			attr & 0x3f,
			0);
}



/***************************************************************************
    Starfield information
    There's 3 sets of stars planes at different speeds.

    a000 ---> (bit 0 = 1) enable starfield.
              (bit 0 = 0) disable starfield.
    a001 ---> starfield plane 0 control
    a002 ---> starfield plane 1 control
    a003 ---> starfield plane 2 control
***************************************************************************/

/* starfield speed constants (bigger = faster) */
#define SPEED_1 0.5f
#define SPEED_2 1.0f
#define SPEED_3 2.0f

void gaplus_base_state::starfield_init()
{
	int generator = 0;
	int set = 0;

	const int width = m_screen->width();
	const int height = m_screen->height();

	m_total_stars = 0;

	/* precalculate the star background */
	/* this comes from the Galaxian hardware, Gaplus is probably different */

	for (int y = 0; y < height; y++)
	{
		for (int x = width * 2 - 1; x >= 0; x--)
		{
			generator <<= 1;
			const int bit1 = (~generator >> 17) & 1;
			const int bit2 = (generator >> 5) & 1;

			if (bit1 ^ bit2) generator |= 1;

			if (BIT(~generator, 16) && (generator & 0xff) == 0xff)
			{
				const int color = ~(generator >> 8) & 0x3f;
				if (color && m_total_stars < MAX_STARS)
				{
					m_stars[m_total_stars].x = x;
					m_stars[m_total_stars].y = y;
					m_stars[m_total_stars].col = color;
					m_stars[m_total_stars].set = set++;

					if (set == 3)
						set = 0;

					m_total_stars++;
				}
			}
		}
	}
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void gaplus_base_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(gaplus_state::get_tile_info),this),tilemap_mapper_delegate(FUNC(gaplus_state::tilemap_scan),this),8,8,36,28);

	m_bg_tilemap->configure_groups(*m_gfxdecode->gfx(0), 0xff);

	starfield_init();

	save_item(NAME(m_starfield_control));

	for (int i = 0; i < MAX_STARS; i++)
	{
		save_item(NAME(m_stars[i].x), i);
		save_item(NAME(m_stars[i].y), i);
		// col and set aren't changed after init
	}
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(gaplus_base_state::videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(gaplus_base_state::starfield_control_w)
{
	m_starfield_control[offset & 3] = data;
}



/***************************************************************************

    Display Refresh

***************************************************************************/

void gaplus_base_state::starfield_render(bitmap_ind16 &bitmap)
{
	/* check if we're running */
	if ((m_starfield_control[0] & 1) == 0)
		return;

	const int width = m_screen->width();
	const int height = m_screen->height();

	/* draw the starfields */
	for (int i = 0; i < m_total_stars; i++)
	{
		int x = m_stars[i].x;
		int y = m_stars[i].y;

		if (x >= 0 && x < width && y >= 0 && y < height)
		{
			bitmap.pix16(y, x) = m_stars[i].col;
		}
	}
}

void gaplus_base_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect ) const
{
	uint8_t *spriteram = m_spriteram + 0x780;
	uint8_t *spriteram_2 = spriteram + 0x800;
	uint8_t *spriteram_3 = spriteram_2 + 0x800;

	for (int offs = 0;offs < 0x80;offs += 2)
	{
		/* is it on? */
		if ((spriteram_3[offs+1] & 2) == 0)
		{
			static const int gfx_offs[2][2] =
			{
				{ 0, 1 },
				{ 2, 3 }
			};
			const int sprite = spriteram[offs] | ((spriteram_3[offs] & 0x40) << 2);
			const int color = spriteram[offs+1] & 0x3f;
			const int sx = spriteram_2[offs+1] + 0x100 * (spriteram_3[offs+1] & 1) - 71;
			int sy = 256 - spriteram_2[offs] - 8;
			int flipx = BIT(spriteram_3[offs], 0);
			int flipy = BIT(spriteram_3[offs], 1);
			const int sizex = BIT(spriteram_3[offs], 3);
			const int sizey = BIT(spriteram_3[offs], 5);
			const int duplicate = spriteram_3[offs] & 0x80;

			if (flip_screen())
			{
				flipx ^= 1;
				flipy ^= 1;
			}

			sy -= 16 * sizey;
			sy = (sy & 0xff) - 32;  // fix wraparound

			for (int y = 0;y  <= sizey; y++)
			{
				for (int x = 0; x <= sizex; x++)
				{
					m_gfxdecode->gfx(1)->transmask(bitmap, cliprect,
						sprite + (duplicate ? 0 : (gfx_offs[y ^ (sizey * flipy)][x ^ (sizex * flipx)])),
						color,
						flipx,flipy,
						sx + 16 * x, sy + 16 * y,
						m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 0xff));
				}
			}
		}
	}
}

uint32_t gaplus_base_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* flip screen control is embedded in RAM */
	flip_screen_set(m_spriteram[0x1f7f - 0x800] & 1);

	bitmap.fill(0, cliprect);

	starfield_render(bitmap);

	/* draw the low priority characters */
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	draw_sprites(bitmap, cliprect);

	/* draw the high priority characters */
	/* (I don't know if this feature is used by Gaplus, but it's shown in the schematics) */
	m_bg_tilemap->draw(screen, bitmap, cliprect, 1, 0);
	return 0;
}


WRITE_LINE_MEMBER(gaplus_base_state::screen_vblank)/* update starfields */
{
	// falling edge
	if (!state)
	{
		struct star *stars = m_stars;
		int i;

		int width = m_screen->width();
		int height = m_screen->height();

		/* check if we're running */
		if ( ( m_starfield_control[0] & 1 ) == 0 )
			return;

		/* update the starfields */
		for ( i = 0; i < m_total_stars; i++ ) {
			switch( m_starfield_control[stars[i].set + 1] ) {
				case 0x87:
					/* stand still */
				break;

				case 0x86:
					/* scroll down (speed 1) */
					stars[i].x += SPEED_1;
				break;

				case 0x85:
					/* scroll down (speed 2) */
					stars[i].x += SPEED_2;
				break;

				case 0x06:
					/* scroll down (speed 3) */
					stars[i].x += SPEED_3;
				break;

				case 0x80:
					/* scroll up (speed 1) */
					stars[i].x -= SPEED_1;
				break;

				case 0x82:
					/* scroll up (speed 2) */
					stars[i].x -= SPEED_2;
				break;

				case 0x81:
					/* scroll up (speed 3) */
					stars[i].x -= SPEED_3;
				break;

				case 0x9f:
					/* scroll left (speed 2) */
					stars[i].y += SPEED_2;
				break;

				case 0xaf:
					/* scroll left (speed 1) */
					stars[i].y += SPEED_1;
				break;
			}

			/* wrap */
			if ( stars[i].x < 0 )
				stars[i].x = ( float )( width*2 ) + stars[i].x;

			if ( stars[i].x >= ( float )( width*2 ) )
				stars[i].x -= ( float )( width*2 );

			if ( stars[i].y < 0 )
				stars[i].y = ( float )( height ) + stars[i].y;

			if ( stars[i].y >= ( float )( height ) )
				stars[i].y -= ( float )( height );
		}
	}
}
