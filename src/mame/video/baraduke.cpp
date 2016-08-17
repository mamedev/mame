// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
#include "emu.h"
#include "includes/baraduke.h"


/***************************************************************************

    Convert the color PROMs.

    The palette PROMs are connected to the RGB output this way:

    bit 3   -- 220 ohm resistor  -- RED/GREEN/BLUE
            -- 470 ohm resistor  -- RED/GREEN/BLUE
            -- 1  kohm resistor  -- RED/GREEN/BLUE
    bit 0   -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

PALETTE_INIT_MEMBER(baraduke_state, baraduke)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;
	int bit0,bit1,bit2,bit3,r,g,b;

	for (i = 0; i < 2048; i++)
	{
		/* red component */
		bit0 = (color_prom[2048] >> 0) & 0x01;
		bit1 = (color_prom[2048] >> 1) & 0x01;
		bit2 = (color_prom[2048] >> 2) & 0x01;
		bit3 = (color_prom[2048] >> 3) & 0x01;
		r = 0x0e*bit0 + 0x1f*bit1 + 0x43*bit2 + 0x8f*bit3;

		/* green component */
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		g = 0x0e*bit0 + 0x1f*bit1 + 0x43*bit2 + 0x8f*bit3;

		/* blue component */
		bit0 = (color_prom[0] >> 4) & 0x01;
		bit1 = (color_prom[0] >> 5) & 0x01;
		bit2 = (color_prom[0] >> 6) & 0x01;
		bit3 = (color_prom[0] >> 7) & 0x01;
		b = 0x0e*bit0 + 0x1f*bit1 + 0x43*bit2 + 0x8f*bit3;

		palette.set_pen_color(i,rgb_t(r,g,b));
		color_prom++;
	}
}



/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

/* convert from 32x32 to 36x28 */
TILEMAP_MAPPER_MEMBER(baraduke_state::tx_tilemap_scan)
{
	int offs;

	row += 2;
	col -= 2;
	if (col & 0x20)
		offs = row + ((col & 0x1f) << 5);
	else
		offs = col + (row << 5);

	return offs;
}

TILE_GET_INFO_MEMBER(baraduke_state::tx_get_tile_info)
{
	SET_TILE_INFO_MEMBER(0,
			m_textram[tile_index],
			(m_textram[tile_index+0x400] << 2) & 0x1ff,
			0);
}

TILE_GET_INFO_MEMBER(baraduke_state::get_tile_info0)
{
	int code = m_videoram[2*tile_index];
	int attr = m_videoram[2*tile_index + 1];

	SET_TILE_INFO_MEMBER(1,
			code + ((attr & 0x03) << 8),
			attr,
			0);
}

TILE_GET_INFO_MEMBER(baraduke_state::get_tile_info1)
{
	int code = m_videoram[0x1000 + 2*tile_index];
	int attr = m_videoram[0x1000 + 2*tile_index + 1];

	SET_TILE_INFO_MEMBER(2,
			code + ((attr & 0x03) << 8),
			attr,
			0);
}



/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void baraduke_state::video_start()
{
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(baraduke_state::tx_get_tile_info),this),tilemap_mapper_delegate(FUNC(baraduke_state::tx_tilemap_scan),this),8,8,36,28);
	m_bg_tilemap[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(baraduke_state::get_tile_info0),this),TILEMAP_SCAN_ROWS,8,8,64,32);
	m_bg_tilemap[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(baraduke_state::get_tile_info1),this),TILEMAP_SCAN_ROWS,8,8,64,32);

	m_tx_tilemap->set_transparent_pen(3);
	m_bg_tilemap[0]->set_transparent_pen(7);
	m_bg_tilemap[1]->set_transparent_pen(7);

	m_bg_tilemap[0]->set_scrolldx(-26, -227+26);
	m_bg_tilemap[1]->set_scrolldx(-24, -227+24);
	m_bg_tilemap[0]->set_scrolldy(-9, 9);
	m_bg_tilemap[1]->set_scrolldy(-9, 9);
	m_tx_tilemap->set_scrolldy(16,16);
}



/***************************************************************************

    Memory handlers

***************************************************************************/

READ8_MEMBER(baraduke_state::baraduke_videoram_r)
{
	return m_videoram[offset];
}

WRITE8_MEMBER(baraduke_state::baraduke_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap[offset/0x1000]->mark_tile_dirty((offset&0xfff)/2);
}

READ8_MEMBER(baraduke_state::baraduke_textram_r)
{
	return m_textram[offset];
}

WRITE8_MEMBER(baraduke_state::baraduke_textram_w)
{
	m_textram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset & 0x3ff);
}


