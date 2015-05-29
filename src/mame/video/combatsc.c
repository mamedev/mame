// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Manuel Abadia
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"

#include "includes/combatsc.h"

PALETTE_INIT_MEMBER(combatsc_state,combatsc)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int pal;

	for (pal = 0; pal < 8; pal++)
	{
		int i, clut;

		switch (pal)
		{
			default:
			case 0: /* other sprites */
			case 2: /* other sprites(alt) */
			clut = 1;   /* 0 is wrong for Firing Range III targets */
			break;

			case 4: /* player sprites */
			case 6: /* player sprites(alt) */
			clut = 2;
			break;

			case 1: /* background */
			case 3: /* background(alt) */
			clut = 1;
			break;

			case 5: /* foreground tiles */
			case 7: /* foreground tiles(alt) */
			clut = 3;
			break;
		}

		for (i = 0; i < 0x100; i++)
		{
			UINT8 ctabentry;

			if (((pal & 0x01) == 0) && (color_prom[(clut << 8) | i] == 0))
				ctabentry = 0;
			else
				ctabentry = (pal << 4) | (color_prom[(clut << 8) | i] & 0x0f);

			palette.set_pen_indirect((pal << 8) | i, ctabentry);
		}
	}
}


PALETTE_INIT_MEMBER(combatsc_state,combatscb)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int pal;

	for (pal = 0; pal < 8; pal++)
	{
		int i;

		for (i = 0; i < 0x100; i++)
		{
			UINT8 ctabentry;

			if ((pal & 1) == 0)
				/* sprites */
				ctabentry = (pal << 4) | (~color_prom[i] & 0x0f);
			else
				/* chars - no lookup? */
				ctabentry = (pal << 4) | (i & 0x0f);    /* no lookup? */

			palette.set_pen_indirect((pal << 8) | i, ctabentry);
		}
	}
}



/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(combatsc_state::get_tile_info0)
{
	UINT8 ctrl_6 = m_k007121_1->ctrlram_r(generic_space(), 6);
	UINT8 attributes = m_page[0][tile_index];
	int bank = 4 * ((m_vreg & 0x0f) - 1);
	int number, color;

	if (bank < 0)
		bank = 0;

	if ((attributes & 0xb0) == 0)
		bank = 0;   /* text bank */

	if (attributes & 0x80)
		bank += 1;

	if (attributes & 0x10)
		bank += 2;

	if (attributes & 0x20)
		bank += 4;

	color = ((ctrl_6 & 0x10) * 2 + 16) + (attributes & 0x0f);

	number = m_page[0][tile_index + 0x400] + 256 * bank;

	SET_TILE_INFO_MEMBER(0,
			number,
			color,
			0);
	tileinfo.category = (attributes & 0x40) >> 6;
}

TILE_GET_INFO_MEMBER(combatsc_state::get_tile_info1)
{
	UINT8 ctrl_6 = m_k007121_2->ctrlram_r(generic_space(), 6);
	UINT8 attributes = m_page[1][tile_index];
	int bank = 4 * ((m_vreg >> 4) - 1);
	int number, color;

	if (bank < 0)
		bank = 0;

	if ((attributes & 0xb0) == 0)
		bank = 0;   /* text bank */

	if (attributes & 0x80)
		bank += 1;

	if (attributes & 0x10)
		bank += 2;

	if (attributes & 0x20)
		bank += 4;

	color = ((ctrl_6 & 0x10) * 2 + 16 + 4 * 16) + (attributes & 0x0f);

	number = m_page[1][tile_index + 0x400] + 256 * bank;

	SET_TILE_INFO_MEMBER(1,
			number,
			color,
			0);
	tileinfo.category = (attributes & 0x40) >> 6;
}

TILE_GET_INFO_MEMBER(combatsc_state::get_text_info)
{
	UINT8 attributes = m_page[0][tile_index + 0x800];
	int number = m_page[0][tile_index + 0xc00];
	int color = 16 + (attributes & 0x0f);

	SET_TILE_INFO_MEMBER(0,
			number,
			color,
			0);
}


