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

void stfight_state::palette_init()
{
	const UINT8 *color_prom = machine().root_device().memregion("proms")->base();
	int i;

	/* allocate the colortable */
	machine().colortable = colortable_alloc(machine(), 0x100);

	/* text uses colors 0xc0-0xcf */
	for (i = 0; i < 0x40; i++)
	{
		UINT8 ctabentry = (color_prom[i] & 0x0f) | 0xc0;
		colortable_entry_set_value(machine().colortable, i, ctabentry);
	}

	/* fg uses colors 0x40-0x7f */
	for (i = 0x40; i < 0x140; i++)
	{
		UINT8 ctabentry = (color_prom[i + 0x1c0] & 0x0f) | ((color_prom[i + 0x0c0] & 0x03) << 4) | 0x40;
		colortable_entry_set_value(machine().colortable, i, ctabentry);
	}

	/* bg uses colors 0-0x3f */
	for (i = 0x140; i < 0x240; i++)
	{
		UINT8 ctabentry = (color_prom[i + 0x2c0] & 0x0f) | ((color_prom[i + 0x1c0] & 0x03) << 4);
		colortable_entry_set_value(machine().colortable, i, ctabentry);
	}

	/* bg uses colors 0x80-0xbf */
	for (i = 0x240; i < 0x340; i++)
	{
		UINT8 ctabentry = (color_prom[i + 0x3c0] & 0x0f) | ((color_prom[i + 0x2c0] & 0x03) << 4) | 0x80;
		colortable_entry_set_value(machine().colortable, i, ctabentry);
	}
}


static void set_pens(running_machine &machine)
{
	stfight_state *state = machine.driver_data<stfight_state>();
	int i;

	for (i = 0; i < 0x100; i++)
	{
		UINT16 data = state->m_generic_paletteram_8[i] | (state->m_generic_paletteram2_8[i] << 8);
		rgb_t color = MAKE_RGB(pal4bit(data >> 4), pal4bit(data >> 0), pal4bit(data >> 8));

		colortable_palette_set_color(machine.colortable, i, color);
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
	UINT8   *fgMap = machine().root_device().memregion("gfx5")->base();
	int attr,tile_base;

	attr = fgMap[0x8000+tile_index];
	tile_base = ((attr & 0x80) << 2) | ((attr & 0x20) << 3);

	SET_TILE_INFO_MEMBER(
			1,
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
	UINT8   *bgMap = machine().root_device().memregion("gfx6")->base();
	int attr,tile_bank,tile_base;

	attr = bgMap[0x8000+tile_index];
	tile_bank = (attr & 0x20) >> 5;
	tile_base = (attr & 0x80) << 1;

	SET_TILE_INFO_MEMBER(
			2+tile_bank,
			tile_base + bgMap[tile_index],
			attr & 0x07,
			0);
}

TILE_GET_INFO_MEMBER(stfight_state::get_tx_tile_info)
{
	UINT8 attr = m_text_attr_ram[tile_index];
	int color = attr & 0x0f;

	tileinfo.group = color;

	SET_TILE_INFO_MEMBER(
			0,
			m_text_char_ram[tile_index] + ((attr & 0x80) << 1),
			attr & 0x0f,
			TILE_FLIPYX((attr & 0x60) >> 5));
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void stfight_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(stfight_state::get_bg_tile_info),this),tilemap_mapper_delegate(FUNC(stfight_state::bg_scan),this),16,16,128,256);
	m_fg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(stfight_state::get_fg_tile_info),this),tilemap_mapper_delegate(FUNC(stfight_state::fg_scan),this),16,16,128,256);
	m_tx_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(stfight_state::get_tx_tile_info),this),TILEMAP_SCAN_ROWS, 8,8,32,32);

	m_fg_tilemap->set_transparent_pen(0x0f);
	colortable_configure_tilemap_groups(machine().colortable, m_tx_tilemap, machine().gfx[0], 0xcf);
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

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	stfight_state *state = machine.driver_data<stfight_state>();
	int offs,sx,sy;

	for (offs = 0;offs < 4096;offs += 32)
	{
		int code;
		int attr = state->m_sprite_ram[offs+1];
		int flipx = attr & 0x10;
		int color = attr & 0x0f;
		int pri = (attr & 0x20) >> 5;

		sy = state->m_sprite_ram[offs+2];
		sx = state->m_sprite_ram[offs+3];

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

			if (state->flip_screen())
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
			}

			code = state->m_sprite_base + state->m_sprite_ram[offs];

			pdrawgfx_transpen(bitmap,cliprect,machine.gfx[4],
						code,
						color,
						flipx,state->flip_screen(),
						sx,sy,
						machine.priority_bitmap,
						pri ? 0x02 : 0,0x0f);
		}
	}
}


UINT32 stfight_state::screen_update_stfight(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	set_pens(machine());

	machine().priority_bitmap.fill(0, cliprect);

	bitmap.fill(0, cliprect);   /* in case m_bg_tilemap is disabled */
	m_bg_tilemap->draw(bitmap, cliprect, 0,0);
	m_fg_tilemap->draw(bitmap, cliprect, 0,1);

	/* Draw sprites (may be obscured by foreground layer) */
	if (m_vh_latch_ram[0x07] & 0x40)
		draw_sprites(machine(), bitmap,cliprect);

	m_tx_tilemap->draw(bitmap, cliprect, 0,0);
	return 0;
}
