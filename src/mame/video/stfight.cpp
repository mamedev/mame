// license:BSD-3-Clause
// copyright-holders:Mark McDougall
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/stfight.h"


/*
        Graphics ROM Format
        ===================

        Each tile is 8x8 pixels
        Each composite tile is 2x2 tiles, 16x16 pixels
        Each screen is 32x32 composite tiles, 64x64 tiles, 256x256 pixels
        Each layer is a 4-plane bitmap 8x16 screens, 2048x4096 pixels

        There are 4x256=1024 composite tiles defined for each layer

        Each layer is mapped using 2 bytes/composite tile
        - one byte for the tile
        - one byte for the tile bank, attribute
            - b7,b5     tile bank (0-3)

        Each pixel is 4 bits = 16 colours.

 */

PALETTE_INIT_MEMBER(stfight_state, stfight)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	/* text uses colors 0xc0-0xcf */
	for (i = 0; i < 0x40; i++)
	{
		UINT8 ctabentry = (color_prom[i] & 0x0f) | 0xc0;
		palette.set_pen_indirect(i, ctabentry);
	}

	/* fg uses colors 0x40-0x7f */
	for (i = 0x40; i < 0x140; i++)
	{
		UINT8 ctabentry = (color_prom[i + 0x1c0] & 0x0f) | ((color_prom[i + 0x0c0] & 0x03) << 4) | 0x40;
		palette.set_pen_indirect(i, ctabentry);
	}

	/* bg uses colors 0-0x3f */
	for (i = 0x140; i < 0x240; i++)
	{
		UINT8 ctabentry = (color_prom[i + 0x2c0] & 0x0f) | ((color_prom[i + 0x1c0] & 0x03) << 4);
		palette.set_pen_indirect(i, ctabentry);
	}

	/* bg uses colors 0x80-0xbf */
	for (i = 0x240; i < 0x340; i++)
	{
		UINT8 ctabentry = (color_prom[i + 0x3c0] & 0x0f) | ((color_prom[i + 0x2c0] & 0x03) << 4) | 0x80;
		palette.set_pen_indirect(i, ctabentry);
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILEMAP_MAPPER_MEMBER(stfight_state::fg_scan)
{
	/* logical (col,row) -> memory offset */
	return (col & 0x0f) + ((row & 0x0f) << 4) + ((col & 0x70) << 4) + ((row & 0xf0) << 7);
}

TILE_GET_INFO_MEMBER(stfight_state::get_fg_tile_info)
{
	UINT8   *fgMap = memregion("gfx5")->base();
	int attr,tile_base;

	attr = fgMap[0x8000+tile_index];
	tile_base = ((attr & 0x80) << 2) | ((attr & 0x20) << 3);

	SET_TILE_INFO_MEMBER(1,
			tile_base + fgMap[tile_index],
			attr & 0x07,
			0);
}

TILEMAP_MAPPER_MEMBER(stfight_state::bg_scan)
{
	/* logical (col,row) -> memory offset */
	return ((col & 0x0e) >> 1) + ((row & 0x0f) << 3) + ((col & 0x70) << 3) +
			((row & 0x80) << 3) + ((row & 0x10) << 7) + ((col & 0x01) << 12) +
			((row & 0x60) << 8);
}

TILE_GET_INFO_MEMBER(stfight_state::get_bg_tile_info)
{
	UINT8   *bgMap = memregion("gfx6")->base();
	int attr,tile_bank,tile_base;

	attr = bgMap[0x8000+tile_index];
	tile_bank = (attr & 0x20) >> 5;
	tile_base = (attr & 0x80) << 1;

	SET_TILE_INFO_MEMBER(2+tile_bank,
			tile_base + bgMap[tile_index],
			attr & 0x07,
			0);
}

TILE_GET_INFO_MEMBER(stfight_state::get_tx_tile_info)
{
	UINT8 attr = m_text_attr_ram[tile_index];
	int color = attr & 0x0f;

	tileinfo.group = color;

	SET_TILE_INFO_MEMBER(0,
			m_text_char_ram[tile_index] + ((attr & 0x80) << 1),
			attr & 0x0f,
			TILE_FLIPYX((attr & 0x60) >> 5));
}

TILE_GET_INFO_MEMBER(stfight_state::get_cshooter_tx_tile_info)
{
	UINT8 tile = m_tx_vram[tile_index*2];
	UINT8 attr = m_tx_vram[tile_index*2+1];
	int color = attr & 0x0f;

	tileinfo.group = color;

	SET_TILE_INFO_MEMBER(0,
			(tile << 1) | ((attr & 0x20) >> 5),
			attr & 0x0f,
			/*TILE_FLIPYX((attr & 0x60) >> 5)*/0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(stfight_state,stfight)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(stfight_state::get_bg_tile_info),this),tilemap_mapper_delegate(FUNC(stfight_state::bg_scan),this),16,16,128,256);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(stfight_state::get_fg_tile_info),this),tilemap_mapper_delegate(FUNC(stfight_state::fg_scan),this),16,16,128,256);
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(stfight_state::get_tx_tile_info),this),TILEMAP_SCAN_ROWS, 8,8,32,32);

	m_fg_tilemap->set_transparent_pen(0x0f);
	m_tx_tilemap->configure_groups(*m_gfxdecode->gfx(0), 0xcf);
}