TILE_GET_INFO_MEMBER(combatsc_state::get_tile_info0_bootleg)
{
	UINT8 attributes = m_page[0][tile_index];
	int bank = 4 * ((m_vreg & 0x0f) - 1);
	int number, pal, color;

	if (bank < 0)
		bank = 0;

	if ((attributes & 0xb0) == 0)
		bank = 0;   /* text bank */

	if (attributes & 0x80)
		bank += 1;

	if (attributes & 0x10)
		bank += 2;

	if (attributes & 0x20)
		bank += 4;

	pal = (bank == 0 || bank >= 0x1c || (attributes & 0x40)) ? 1 : 3;
	color = pal*16;// + (attributes & 0x0f);
	number = m_page[0][tile_index + 0x400] + 256 * bank;

	SET_TILE_INFO_MEMBER(0,
			number,
			color,
			0);
}

TILE_GET_INFO_MEMBER(combatsc_state::get_tile_info1_bootleg)
{
	UINT8 attributes = m_page[1][tile_index];
	int bank = 4*((m_vreg >> 4) - 1);
	int number, pal, color;

	if (bank < 0)
		bank = 0;

	if ((attributes & 0xb0) == 0)
		bank = 0;   /* text bank */

	if (attributes & 0x80)
		bank += 1;

	if (attributes & 0x10)
		bank += 2;

	if (attributes & 0x20)
		bank += 4;

	pal = (bank == 0 || bank >= 0x1c || (attributes & 0x40)) ? 5 : 7;
	color = pal * 16;// + (attributes & 0x0f);
	number = m_page[1][tile_index + 0x400] + 256 * bank;

	SET_TILE_INFO_MEMBER(1,
			number,
			color,
			0);
}

TILE_GET_INFO_MEMBER(combatsc_state::get_text_info_bootleg)
{
//  UINT8 attributes = m_page[0][tile_index + 0x800];
	int number = m_page[0][tile_index + 0xc00];
	int color = 16;// + (attributes & 0x0f);

	SET_TILE_INFO_MEMBER(1,
			number,
			color,
			0);
}

