// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Manuel Abadia
/***************************************************************************

  combatsc.cpp

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"

#include "combatsc.h"

void combatsc_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int pal = 0; pal < 8; pal++)
	{
		int clut;

		switch (pal)
		{
		default:
		case 0: // other sprites
		case 2: // other sprites(alt)
			clut = 1;   // 0 is wrong for Firing Range III targets
			break;

		case 4: // player sprites
		case 6: // player sprites(alt)
			clut = 2;
			break;

		case 1: // background
		case 3: // background(alt)
			clut = 1;
			break;

		case 5: // foreground tiles
		case 7: // foreground tiles(alt)
			clut = 3;
			break;
		}

		for (int i = 0; i < 0x100; i++)
		{
			uint8_t ctabentry;

			if (((pal & 0x01) == 0) && (color_prom[(clut << 8) | i] == 0))
				ctabentry = 0;
			else
				ctabentry = (pal << 4) | (color_prom[(clut << 8) | i] & 0x0f);

			palette.set_pen_indirect((pal << 8) | i, ctabentry);
		}
	}
}


void combatscb_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int pal = 0; pal < 8; pal++)
	{
		for (int i = 0; i < 0x100; i++)
		{
			uint8_t ctabentry;

			if ((pal & 1) == 0)
				// sprites
				ctabentry = (pal << 4) | (~color_prom[i] & 0x0f);
			else
				// chars - no lookup?
				ctabentry = (pal << 4) | (i & 0x0f);    // no lookup?

			palette.set_pen_indirect((pal << 8) | i, ctabentry);
		}
	}
}



/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(combatsc_state::get_tile_info0)
{
	uint8_t ctrl_6 = m_k007121[0]->ctrlram_r(6);
	uint8_t attributes = m_videoram[0][tile_index];
	int bank = 4 * ((m_vreg & 0x0f) - 1);
	int number, color;

	if (bank < 0)
		bank = 0;

	if ((attributes & 0xb0) == 0)
		bank = 0;   // text bank

	if (attributes & 0x80)
		bank += 1;

	if (attributes & 0x10)
		bank += 2;

	if (attributes & 0x20)
		bank += 4;

	color = ((ctrl_6 & 0x10) * 2 + 16) + (attributes & 0x0f);

	number = m_videoram[0][tile_index + 0x400] + 256 * bank;

	tileinfo.set(0,
			number,
			color,
			0);
	tileinfo.category = (attributes & 0x40) >> 6;
}

TILE_GET_INFO_MEMBER(combatsc_state::get_tile_info1)
{
	uint8_t ctrl_6 = m_k007121[1]->ctrlram_r(6);
	uint8_t attributes = m_videoram[1][tile_index];
	int bank = 4 * ((m_vreg >> 4) - 1);
	int number, color;

	if (bank < 0)
		bank = 0;

	if ((attributes & 0xb0) == 0)
		bank = 0;   // text bank

	if (attributes & 0x80)
		bank += 1;

	if (attributes & 0x10)
		bank += 2;

	if (attributes & 0x20)
		bank += 4;

	color = ((ctrl_6 & 0x10) * 2 + 16 + 4 * 16) + (attributes & 0x0f);

	number = m_videoram[1][tile_index + 0x400] + 256 * bank;

	tileinfo.set(1,
			number,
			color,
			0);
	tileinfo.category = (attributes & 0x40) >> 6;
}

TILE_GET_INFO_MEMBER(combatsc_state::get_text_info)
{
	uint8_t attributes = m_videoram[0][tile_index + 0x800];
	int number = m_videoram[0][tile_index + 0xc00];
	int color = 16 + (attributes & 0x0f);

	tileinfo.set(0,
			number,
			color,
			0);
}


TILE_GET_INFO_MEMBER(combatscb_state::get_tile_info0)
{
	uint8_t attributes = m_videoram[0][tile_index];
	int bank = 4 * ((m_vreg & 0x0f) - 1);
	int number, pal, color;

	if (bank < 0)
		bank = 0;

	if ((attributes & 0xb0) == 0)
		bank = 0;   // text bank

	if (attributes & 0x80)
		bank += 1;

	if (attributes & 0x10)
		bank += 2;

	if (attributes & 0x20)
		bank += 4;

	pal = (bank == 0 || bank >= 0x1c || (attributes & 0x40)) ? 1 : 3;
	color = pal*16;// + (attributes & 0x0f);
	number = m_videoram[0][tile_index + 0x400] + 256 * bank;

	tileinfo.set(0,
			number,
			color,
			0);
}

TILE_GET_INFO_MEMBER(combatscb_state::get_tile_info1)
{
	uint8_t attributes = m_videoram[1][tile_index];
	int bank = 4*((m_vreg >> 4) - 1);
	int number, pal, color;

	if (bank < 0)
		bank = 0;

	if ((attributes & 0xb0) == 0)
		bank = 0;   // text bank

	if (attributes & 0x80)
		bank += 1;

	if (attributes & 0x10)
		bank += 2;

	if (attributes & 0x20)
		bank += 4;

	pal = (bank == 0 || bank >= 0x1c || (attributes & 0x40)) ? 5 : 7;
	color = pal * 16;// + (attributes & 0x0f);
	number = m_videoram[1][tile_index + 0x400] + 256 * bank;

	tileinfo.set(1,
			number,
			color,
			0);
}

TILE_GET_INFO_MEMBER(combatscb_state::get_text_info)
{
//  uint8_t attributes = m_videoram[0][tile_index + 0x800];
	int number = m_videoram[0][tile_index + 0xc00];
	int color = 16;// + (attributes & 0x0f);

	tileinfo.set(1,
			number,
			color,
			0);
}

/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void combatsc_state::video_start()
{
	m_bg_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(combatsc_state::get_tile_info0)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(combatsc_state::get_tile_info1)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_textlayer =  &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(combatsc_state::get_text_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_spriteram[0] = make_unique_clear<uint8_t[]>(0x800);
	m_spriteram[1] = make_unique_clear<uint8_t[]>(0x800);

	m_bg_tilemap[0]->set_transparent_pen(0);
	m_bg_tilemap[1]->set_transparent_pen(0);
	m_textlayer->set_transparent_pen(0);

	m_textlayer->set_scroll_rows(32);

	save_pointer(NAME(m_spriteram[0]), 0x800);
	save_pointer(NAME(m_spriteram[1]), 0x800);
	save_item(NAME(m_textflip));
}

void combatscb_state::video_start()
{
	m_bg_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(combatscb_state::get_tile_info0)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(combatscb_state::get_tile_info1)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_textlayer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(combatscb_state::get_text_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_spriteram[0] = make_unique_clear<uint8_t[]>(0x800);
	m_spriteram[1] = make_unique_clear<uint8_t[]>(0x800);

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

void combatsc_base_state::videoview0_w(offs_t offset, uint8_t data)
{
	m_videoram[0][offset] = data;

	if (offset < 0x800)
	{
		m_bg_tilemap[0]->mark_tile_dirty(offset & 0x3ff);
	}
	else if (offset < 0x1000)
	{
		m_textlayer->mark_tile_dirty(offset & 0x3ff);
	}
}

void combatsc_base_state::videoview1_w(offs_t offset, uint8_t data)
{
	m_videoram[1][offset] = data;

	if (offset < 0x800)
	{
		m_bg_tilemap[1]->mark_tile_dirty(offset & 0x3ff);
	}
}

void combatsc_state::pf_control_w(offs_t offset, uint8_t data)
{
	k007121_device *k007121 = m_video_circuit ? m_k007121[1] : m_k007121[0];
	k007121->ctrl_w(offset, data);

	if (offset == 7)
	{
		m_bg_tilemap[m_video_circuit]->set_flip((data & 0x08) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
		if (m_video_circuit == 0)
		{
			m_textflip = (data & 0x08) == 0x08;
			m_textlayer->set_flip((data & 0x08) ? TILEMAP_FLIPY | TILEMAP_FLIPX : 0);
		}
	}
	if (offset == 3)
	{
		if (data & 0x08)
			memcpy(m_spriteram[m_video_circuit].get(), m_videoram[m_video_circuit] + 0x1000, 0x800);
		else
			memcpy(m_spriteram[m_video_circuit].get(), m_videoram[m_video_circuit] + 0x1800, 0x800);
	}
}

/***************************************************************************

    Display Refresh

***************************************************************************/

