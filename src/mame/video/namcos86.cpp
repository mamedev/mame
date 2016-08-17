// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*******************************************************************

Namco System 86 Video Hardware

*******************************************************************/

#include "emu.h"
#include "includes/namcos86.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Rolling Thunder has two palette PROMs (512x8 and 512x4) and two 2048x8
  lookup table PROMs.
  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 1  kohm resistor  -- BLUE
  bit 0 -- 2.2kohm resistor  -- BLUE

  bit 7 -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 2.2kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
        -- 1  kohm resistor  -- RED
  bit 0 -- 2.2kohm resistor  -- RED

***************************************************************************/

PALETTE_INIT_MEMBER(namcos86_state, namcos86)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;
	rgb_t palette_val[512];

	for (i = 0;i < 512;i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;

		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[0] >> 4) & 0x01;
		bit1 = (color_prom[0] >> 5) & 0x01;
		bit2 = (color_prom[0] >> 6) & 0x01;
		bit3 = (color_prom[0] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[512] >> 0) & 0x01;
		bit1 = (color_prom[512] >> 1) & 0x01;
		bit2 = (color_prom[512] >> 2) & 0x01;
		bit3 = (color_prom[512] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_val[i] = rgb_t(r,g,b);
		color_prom++;
	}

	color_prom += 512;
	/* color_prom now points to the beginning of the lookup table */

	/* tiles lookup table */
	for (i = 0;i < 2048;i++)
		palette.set_pen_color(i, palette_val[*color_prom++]);

	/* sprites lookup table */
	for (i = 0;i < 2048;i++)
		palette.set_pen_color(2048 + i, palette_val[256 + *color_prom++]);

	/* color_prom now points to the beginning of the tile address decode PROM */

	m_tile_address_prom = color_prom;   /* we'll need this at run time */
}




/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

inline void namcos86_state::get_tile_info(tile_data &tileinfo,int tile_index,int layer,UINT8 *vram)
{
	int attr = vram[2*tile_index + 1];
	int tile_offs;
	if (layer & 2)
		tile_offs = ((m_tile_address_prom[((layer & 1) << 4) + (attr & 0x03)] & 0xe0) >> 5) * 0x100;
	else
		tile_offs = ((m_tile_address_prom[((layer & 1) << 4) + ((attr & 0x03) << 2)] & 0x0e) >> 1) * 0x100 + m_tilebank * 0x800;

	SET_TILE_INFO_MEMBER((layer & 2) ? 1 : 0,
			vram[2*tile_index] + tile_offs,
			attr,
			0);
}

TILE_GET_INFO_MEMBER(namcos86_state::get_tile_info0)
{
	get_tile_info(tileinfo,tile_index,0,&m_rthunder_videoram1[0x0000]);
}

TILE_GET_INFO_MEMBER(namcos86_state::get_tile_info1)
{
	get_tile_info(tileinfo,tile_index,1,&m_rthunder_videoram1[0x1000]);
}

TILE_GET_INFO_MEMBER(namcos86_state::get_tile_info2)
{
	get_tile_info(tileinfo,tile_index,2,&m_rthunder_videoram2[0x0000]);
}

TILE_GET_INFO_MEMBER(namcos86_state::get_tile_info3)
{
	get_tile_info(tileinfo,tile_index,3,&m_rthunder_videoram2[0x1000]);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void namcos86_state::video_start()
{
	m_bg_tilemap[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(namcos86_state::get_tile_info0),this),TILEMAP_SCAN_ROWS,8,8,64,32);
	m_bg_tilemap[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(namcos86_state::get_tile_info1),this),TILEMAP_SCAN_ROWS,8,8,64,32);
	m_bg_tilemap[2] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(namcos86_state::get_tile_info2),this),TILEMAP_SCAN_ROWS,8,8,64,32);
	m_bg_tilemap[3] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(namcos86_state::get_tile_info3),this),TILEMAP_SCAN_ROWS,8,8,64,32);

	for (int i = 0; i < 4; i++)
	{
		static const int xdisp[] = { 47, 49, 46, 48 };

		m_bg_tilemap[i]->set_scrolldx(xdisp[i], 422 - xdisp[i]);
		m_bg_tilemap[i]->set_scrolldy(-9, 9);
		m_bg_tilemap[i]->set_transparent_pen(7);
	}

	m_spriteram = m_rthunder_spriteram + 0x1800;

	save_item(NAME(m_tilebank));
	save_item(NAME(m_xscroll));
	save_item(NAME(m_yscroll));
	save_item(NAME(m_backcolor));
	save_item(NAME(m_copy_sprites));
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(namcos86_state::videoram1_w)
{
	m_rthunder_videoram1[offset] = data;
	m_bg_tilemap[offset/0x1000]->mark_tile_dirty((offset & 0xfff)/2);
}

WRITE8_MEMBER(namcos86_state::videoram2_w)
{
	m_rthunder_videoram2[offset] = data;
	m_bg_tilemap[2+offset/0x1000]->mark_tile_dirty((offset & 0xfff)/2);
}

WRITE8_MEMBER(namcos86_state::tilebank_select_w)
{
	int bit = BIT(offset,10);
	if (m_tilebank != bit)
	{
		m_tilebank = bit;
		m_bg_tilemap[0]->mark_all_dirty();
		m_bg_tilemap[1]->mark_all_dirty();
	}
}