/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(combatsc_state,combatsc)
{
	m_bg_tilemap[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(combatsc_state::get_tile_info0),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(combatsc_state::get_tile_info1),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_textlayer =  &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(combatsc_state::get_text_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_spriteram[0] = auto_alloc_array_clear(machine(), UINT8, 0x800);
	m_spriteram[1] = auto_alloc_array_clear(machine(), UINT8, 0x800);

	m_bg_tilemap[0]->set_transparent_pen(0);
	m_bg_tilemap[1]->set_transparent_pen(0);
	m_textlayer->set_transparent_pen(0);

	m_textlayer->set_scroll_rows(32);

	save_pointer(NAME(m_spriteram[0]), 0x800);
	save_pointer(NAME(m_spriteram[1]), 0x800);
}

VIDEO_START_MEMBER(combatsc_state,combatscb)
{
	m_bg_tilemap[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(combatsc_state::get_tile_info0_bootleg),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(combatsc_state::get_tile_info1_bootleg),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_textlayer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(combatsc_state::get_text_info_bootleg),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_spriteram[0] = auto_alloc_array_clear(machine(), UINT8, 0x800);
	m_spriteram[1] = auto_alloc_array_clear(machine(), UINT8, 0x800);

	m_bg_tilemap[0]->set_transparent_pen(0);
	m_bg_tilemap[1]->set_transparent_pen(0);
	m_textlayer->set_transparent_pen(0);

	m_bg_tilemap[0]->set_scroll_rows(32);
	m_bg_tilemap[1]->set_scroll_rows(32);

	save_pointer(NAME(m_spriteram[0]), 0x800);
	save_pointer(NAME(m_spriteram[1]), 0x800);
}

/***************************************************************************

    Memory handlers

***************************************************************************/

READ8_MEMBER(combatsc_state::combatsc_video_r)
{
	return m_videoram[offset];
}

WRITE8_MEMBER(combatsc_state::combatsc_video_w)
{
	m_videoram[offset] = data;

	if (offset < 0x800)
	{
		if (m_video_circuit)
			m_bg_tilemap[1]->mark_tile_dirty(offset & 0x3ff);
		else
			m_bg_tilemap[0]->mark_tile_dirty(offset & 0x3ff);
	}
	else if (offset < 0x1000 && m_video_circuit == 0)
	{
		m_textlayer->mark_tile_dirty(offset & 0x3ff);
	}
}

WRITE8_MEMBER(combatsc_state::combatsc_pf_control_w)
{
	k007121_device *k007121 = m_video_circuit ? m_k007121_2 : m_k007121_1;
	k007121->ctrl_w(space, offset, data);

	if (offset == 7)
		m_bg_tilemap[m_video_circuit]->set_flip((data & 0x08) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	if (offset == 3)
	{
		if (data & 0x08)
			memcpy(m_spriteram[m_video_circuit], m_page[m_video_circuit] + 0x1000, 0x800);
		else
			memcpy(m_spriteram[m_video_circuit], m_page[m_video_circuit] + 0x1800, 0x800);
	}
}

READ8_MEMBER(combatsc_state::combatsc_scrollram_r)
{
	return m_scrollram[offset];
}

WRITE8_MEMBER(combatsc_state::combatsc_scrollram_w)
{
	m_scrollram[offset] = data;
}



/***************************************************************************

    Display Refresh

***************************************************************************/

void combatsc_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, const UINT8 *source, int circuit, bitmap_ind8 &priority_bitmap, UINT32 pri_mask )
{
	k007121_device *k007121 = circuit ? m_k007121_2 : m_k007121_1;
	address_space &space = machine().driver_data()->generic_space();
	int base_color = (circuit * 4) * 16 + (k007121->ctrlram_r(space, 6) & 0x10) * 2;

	k007121->sprites_draw(bitmap, cliprect, m_gfxdecode->gfx(circuit), m_palette, source, base_color, 0, 0, priority_bitmap, pri_mask);
}


UINT32 combatsc_state::screen_update_combatsc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

	address_space &space = machine().driver_data()->generic_space();
	if (m_k007121_1->ctrlram_r(space, 1) & 0x02)
	{
		m_bg_tilemap[0]->set_scroll_rows(32);
		for (i = 0; i < 32; i++)
			m_bg_tilemap[0]->set_scrollx(i, m_scrollram0[i]);
	}
	else
	{
		m_bg_tilemap[0]->set_scroll_rows(1);
		m_bg_tilemap[0]->set_scrollx(0, m_k007121_1->ctrlram_r(space, 0) | ((m_k007121_1->ctrlram_r(space, 1) & 0x01) << 8));
	}

	if (m_k007121_2->ctrlram_r(space, 1) & 0x02)
	{
		m_bg_tilemap[1]->set_scroll_rows(32);
		for (i = 0; i < 32; i++)
			m_bg_tilemap[1]->set_scrollx(i, m_scrollram1[i]);
	}
	else
	{
		m_bg_tilemap[1]->set_scroll_rows(1);
		m_bg_tilemap[1]->set_scrollx(0, m_k007121_2->ctrlram_r(space, 0) | ((m_k007121_2->ctrlram_r(space, 1) & 0x01) << 8));
	}

	m_bg_tilemap[0]->set_scrolly(0, m_k007121_1->ctrlram_r(space, 2));
	m_bg_tilemap[1]->set_scrolly(0, m_k007121_2->ctrlram_r(space, 2));

	screen.priority().fill(0, cliprect);

	if (m_priority == 0)
	{
		m_bg_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE | 0, 4);
		m_bg_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE | 1, 8);
		m_bg_tilemap[0]->draw(screen, bitmap, cliprect, 0, 1);
		m_bg_tilemap[0]->draw(screen, bitmap, cliprect, 1, 2);

		/* we use the priority buffer so sprites are drawn front to back */
		draw_sprites(bitmap, cliprect, m_spriteram[1], 1, screen.priority(), 0x0f00);
		draw_sprites(bitmap, cliprect, m_spriteram[0], 0, screen.priority(), 0x4444);
	}
	else
	{
		m_bg_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE | 0, 1);
		m_bg_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE | 1, 2);
		m_bg_tilemap[1]->draw(screen, bitmap, cliprect, 1, 4);
		m_bg_tilemap[1]->draw(screen, bitmap, cliprect, 0, 8);

		/* we use the priority buffer so sprites are drawn front to back */
		draw_sprites(bitmap, cliprect, m_spriteram[1], 1, screen.priority(), 0x0f00);
		draw_sprites(bitmap, cliprect, m_spriteram[0], 0, screen.priority(), 0x4444);
	}

	if (m_k007121_1->ctrlram_r(space, 1) & 0x08)
	{
		for (i = 0; i < 32; i++)
		{
			m_textlayer->set_scrollx(i, m_scrollram0[0x20 + i] ? 0 : TILE_LINE_DISABLED);
			m_textlayer->draw(screen, bitmap, cliprect, 0, 0);
		}
	}

	/* chop the extreme columns if necessary */
	if (m_k007121_1->ctrlram_r(space, 3) & 0x40)
	{
		rectangle clip;

		clip = cliprect;
		clip.max_x = clip.min_x + 7;
		bitmap.fill(0, clip);

		clip = cliprect;
		clip.min_x = clip.max_x - 7;
		bitmap.fill(0, clip);
	}
	return 0;
}








