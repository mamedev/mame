// license:BSD-3-Clause
// copyright-holders:Yochizo
/***************************************************************************

Functions to emulate the video hardware of the machine.

TODO
====

Harmonise draw_sprites code and offload it into taitoic.c, leaving this
as short as possible (TC0080VCO has sprites as well as tilemaps in its
address space).

Maybe wait until the Taito Air system is done - also uses TC0080VCO.

Why does syvalion draw_sprites ignore the zoomy value
(using zoomx instead) ???


Sprite ram notes
----------------

BG / chain RAM
-----------------------------------------
[0]  +0         +1
     -xxx xxxx  xxxx xxxx = tile number

[1]  +0         +1
     ---- ----  x--- ---- = flip Y
     ---- ----  -x-- ---- = flip X
     ---- ----  --xx xxxx = color


sprite RAM
--------------------------------------------------------------
 +0         +1         +2         +3
 ---x ----  ---- ----  ---- ----  ---- ---- = unknown
 ---- xx--  ---- ----  ---- ----  ---- ---- = chain y size
 ---- --xx  xxxx xxxx  ---- ----  ---- ---- = sprite x coords
 ---- ----  ---- ----  ---- --xx  xxxx xxxx = sprite y coords

 +4         +5         +6         +7
 --xx xxxx  ---- ----  ---- ----  ---- ---- = zoom x
 ---- ----  --xx xxxx  ---- ----  ---- ---- = zoom y
 ---- ----  ---- ----  ---x xxxx  xxxx xxxx = tile information offset


***************************************************************************/

#include "emu.h"
#include "includes/taito_h.h"


/***************************************************************************
  Screen refresh
***************************************************************************/

void syvalion_state::syvalion_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0x03f8 / 2; offs >= 0; offs -= 0x008 / 2)
	{
		m_tc0080vco->get_sprite_params(offs, false);

		if (m_tc0080vco->get_sprite_tile_offs())
		{
			m_tc0080vco->draw_single_sprite(bitmap, cliprect);
		}
	}
}

void taitoh_state::recordbr_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority)
{
	for (int offs = 0x03f8 / 2; offs >= 0; offs -= 0x008 / 2)
	{
		if (offs <  0x01b0 && priority == 0)    continue;
		if (offs >= 0x01b0 && priority == 1)    continue;

		m_tc0080vco->get_sprite_params(offs, true);

		if (m_tc0080vco->get_sprite_tile_offs())
		{
			m_tc0080vco->draw_single_sprite(bitmap, cliprect);
		}
	}
}

void taitoh_state::dleague_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority)
{
	for (int offs = 0x03f8 / 2; offs >= 0; offs -= 0x008 / 2)
	{
		m_tc0080vco->get_sprite_params(offs, false);
		int pribit = (m_tc0080vco->sprram_r(offs + 0) & 0x1000) >> 12;

		if (m_tc0080vco->get_sprite_tile_offs())
		{
			if (m_tc0080vco->get_sprite_zoomx() < 63)
				pribit = 0;

			if (m_tc0080vco->scrram_r(0x0002) & 0x8000)
				pribit = 1;

			if (priority == pribit)
			{
				m_tc0080vco->draw_single_sprite(bitmap, cliprect);
			}
		}
	}
}



void taitoh_state::taitoh_log_vram()
{
#ifdef MAME_DEBUG
	// null function: the necessary pointers are now internal to taitoic.c
	// Recreate it there if wanted (add prototype to taitoic.h)
#endif
}


/**************************************************************************/

u32 syvalion_state::screen_update_syvalion(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tc0080vco->tilemap_update();

	taitoh_log_vram();

	bitmap.fill(0, cliprect);

	m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 0, TILEMAP_DRAW_OPAQUE, 0);
	m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 1, 0, 0);
	syvalion_draw_sprites(bitmap,cliprect);
	m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 2, 0, 0);

	return 0;
}


u32 taitoh_state::screen_update_recordbr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tc0080vco->tilemap_update();

	taitoh_log_vram();

	bitmap.fill(0, cliprect);

#ifdef MAME_DEBUG
	if (!machine().input().code_pressed(KEYCODE_A))
		m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 0, TILEMAP_DRAW_OPAQUE, 0);
	if (!machine().input().code_pressed(KEYCODE_S))
		recordbr_draw_sprites(bitmap, cliprect, 0);
	if (!machine().input().code_pressed(KEYCODE_D))
		m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 1, 0, 0);
	if (!machine().input().code_pressed(KEYCODE_F))
		recordbr_draw_sprites(bitmap, cliprect, 1);
#else
	m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 0, TILEMAP_DRAW_OPAQUE, 0);
	recordbr_draw_sprites(bitmap, cliprect, 0);
	m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 1, 0, 0);
	recordbr_draw_sprites(bitmap, cliprect, 1);
#endif

	m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 2, 0, 0);
	return 0;
}


u32 taitoh_state::screen_update_dleague(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tc0080vco->tilemap_update();

	taitoh_log_vram();

	bitmap.fill(0, cliprect);

#ifdef MAME_DEBUG
	if (!machine().input().code_pressed(KEYCODE_A))
		m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 0, TILEMAP_DRAW_OPAQUE, 0);
	if (!machine().input().code_pressed(KEYCODE_S))
		dleague_draw_sprites(bitmap, cliprect, 0);
	if (!machine().input().code_pressed(KEYCODE_D))
		m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 1, 0, 0);
	if (!machine().input().code_pressed(KEYCODE_F))
		dleague_draw_sprites(bitmap, cliprect, 1);
#else
	m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 0, TILEMAP_DRAW_OPAQUE, 0);
	dleague_draw_sprites (bitmap, cliprect, 0);
	m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 1, 0, 0);
	dleague_draw_sprites (bitmap, cliprect, 1);
#endif

	m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 2, 0, 0);
	return 0;
}