void combatsc_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, const uint8_t *source, int circuit, bitmap_ind8 &priority_bitmap, uint32_t pri_mask)
{
	k007121_device *k007121 = circuit ? m_k007121[1] : m_k007121[0];
	int base_color = (circuit * 4) * 16 + (k007121->ctrlram_r(6) & 0x10) * 2;

	k007121->sprites_draw(bitmap, cliprect, m_gfxdecode->gfx(circuit), *m_palette, source, base_color, 0, 0, priority_bitmap, pri_mask);
}


uint32_t combatsc_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_k007121[0]->ctrlram_r(1) & 0x02)
	{
		m_bg_tilemap[0]->set_scroll_rows(32);
		for (int i = 0; i < 32; i++)
			m_bg_tilemap[0]->set_scrollx(i, m_scrollram[0][i]);
	}
	else
	{
		m_bg_tilemap[0]->set_scroll_rows(1);
		m_bg_tilemap[0]->set_scrollx(0, m_k007121[0]->ctrlram_r(0) | ((m_k007121[0]->ctrlram_r(1) & 0x01) << 8));
	}

	if (m_k007121[1]->ctrlram_r(1) & 0x02)
	{
		m_bg_tilemap[1]->set_scroll_rows(32);
		for (int i = 0; i < 32; i++)
			m_bg_tilemap[1]->set_scrollx(i, m_scrollram[1][i]);
	}
	else
	{
		m_bg_tilemap[1]->set_scroll_rows(1);
		m_bg_tilemap[1]->set_scrollx(0, m_k007121[1]->ctrlram_r(0) | ((m_k007121[1]->ctrlram_r(1) & 0x01) << 8));
	}

	m_bg_tilemap[0]->set_scrolly(0, m_k007121[0]->ctrlram_r(2));
	m_bg_tilemap[1]->set_scrolly(0, m_k007121[1]->ctrlram_r(2));

	screen.priority().fill(0, cliprect);

	if (m_priority == 0)
	{
		m_bg_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE | 0, 4);
		m_bg_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE | 1, 8);
		m_bg_tilemap[0]->draw(screen, bitmap, cliprect, 0, 1);
		m_bg_tilemap[0]->draw(screen, bitmap, cliprect, 1, 2);

		// we use the priority buffer so sprites are drawn front to back
		draw_sprites(bitmap, cliprect, m_spriteram[1].get(), 1, screen.priority(), 0x0f00);
		draw_sprites(bitmap, cliprect, m_spriteram[0].get(), 0, screen.priority(), 0x4444);
	}
	else
	{
		m_bg_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE | 0, 1);
		m_bg_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE | 1, 2);

		// we use the priority buffer so sprites are drawn front to back
		// drill sergeant ribbons goes here, MT #06259
		draw_sprites(bitmap, cliprect, m_spriteram[1].get(), 1, screen.priority(), 0x0f00);
		// guess: move the face as well (should go behind hands but it isn't tested)
		draw_sprites(bitmap, cliprect, m_spriteram[0].get(), 0, screen.priority(), 0x4444);

		m_bg_tilemap[1]->draw(screen, bitmap, cliprect, 1, 4);
		m_bg_tilemap[1]->draw(screen, bitmap, cliprect, 0, 8);
	}

	//if (m_k007121[0]->ctrlram_r(1) & 0x08)
	{
		rectangle clip;
		clip = cliprect;

		for (int i = 0; i < 32; i++)
		{
			// scrollram [0x20]-[0x3f]: char enable (presumably bit 0 only)
			uint8_t base_scroll = m_textflip == true ? (0x3f - i) : (0x20 + i);
			auto slot = m_scroll_view.entry();
			if (m_scrollram[*slot][base_scroll] == 0)
				continue;


			clip.min_y = i * 8;
			clip.max_y = clip.min_y + 7;

			// bit 3 of reg [1] selects if tiles are opaque or have transparent pen.
			m_textlayer->draw(screen, bitmap, clip, m_k007121[0]->ctrlram_r(1) & 0x08 ? TILEMAP_DRAW_OPAQUE : 0, 0);
		}
	}

	// chop the extreme columns if necessary
	if (m_k007121[0]->ctrlram_r(3) & 0x40)
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