/***************************************************************************

    bootleg Combat School sprites. Each sprite has 5 bytes:

byte #0:    sprite number
byte #1:    y position
byte #2:    x position
byte #3:
    bit 0:      x position (bit 0)
    bits 1..3:  ???
    bit 4:      flip x
    bit 5:      unused?
    bit 6:      sprite bank # (bit 2)
    bit 7:      ???
byte #4:
    bits 0,1:   sprite bank # (bits 0 & 1)
    bits 2,3:   unused?
    bits 4..7:  sprite color

***************************************************************************/

void combatsc_state::bootleg_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, const UINT8 *source, int circuit )
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	gfx_element *gfx = m_gfxdecode->gfx(circuit + 2);

	int limit = circuit ? (space.read_byte(0xc2) * 256 + space.read_byte(0xc3)) : (space.read_byte(0xc0) * 256 + space.read_byte(0xc1));
	const UINT8 *finish;

	source += 0x1000;
	finish = source;
	source += 0x400;
	limit = (0x3400 - limit) / 8;
	if (limit >= 0)
		finish = source - limit * 8;
	source -= 8;

	while (source > finish)
	{
		UINT8 attributes = source[3]; /* PBxF ?xxX */
		{
			int number = source[0];
			int x = source[2] - 71 + (attributes & 0x01)*256;
			int y = 242 - source[1];
			UINT8 color = source[4]; /* CCCC xxBB */

			int bank = (color & 0x03) | ((attributes & 0x40) >> 4);

			number = ((number & 0x02) << 1) | ((number & 0x04) >> 1) | (number & (~6));
			number += 256 * bank;

			color = (circuit * 4) * 16 + (color >> 4);

			/*  hacks to select alternate palettes */
//          if(m_vreg == 0x40 && (attributes & 0x40)) color += 1*16;
//          if(m_vreg == 0x23 && (attributes & 0x02)) color += 1*16;
//          if(m_vreg == 0x66 ) color += 2*16;

				gfx->transpen(bitmap,cliprect,
							number, color,
							attributes & 0x10,0, /* flip */
							x, y, 15 );
		}
		source -= 8;
	}
}

UINT32 combatsc_state::screen_update_combatscb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

	for (i = 0; i < 32; i++)
	{
		m_bg_tilemap[0]->set_scrollx(i, m_io_ram[0x040 + i] + 5);
		m_bg_tilemap[1]->set_scrollx(i, m_io_ram[0x060 + i] + 3);
	}
	m_bg_tilemap[0]->set_scrolly(0, m_io_ram[0x000] + 1);
	m_bg_tilemap[1]->set_scrolly(0, m_io_ram[0x020] + 1);

	if (m_priority == 0)
	{
		m_bg_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		bootleg_draw_sprites(bitmap,cliprect, m_page[0], 0);
		m_bg_tilemap[0]->draw(screen, bitmap, cliprect, 0 ,0);
		bootleg_draw_sprites(bitmap,cliprect, m_page[1], 1);
	}
	else
	{
		m_bg_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		bootleg_draw_sprites(bitmap,cliprect, m_page[0], 0);
		m_bg_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
		bootleg_draw_sprites(bitmap,cliprect, m_page[1], 1);
	}

	m_textlayer->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