VIDEO_START_MEMBER(stfight_state,cshooter)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(stfight_state::get_bg_tile_info),this),tilemap_mapper_delegate(FUNC(stfight_state::bg_scan),this),16,16,128,256);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(stfight_state::get_fg_tile_info),this),tilemap_mapper_delegate(FUNC(stfight_state::fg_scan),this),16,16,128,256);
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(stfight_state::get_cshooter_tx_tile_info),this),TILEMAP_SCAN_ROWS, 8,8,32,32);

	m_fg_tilemap->set_transparent_pen(0x0f);
	m_tx_tilemap->configure_groups(*m_gfxdecode->gfx(0), 0xcf);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(stfight_state::stfight_text_char_w)
{
	m_text_char_ram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(stfight_state::stfight_text_attr_w)
{
	m_text_attr_ram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(stfight_state::cshooter_text_w)
{
	m_tx_vram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset/2);
}


WRITE8_MEMBER(stfight_state::stfight_sprite_bank_w)
{
	m_sprite_base = ( ( data & 0x04 ) << 7 ) |
							( ( data & 0x01 ) << 8 );
}

WRITE8_MEMBER(stfight_state::stfight_vh_latch_w)
{
	int scroll;


	m_vh_latch_ram[offset] = data;

	switch( offset )
	{
		case 0x00:
		case 0x01:
			scroll = (m_vh_latch_ram[1] << 8) | m_vh_latch_ram[0];
			m_fg_tilemap->set_scrollx(0,scroll);
			break;

		case 0x02:
		case 0x03:
			scroll = (m_vh_latch_ram[3] << 8) | m_vh_latch_ram[2];
			m_fg_tilemap->set_scrolly(0,scroll);
			break;

		case 0x04:
		case 0x05:
			scroll = (m_vh_latch_ram[5] << 8) | m_vh_latch_ram[4];
			m_bg_tilemap->set_scrollx(0,scroll);
			break;

		case 0x06:
		case 0x08:
			scroll = (m_vh_latch_ram[8] << 8) | m_vh_latch_ram[6];
			m_bg_tilemap->set_scrolly(0,scroll);
			break;

		case 0x07:
			m_tx_tilemap->enable(data & 0x80);
			/* 0x40 = sprites */
			m_bg_tilemap->enable(data & 0x20);
			m_fg_tilemap->enable(data & 0x10);
			flip_screen_set(data & 0x01);
			break;
	}
}

/***************************************************************************

  Display refresh

***************************************************************************/

void stfight_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs,sx,sy;

	for (offs = 0;offs < 4096;offs += 32)
	{
		int code;
		int attr = m_sprite_ram[offs+1];
		int flipx = attr & 0x10;
		int color = attr & 0x0f;
		int pri = (attr & 0x20) >> 5;

		sy = m_sprite_ram[offs+2];
		sx = m_sprite_ram[offs+3];

		// non-active sprites have zero y coordinate value
		if( sy > 0 )
		{
			// sprites which wrap onto/off the screen have
			// a sign extension bit in the sprite attribute
			if( sx >= 0xf0 )
			{
				if (attr & 0x80)
					sx -= 0x100;
			}

			if (flip_screen())
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
			}

			code = m_sprite_base + m_sprite_ram[offs];

			m_gfxdecode->gfx(4)->prio_transpen(bitmap,cliprect,
						code,
						color,
						flipx,flip_screen(),
						sx,sy,
						screen.priority(),
						pri ? 0x02 : 0,0x0f);
		}
	}
}


UINT32 stfight_state::screen_update_stfight(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	bitmap.fill(0, cliprect);   /* in case m_bg_tilemap is disabled */
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0,1);

	/* Draw sprites (may be obscured by foreground layer) */
	if (m_vh_latch_ram[0x07] & 0x40)
		draw_sprites(screen,bitmap,cliprect);

	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}

void stfight_state::cshooter_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int i = m_sprite_ram.bytes() - 4; i >= 0 ; i -= 4)
	{
		if (m_sprite_ram[i+1]&0x80)
			continue;

		int attr = m_sprite_ram[i+1];
		int flipx = attr & 0x10;
		int color = attr & 0x0f;
		int pri = (attr & 0x20) >> 5;

		/* BCD debug code, to be removed in the end */
		UINT8 tile_low = (m_sprite_ram[i]&0x0f);
		UINT8 tile_high = ((m_sprite_ram[i]&0xf0)>>4);

		tile_low += (tile_low > 0x9) ? 0x37 : 0x30;
		tile_high += (tile_high > 0x9) ? 0x37 : 0x30;

		m_gfxdecode->gfx(0)->prio_transpen(bitmap,cliprect, tile_high << 1, color, flipx, 0, m_sprite_ram[i+3],m_sprite_ram[i+2],screen.priority(),pri ? 0x02 : 0,0x00);
		m_gfxdecode->gfx(0)->prio_transpen(bitmap,cliprect, tile_high << 1, color, flipx, 0, m_sprite_ram[i+3]+8,m_sprite_ram[i+2],screen.priority(),pri ? 0x02 : 0,0x00);
		m_gfxdecode->gfx(0)->prio_transpen(bitmap,cliprect, tile_low << 1, color, flipx, 0, m_sprite_ram[i+3]+8,m_sprite_ram[i+2]+8,screen.priority(),pri ? 0x02 : 0,0x00);
		m_gfxdecode->gfx(0)->prio_transpen(bitmap,cliprect, tile_low << 1, color, flipx, 0, m_sprite_ram[i+3],m_sprite_ram[i+2]+8,screen.priority(),pri ? 0x02 : 0,0x00);
	}
}

UINT32 stfight_state::screen_update_cshooter(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	bitmap.fill(0, cliprect);   /* in case m_bg_tilemap is disabled */
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0,1);

	/* Draw sprites (may be obscured by foreground layer) */
//  if (m_vh_latch_ram[0x07] & 0x40)
		cshooter_draw_sprites(screen,bitmap,cliprect);

	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}