void namcos86_state::scroll_w(address_space &space, int offset, int data, int layer)
{
	switch (offset)
	{
		case 0:
			m_xscroll[layer] = (m_xscroll[layer]&0xff)|(data<<8);
			break;
		case 1:
			m_xscroll[layer] = (m_xscroll[layer]&0xff00)|data;
			break;
		case 2:
			m_yscroll[layer] = data;
			break;
	}
}

WRITE8_MEMBER(namcos86_state::scroll0_w)
{
	scroll_w(space,offset,data,0);
}
WRITE8_MEMBER(namcos86_state::scroll1_w)
{
	scroll_w(space,offset,data,1);
}
WRITE8_MEMBER(namcos86_state::scroll2_w)
{
	scroll_w(space,offset,data,2);
}
WRITE8_MEMBER(namcos86_state::scroll3_w)
{
	scroll_w(space,offset,data,3);
}

WRITE8_MEMBER(namcos86_state::backcolor_w)
{
	m_backcolor = data;
}


WRITE8_MEMBER(namcos86_state::spriteram_w)
{
	m_rthunder_spriteram[offset] = data;

	/* a write to this offset tells the sprite chip to buffer the sprite list */
	if (offset == 0x1ff2)
		m_copy_sprites = 1;
}


/***************************************************************************

  Display refresh

***************************************************************************/

/*
sprite format:

0-3  scratchpad RAM
4-9  CPU writes here, hardware copies from here to 10-15
10   xx------  X size (16, 8, 32, 4)
10   --x-----  X flip
10   ---xx---  X offset inside 32x32 tile
10   -----xxx  tile bank
11   xxxxxxxx  tile number
12   xxxxxxx-  color
12   -------x  X position MSB
13   xxxxxxxx  X position
14   xxx-----  priority
14   ---xx---  Y offset inside 32x32 tile
14   -----xx-  Y size (16, 8, 32, 4)
14   -------x  Y flip
15   xxxxxxxx  Y position
*/

void namcos86_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const UINT8 *source = &m_spriteram[0x0800-0x20]; /* the last is NOT a sprite */
	const UINT8 *finish = &m_spriteram[0];
	gfx_element *gfx = m_gfxdecode->gfx(2);

	int sprite_xoffs = m_spriteram[0x07f5] + ((m_spriteram[0x07f4] & 1) << 8);
	int sprite_yoffs = m_spriteram[0x07f7];

	int bank_sprites = m_gfxdecode->gfx(2)->elements() / 8;

	while (source >= finish)
	{
		static const int sprite_size[4] = { 16, 8, 32, 4 };
		int attr1 = source[10];
		int attr2 = source[14];
		int color = source[12];
		int flipx = (attr1 & 0x20) >> 5;
		int flipy = (attr2 & 0x01);
		int sizex = sprite_size[(attr1 & 0xc0) >> 6];
		int sizey = sprite_size[(attr2 & 0x06) >> 1];
		int tx = (attr1 & 0x18) & (~(sizex-1));
		int ty = (attr2 & 0x18) & (~(sizey-1));
		int sx = source[13] + ((color & 0x01) << 8);
		int sy = -source[15] - sizey;
		int sprite = source[11];
		int sprite_bank = attr1 & 7;
		int priority = (source[14] & 0xe0) >> 5;
		int pri_mask = (0xff << (priority + 1)) & 0xff;

		sprite &= bank_sprites-1;
		sprite += sprite_bank * bank_sprites;
		color = color >> 1;

		sx += sprite_xoffs;
		sy -= sprite_yoffs;

		if (flip_screen())
		{
			sx = -sx - sizex;
			sy = -sy - sizey;
			flipx ^= 1;
			flipy ^= 1;
		}

		sy++;   /* sprites are buffered and delayed by one scanline */

		gfx->set_source_clip(tx, sizex, ty, sizey);
		gfx->prio_transpen(bitmap,cliprect,
				sprite,
				color,
				flipx,flipy,
				sx & 0x1ff,
				((sy + 16) & 0xff) - 16,
				screen.priority(), pri_mask,0xf);

		source -= 0x10;
	}
}


void namcos86_state::set_scroll(int layer)
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


UINT32 namcos86_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* flip screen is embedded in the sprite control registers */
	flip_screen_set(m_spriteram[0x07f6] & 1);
	set_scroll(0);
	set_scroll(1);
	set_scroll(2);
	set_scroll(3);

	screen.priority().fill(0, cliprect);

	bitmap.fill(m_gfxdecode->gfx(0)->colorbase() + 8*m_backcolor+7, cliprect);

	for (int layer = 0;layer < 8;layer++)
	{
		for (int i = 3;i >= 0;i--)
		{
			if (((m_xscroll[i] & 0x0e00) >> 9) == layer)
				m_bg_tilemap[i]->draw(screen, bitmap, cliprect, 0,layer,0);
		}
	}

	draw_sprites(screen,bitmap,cliprect);
	return 0;
}


void namcos86_state::screen_eof(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		if (m_copy_sprites)
		{
			for (int i = 0;i < 0x800;i += 16)
			{
				for (int j = 10;j < 16;j++)
					m_spriteram[i+j] = m_spriteram[i+j - 6];
			}

			m_copy_sprites = 0;
		}
	}
}