void combatscb_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, const uint8_t *source, int circuit)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	gfx_element *gfx = m_gfxdecode->gfx(circuit + 2);

	int limit = circuit ? (space.read_byte(0xc2) * 256 + space.read_byte(0xc3)) : (space.read_byte(0xc0) * 256 + space.read_byte(0xc1));
	const uint8_t *finish;

	source += 0x1000;
	finish = source;
	source += 0x400;
	limit = (0x3400 - limit) / 8;
	if (limit >= 0)
		finish = source - limit * 8;
	source -= 8;

	while (source > finish)
	{
		uint8_t attributes = source[3]; // PBxF ?xxX
		{
			int number = source[0];
			int x = source[2] - 71 + (attributes & 0x01)*256;
			int y = 242 - source[1];
			uint8_t color = source[4]; // CCCC xxBB

			int bank = (color & 0x03) | ((attributes & 0x40) >> 4);

			number = ((number & 0x02) << 1) | ((number & 0x04) >> 1) | (number & (~6));
			number += 256 * bank;

			color = (circuit * 4) * 16 + (color >> 4);

			//  hacks to select alternate palettes
//          if(m_vreg == 0x40 && (attributes & 0x40)) color += 1*16;
//          if(m_vreg == 0x23 && (attributes & 0x02)) color += 1*16;
//          if(m_vreg == 0x66 ) color += 2*16;

				gfx->transpen(bitmap,cliprect,
							number, color,
							attributes & 0x10,0, // flip
							x, y, 15 );
		}
		source -= 8;
	}
}

uint32_t combatscb_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int i = 0; i < 32; i++)
	{
		m_bg_tilemap[0]->set_scrollx(i, m_io_ram[0x040 + i] + 5);
		m_bg_tilemap[1]->set_scrollx(i, m_io_ram[0x060 + i] + 3);
	}
	m_bg_tilemap[0]->set_scrolly(0, m_io_ram[0x000] + 1);
	m_bg_tilemap[1]->set_scrolly(0, m_io_ram[0x020] + 1);

	if (m_priority == 0)
	{
		m_bg_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		draw_sprites(bitmap, cliprect, m_videoram[0], 0);
		m_bg_tilemap[0]->draw(screen, bitmap, cliprect, 0 ,0);
		draw_sprites(bitmap, cliprect, m_videoram[1], 1);
	}
	else
	{
		m_bg_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		draw_sprites(bitmap, cliprect, m_videoram[0], 0);
		m_bg_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
		draw_sprites(bitmap, cliprect, m_videoram[1], 1);
	}

	m_textlayer->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