void baraduke_state::scroll_w(address_space &space, int layer, int offset, int data)
{
	switch (offset)
	{
		case 0: /* high scroll x */
			m_xscroll[layer] = (m_xscroll[layer] & 0xff) | (data << 8);
			break;
		case 1: /* low scroll x */
			m_xscroll[layer] = (m_xscroll[layer] & 0xff00) | data;
			break;
		case 2: /* scroll y */
			m_yscroll[layer] = data;
			break;
	}
}

WRITE8_MEMBER(baraduke_state::baraduke_scroll0_w)
{
	scroll_w(space, 0, offset, data);
}
WRITE8_MEMBER(baraduke_state::baraduke_scroll1_w)
{
	scroll_w(space, 1, offset, data);
}



READ8_MEMBER(baraduke_state::baraduke_spriteram_r)
{
	return m_spriteram[offset];
}

WRITE8_MEMBER(baraduke_state::baraduke_spriteram_w)
{
	m_spriteram[offset] = data;

	/* a write to this offset tells the sprite chip to buffer the sprite list */
	if (offset == 0x1ff2)
		m_copy_sprites = 1;
}



/***************************************************************************

    Display Refresh

***************************************************************************/

void baraduke_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int sprite_priority)
{
	UINT8 *spriteram = m_spriteram + 0x1800;
	const UINT8 *source = &spriteram[0x0000];
	const UINT8 *finish = &spriteram[0x0800-16];    /* the last is NOT a sprite */

	int sprite_xoffs = spriteram[0x07f5] - 256 * (spriteram[0x07f4] & 1);
	int sprite_yoffs = spriteram[0x07f7];

	while( source<finish )
	{
/*
    source[10] S-FT ---P
    source[11] TTTT TTTT
    source[12] CCCC CCCX
    source[13] XXXX XXXX
    source[14] ---T -S-F
    source[15] YYYY YYYY
*/
		int priority = source[10] & 0x01;
		if (priority == sprite_priority)
		{
			static const int gfx_offs[2][2] =
			{
				{ 0, 1 },
				{ 2, 3 }
			};
			int attr1 = source[10];
			int attr2 = source[14];
			int color = source[12];
			int sx = source[13] + (color & 0x01)*256;
			int sy = 240 - source[15];
			int flipx = (attr1 & 0x20) >> 5;
			int flipy = (attr2 & 0x01);
			int sizex = (attr1 & 0x80) >> 7;
			int sizey = (attr2 & 0x04) >> 2;
			int sprite = (source[11] & 0xff)*4;
			int x,y;

			if ((attr1 & 0x10) && !sizex) sprite += 1;
			if ((attr2 & 0x10) && !sizey) sprite += 2;
			color = color >> 1;

			sx += sprite_xoffs;
			sy -= sprite_yoffs;

			sy -= 16 * sizey;

			if (flip_screen())
			{
				sx = 496+3 - 16 * sizex - sx;
				sy = 240 - 16 * sizey - sy;
				flipx ^= 1;
				flipy ^= 1;
			}

			for (y = 0;y <= sizey;y++)
			{
				for (x = 0;x <= sizex;x++)
				{
					m_gfxdecode->gfx(3)->transpen(bitmap,cliprect,
						sprite + gfx_offs[y ^ (sizey * flipy)][x ^ (sizex * flipx)],
						color,
						flipx,flipy,
						-71 + ((sx + 16*x) & 0x1ff),
						1 + ((sy + 16*y) & 0xff),0xf);
				}
			}
		}

		source+=16;
	}
}


void baraduke_state::set_scroll(int layer)
{
	int scrollx = m_xscroll[layer];
	int scrolly = m_yscroll[layer];

	if (flip_screen())
	{
		scrollx = -scrollx;
		scrolly = -scrolly;
	}

	m_bg_tilemap[layer]->set_scrollx(0, scrollx);
	m_bg_tilemap[layer]->set_scrolly(0, scrolly);
}


UINT32 baraduke_state::screen_update_baraduke(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *spriteram = m_spriteram + 0x1800;
	int back;

	/* flip screen is embedded in the sprite control registers */
	flip_screen_set(spriteram[0x07f6] & 0x01);
	set_scroll(0);
	set_scroll(1);

	if (((m_xscroll[0] & 0x0e00) >> 9) == 6)
		back = 1;
	else
		back = 0;

	m_bg_tilemap[back]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE,0);
	draw_sprites(bitmap,cliprect,0);
	m_bg_tilemap[back ^ 1]->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap,cliprect,1);

	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}


void baraduke_state::screen_eof_baraduke(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		if (m_copy_sprites)
		{
			UINT8 *spriteram = m_spriteram + 0x1800;
			int i,j;

			for (i = 0;i < 0x800;i += 16)
			{
				for (j = 10;j < 16;j++)
					spriteram[i+j] = spriteram[i+j - 6];
			}

			m_copy_sprites = 0;
		}
	}
}
