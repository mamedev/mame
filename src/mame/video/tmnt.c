// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "includes/tmnt.h"

TILE_GET_INFO_MEMBER(tmnt_state::glfgreat_get_roz_tile_info)
{
	UINT8 *rom = memregion("user1")->base();
	int code;

	tile_index += 0x40000 * m_glfgreat_roz_rom_bank;

	code = rom[tile_index + 0x80000] + 256 * rom[tile_index] + 256 * 256 * ((rom[tile_index / 4 + 0x100000] >> (2 * (tile_index & 3))) & 3);

	SET_TILE_INFO_MEMBER(0, code & 0x3fff, code >> 14, 0);
}

TILE_GET_INFO_MEMBER(tmnt_state::prmrsocr_get_roz_tile_info)
{
	UINT8 *rom = memregion("user1")->base();
	int code = rom[tile_index + 0x20000] + 256 * rom[tile_index];

	SET_TILE_INFO_MEMBER(0, code & 0x1fff, code >> 13, 0);
}



/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

/* Missing in Action */

K052109_CB_MEMBER(tmnt_state::mia_tile_callback)
{
	*flags = (*color & 0x04) ? TILE_FLIPX : 0;
	if (layer == 0)
	{
		*code |= ((*color & 0x01) << 8);
		*color = m_layer_colorbase[layer] + ((*color & 0x80) >> 5) + ((*color & 0x10) >> 1);
	}
	else
	{
		*code |= ((*color & 0x01) << 8) | ((*color & 0x18) << 6) | (bank << 11);
		*color = m_layer_colorbase[layer] + ((*color & 0xe0) >> 5);
	}
}

K052109_CB_MEMBER(tmnt_state::cuebrick_tile_callback)
{
	if ((m_k052109->get_rmrd_line() == CLEAR_LINE) && (layer == 0))
	{
		*code |= ((*color & 0x01) << 8);
		*color = m_layer_colorbase[layer]  + ((*color & 0x0e) >> 1);
	}
	else
	{
		*code |= ((*color & 0xf) << 8);
		*color = m_layer_colorbase[layer] + ((*color & 0xe0) >> 5);
	}
}

K052109_CB_MEMBER(tmnt_state::tmnt_tile_callback)
{
	*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) | ((*color & 0x0c) << 9) | (bank << 13);
	*color = m_layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}

K052109_CB_MEMBER(tmnt_state::ssbl_tile_callback)
{
	if (layer == 0)
	{
		*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) | ((*color & 0x0c) << 9) | (bank << 13);
	}
	else
	{
		*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) | ((*color & 0x0c) << 9) | (bank << 13);
//      osd_printf_debug("L%d: bank %d code %x color %x\n", layer, bank, *code, *color);
	}

	*color = m_layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}

K052109_CB_MEMBER(tmnt_state::blswhstl_tile_callback)
{
	/* (color & 0x02) is flip y handled internally by the 052109 */
	*code |= ((*color & 0x01) << 8) | ((*color & 0x10) << 5) | ((*color & 0x0c) << 8) | (bank << 12) | m_blswhstl_rombank << 14;
	*color = m_layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}



/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

K051960_CB_MEMBER(tmnt_state::mia_sprite_callback)
{
	*color = m_sprite_colorbase + (*color & 0x0f);
}

K051960_CB_MEMBER(tmnt_state::tmnt_sprite_callback)
{
	*code |= (*color & 0x10) << 9;
	*color = m_sprite_colorbase + (*color & 0x0f);
}

K051960_CB_MEMBER(tmnt_state::punkshot_sprite_callback)
{
	int pri = 0x20 | ((*color & 0x60) >> 2);
	if (pri <= m_layerpri[2])
		*priority = 0;
	else if (pri > m_layerpri[2] && pri <= m_layerpri[1])
		*priority = 0xf0;
	else if (pri > m_layerpri[1] && pri <= m_layerpri[0])
		*priority = 0xf0 | 0xcc;
	else
		*priority = 0xf0 | 0xcc | 0xaa;

	*code |= (*color & 0x10) << 9;
	*color = m_sprite_colorbase + (*color & 0x0f);
}

K051960_CB_MEMBER(tmnt_state::thndrx2_sprite_callback)
{
	int pri = 0x20 | ((*color & 0x60) >> 2);
	if (pri <= m_layerpri[2])
		*priority = 0;
	else if (pri > m_layerpri[2] && pri <= m_layerpri[1])
		*priority = 0xf0;
	else if (pri > m_layerpri[1] && pri <= m_layerpri[0])
		*priority = 0xf0 | 0xcc;
	else
		*priority = 0xf0 | 0xcc | 0xaa;

	*color = m_sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Callbacks for the K053245

***************************************************************************/

K05324X_CB_MEMBER(tmnt_state::lgtnfght_sprite_callback)
{
	int pri = 0x20 | ((*color & 0x60) >> 2);
	if (pri <= m_layerpri[2])
		*priority = 0;
	else if (pri > m_layerpri[2] && pri <= m_layerpri[1])
		*priority = 0xf0;
	else if (pri > m_layerpri[1] && pri <= m_layerpri[0])
		*priority = 0xf0 | 0xcc;
	else
		*priority = 0xf0 | 0xcc | 0xaa;

	*color = m_sprite_colorbase + (*color & 0x1f);
}

K05324X_CB_MEMBER(tmnt_state::blswhstl_sprite_callback)
{
#if 0
if (machine().input().code_pressed(KEYCODE_Q) && (*color & 0x20)) *color = rand();
if (machine().input().code_pressed(KEYCODE_W) && (*color & 0x40)) *color = rand();
if (machine().input().code_pressed(KEYCODE_E) && (*color & 0x80)) *color = rand();
#endif
	int pri = 0x20 | ((*color & 0x60) >> 2);
	if (pri <= m_layerpri[2])
		*priority = 0;
	else if (pri > m_layerpri[2] && pri <= m_layerpri[1])
		*priority = 0xf0;
	else if (pri > m_layerpri[1] && pri <= m_layerpri[0])
		*priority = 0xf0 | 0xcc;
	else
		*priority = 0xf0 | 0xcc | 0xaa;

	*color = m_sprite_colorbase + (*color & 0x1f);
}

K05324X_CB_MEMBER(tmnt_state::prmrsocr_sprite_callback)
{
	int pri = 0x20 | ((*color & 0x60) >> 2);
	if (pri <= m_layerpri[2])
		*priority = 0;
	else if (pri > m_layerpri[2] && pri <= m_layerpri[1])
		*priority = 0xf0;
	else if (pri > m_layerpri[1] && pri <= m_layerpri[0])
		*priority = 0xf0 | 0xcc;
	else
		*priority = 0xf0 | 0xcc | 0xaa;

	*code |= m_prmrsocr_sprite_bank << 14;
	*color = m_sprite_colorbase + (*color & 0x1f);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(tmnt_state,cuebrick)
{
	m_layer_colorbase[0] = 0;
	m_layer_colorbase[1] = 32;
	m_layer_colorbase[2] = 40;
	m_sprite_colorbase = 16;
}

VIDEO_START_MEMBER(tmnt_state,mia)
{
	m_layer_colorbase[0] = 0;
	m_layer_colorbase[1] = 32;
	m_layer_colorbase[2] = 40;
	m_sprite_colorbase = 16;

	m_tmnt_priorityflag = 0;
	save_item(NAME(m_tmnt_priorityflag));
}

VIDEO_START_MEMBER(tmnt_state,tmnt)
{
	m_layer_colorbase[0] = 0;
	m_layer_colorbase[1] = 32;
	m_layer_colorbase[2] = 40;
	m_sprite_colorbase = 16;

	m_tmnt_priorityflag = 0;
	save_item(NAME(m_tmnt_priorityflag));

	m_palette->set_shadow_factor(0.75);
}

VIDEO_START_MEMBER(tmnt_state,lgtnfght)/* also tmnt2, ssriders */
{
	m_k053245->set_z_rejection(0);

	m_dim_c = m_dim_v = m_lastdim = m_lasten = 0;

	save_item(NAME(m_dim_c));
	save_item(NAME(m_dim_v));
	save_item(NAME(m_lastdim));
	save_item(NAME(m_lasten));
}

VIDEO_START_MEMBER(tmnt_state,glfgreat)
{
	m_roz_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tmnt_state::glfgreat_get_roz_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 512, 512);
	m_roz_tilemap->set_transparent_pen(0);

	m_glfgreat_roz_rom_bank = 0;
	m_glfgreat_roz_char_bank = 0;
	m_glfgreat_roz_rom_mode = 0;
	save_item(NAME(m_glfgreat_roz_rom_bank));
	save_item(NAME(m_glfgreat_roz_char_bank));
	save_item(NAME(m_glfgreat_roz_rom_mode));
}

VIDEO_START_MEMBER(tmnt_state,prmrsocr)
{
	m_roz_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tmnt_state::prmrsocr_get_roz_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 512, 256);
	m_roz_tilemap->set_transparent_pen(0);

	m_prmrsocr_sprite_bank = 0;
	m_glfgreat_roz_char_bank = 0;
	save_item(NAME(m_prmrsocr_sprite_bank));
	save_item(NAME(m_glfgreat_roz_char_bank));
}

VIDEO_START_MEMBER(tmnt_state,blswhstl)
{
	m_blswhstl_rombank = -1;
	save_item(NAME(m_blswhstl_rombank));
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_MEMBER(tmnt_state::tmnt_0a0000_w)
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0/1 = coin counters */
		coin_counter_w(machine(), 0, data & 0x01);
		coin_counter_w(machine(), 1, data & 0x02);  /* 2 players version */

		/* bit 3 high then low triggers irq on sound CPU */
		if (m_last == 0x08 && (data & 0x08) == 0)
			m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff);

		m_last = data & 0x08;

		/* bit 5 = irq enable */
		m_irq5_mask = data & 0x20;

		/* bit 7 = enable char ROM reading through the video RAM */
		m_k052109->set_rmrd_line((data & 0x80) ? ASSERT_LINE : CLEAR_LINE);

		/* other bits unused */
	}
}

WRITE16_MEMBER(tmnt_state::punkshot_0a0020_w)
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0 = coin counter */
		coin_counter_w(machine(), 0, data & 0x01);

		/* bit 2 = trigger irq on sound CPU */
		if (m_last == 0x04 && (data & 0x04) == 0)
			m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff);

		m_last = data & 0x04;

		/* bit 3 = enable char ROM reading through the video RAM */
		m_k052109->set_rmrd_line((data & 0x08) ? ASSERT_LINE : CLEAR_LINE);
	}
}

WRITE16_MEMBER(tmnt_state::lgtnfght_0a0018_w)
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0,1 = coin counter */
		coin_counter_w(machine(), 0, data & 0x01);
		coin_counter_w(machine(), 1, data & 0x02);

		/* bit 2 = trigger irq on sound CPU */
		if (m_last == 0x00 && (data & 0x04) == 0x04)
			m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff);

		m_last = data & 0x04;

		/* bit 3 = enable char ROM reading through the video RAM */
		m_k052109->set_rmrd_line((data & 0x08) ? ASSERT_LINE : CLEAR_LINE);
	}
}

WRITE16_MEMBER(tmnt_state::blswhstl_700300_w)
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0,1 = coin counter */
		coin_counter_w(machine(), 0,data & 0x01);
		coin_counter_w(machine(), 1,data & 0x02);

		/* bit 3 = enable char ROM reading through the video RAM */
		m_k052109->set_rmrd_line((data & 0x08) ? ASSERT_LINE : CLEAR_LINE);

		/* bit 7 = select char ROM bank */
		if (m_blswhstl_rombank != ((data & 0x80) >> 7))
		{
			m_blswhstl_rombank = (data & 0x80) >> 7;
			machine().tilemap().mark_all_dirty();
		}

		/* other bits unknown */
	}
}


READ16_MEMBER(tmnt_state::glfgreat_rom_r)
{
	if (m_glfgreat_roz_rom_mode)
		return memregion("zoom")->base()[m_glfgreat_roz_char_bank * 0x80000 + offset];
	else if (offset < 0x40000)
	{
		UINT8 *usr = memregion("user1")->base();
		return usr[offset + 0x80000 + m_glfgreat_roz_rom_bank * 0x40000] + 256 * usr[offset + m_glfgreat_roz_rom_bank * 0x40000];
	}
	else
		return memregion("user1")->base()[((offset & 0x3ffff) >> 2) + 0x100000 + m_glfgreat_roz_rom_bank * 0x10000];
}

WRITE16_MEMBER(tmnt_state::glfgreat_122000_w)
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0,1 = coin counter */
		coin_counter_w(machine(), 0, data & 0x01);
		coin_counter_w(machine(), 1, data & 0x02);

		/* bit 4 = enable char ROM reading through the video RAM */
		m_k052109->set_rmrd_line((data & 0x10) ? ASSERT_LINE : CLEAR_LINE);

		/* bit 5 = 53596 tile rom bank selection */
		if (m_glfgreat_roz_rom_bank != (data & 0x20) >> 5)
		{
			m_glfgreat_roz_rom_bank = (data & 0x20) >> 5;
			m_roz_tilemap->mark_all_dirty();
		}

		/* bit 6,7 = 53596 char bank selection for ROM test */
		m_glfgreat_roz_char_bank = (data & 0xc0) >> 6;

		/* other bits unknown */
	}
	if (ACCESSING_BITS_8_15)
	{
		/* bit 8 = 53596 char/rom selection for ROM test */
		m_glfgreat_roz_rom_mode = data & 0x100;
	}
}


WRITE16_MEMBER(tmnt_state::ssriders_eeprom_w)
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0 is data */
		/* bit 1 is cs (active low) */
		/* bit 2 is clock (active high) */
		ioport("EEPROMOUT")->write(data, 0xff);

		/* bits 3-4 control palette dimming */
		/* 4 = DIMPOL = when set, negate SHAD */
		/* 3 = DIMMOD = when set, or BRIT with [negated] SHAD */
		m_dim_c = data & 0x18;

		/* bit 5 selects sprite ROM for testing in TMNT2 (bits 5-7, actually, according to the schematics) */
		m_k053245->bankselect(((data & 0x20) >> 5) << 2);
	}
}

WRITE16_MEMBER(tmnt_state::ssriders_1c0300_w)
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0,1 = coin counter */
		coin_counter_w(machine(), 0, data & 0x01);
		coin_counter_w(machine(), 1, data & 0x02);

		/* bit 3 = enable char ROM reading through the video RAM */
		m_k052109->set_rmrd_line((data & 0x08) ? ASSERT_LINE : CLEAR_LINE);

		/* bits 4-6 control palette dimming (DIM0-DIM2) */
		m_dim_v = (data & 0x70) >> 4;
	}
}

WRITE16_MEMBER(tmnt_state::prmrsocr_122000_w)
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0,1 = coin counter */
		coin_counter_w(machine(), 0, data & 0x01);
		coin_counter_w(machine(), 1, data & 0x02);

		/* bit 4 = enable char ROM reading through the video RAM */
		m_k052109->set_rmrd_line((data & 0x10) ? ASSERT_LINE : CLEAR_LINE);

		/* bit 6 = sprite ROM bank */
		m_prmrsocr_sprite_bank = (data & 0x40) >> 6;
		m_k053245->bankselect(m_prmrsocr_sprite_bank << 2);

		/* bit 7 = 53596 region selector for ROM test */
		m_glfgreat_roz_char_bank = (data & 0x80) >> 7;

		/* other bits unknown (unused?) */
	}
}

READ16_MEMBER(tmnt_state::prmrsocr_rom_r)
{
	if(m_glfgreat_roz_char_bank)
		return memregion("zoom")->base()[offset];
	else
	{
		UINT8 *usr = memregion("user1")->base();
		return 256 * usr[offset] + usr[offset + 0x020000];
	}
}

WRITE16_MEMBER(tmnt_state::tmnt_priority_w)
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 2/3 = priority; other bits unused */
		/* bit2 = PRI bit3 = PRI2
		      sprite/playfield priority is controlled by these two bits, by bit 3
		      of the background tile color code, and by the SHADOW sprite
		      attribute bit.
		      Priorities are encoded in a PROM (G19 for TMNT). However, in TMNT,
		      the PROM only takes into account the PRI and SHADOW bits.
		      PRI  Priority
		       0   bg fg spr text
		       1   bg spr fg text
		      The SHADOW bit, when set, torns a sprite into a shadow which makes
		      color below it darker (this is done by turning off three resistors
		      in parallel with the RGB output).

		      Note: the background color (color used when all of the four layers
		      are 0) is taken from the *foreground* palette, not the background
		      one as would be more intuitive.
		*/
		m_tmnt_priorityflag = (data & 0x0c) >> 2;
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

UINT32 tmnt_state::screen_update_mia(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_k052109->tilemap_update();

	m_k052109->tilemap_draw(screen, bitmap, cliprect, 2, TILEMAP_DRAW_OPAQUE,0);
	if ((m_tmnt_priorityflag & 1) == 1) m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), 0, 0);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, 0, 0);
	if ((m_tmnt_priorityflag & 1) == 0) m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), 0, 0);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 0, 0, 0);

	return 0;
}

UINT32 tmnt_state::screen_update_tmnt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_k052109->tilemap_update();

	m_k052109->tilemap_draw(screen, bitmap, cliprect, 2, TILEMAP_DRAW_OPAQUE,0);
	if ((m_tmnt_priorityflag & 1) == 1) m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), 0, 0);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, 0, 0);
	if ((m_tmnt_priorityflag & 1) == 0) m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), 0, 0);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 0, 0, 0);

	return 0;
}


UINT32 tmnt_state::screen_update_punkshot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_sprite_colorbase = m_k053251->get_palette_index(K053251_CI1);
	m_layer_colorbase[0] = m_k053251->get_palette_index(K053251_CI2);
	m_layer_colorbase[1] = m_k053251->get_palette_index(K053251_CI4);
	m_layer_colorbase[2] = m_k053251->get_palette_index(K053251_CI3);

	m_k052109->tilemap_update();

	m_sorted_layer[0] = 0;
	m_layerpri[0] = m_k053251->get_priority(K053251_CI2);
	m_sorted_layer[1] = 1;
	m_layerpri[1] = m_k053251->get_priority(K053251_CI4);
	m_sorted_layer[2] = 2;
	m_layerpri[2] = m_k053251->get_priority(K053251_CI3);

	konami_sortlayers3(m_sorted_layer, m_layerpri);

	screen.priority().fill(0, cliprect);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, m_sorted_layer[0], TILEMAP_DRAW_OPAQUE, 1);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, m_sorted_layer[1], 0, 2);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, m_sorted_layer[2], 0, 4);

	m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), -1, -1);
	return 0;
}


UINT32 tmnt_state::screen_update_lgtnfght(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int bg_colorbase;

	bg_colorbase = m_k053251->get_palette_index(K053251_CI0);
	m_sprite_colorbase = m_k053251->get_palette_index(K053251_CI1);
	m_layer_colorbase[0] = m_k053251->get_palette_index(K053251_CI2);
	m_layer_colorbase[1] = m_k053251->get_palette_index(K053251_CI4);
	m_layer_colorbase[2] = m_k053251->get_palette_index(K053251_CI3);

	m_k052109->tilemap_update();

	m_sorted_layer[0] = 0;
	m_layerpri[0] = m_k053251->get_priority(K053251_CI2);
	m_sorted_layer[1] = 1;
	m_layerpri[1] = m_k053251->get_priority(K053251_CI4);
	m_sorted_layer[2] = 2;
	m_layerpri[2] = m_k053251->get_priority(K053251_CI3);

	konami_sortlayers3(m_sorted_layer, m_layerpri);

	screen.priority().fill(0, cliprect);
	bitmap.fill(16 * bg_colorbase, cliprect);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, m_sorted_layer[0], 0, 1);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, m_sorted_layer[1], 0, 2);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, m_sorted_layer[2], 0, 4);

	m_k053245->sprites_draw(bitmap, cliprect, screen.priority());
	return 0;
}


READ16_MEMBER(tmnt_state::glfgreat_ball_r)
{
#ifdef MAME_DEBUG
popmessage("%04x", m_glfgreat_pixel);
#endif
	/* if out of the ROZ layer palette range, it's in the water - return 0 */
	if (m_glfgreat_pixel < 0x400 || m_glfgreat_pixel >= 0x500)
		return 0;
	else
		return m_glfgreat_pixel & 0xff;
}

UINT32 tmnt_state::screen_update_glfgreat(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int bg_colorbase;

	bg_colorbase = m_k053251->get_palette_index(K053251_CI0);
	m_sprite_colorbase  = m_k053251->get_palette_index(K053251_CI1);
	m_layer_colorbase[0] = m_k053251->get_palette_index(K053251_CI2);
	m_layer_colorbase[1] = m_k053251->get_palette_index(K053251_CI3) + 8;   /* weird... */
	m_layer_colorbase[2] = m_k053251->get_palette_index(K053251_CI4);

	m_k052109->tilemap_update();

	m_sorted_layer[0] = 0;
	m_layerpri[0] = m_k053251->get_priority(K053251_CI2);
	m_sorted_layer[1] = 1;
	m_layerpri[1] = m_k053251->get_priority(K053251_CI3);
	m_sorted_layer[2] = 2;
	m_layerpri[2] = m_k053251->get_priority(K053251_CI4);

	konami_sortlayers3(m_sorted_layer, m_layerpri);

	/* not sure about the 053936 priority, but it seems to work */

	screen.priority().fill(0, cliprect);
	bitmap.fill(16 * bg_colorbase, cliprect);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, m_sorted_layer[0], 0, 1);

	if (m_layerpri[0] >= 0x30 && m_layerpri[1] < 0x30)
	{
		m_k053936->zoom_draw(screen, bitmap, cliprect, m_roz_tilemap, 0, 1, 1);
		m_glfgreat_pixel = bitmap.pix16(0x80, 0x105);
	}

	m_k052109->tilemap_draw(screen, bitmap, cliprect, m_sorted_layer[1], 0, 2);

	if (m_layerpri[1] >= 0x30 && m_layerpri[2] < 0x30)
	{
		m_k053936->zoom_draw(screen, bitmap, cliprect, m_roz_tilemap, 0, 1, 1);
		m_glfgreat_pixel = bitmap.pix16(0x80, 0x105);
	}

	m_k052109->tilemap_draw(screen, bitmap, cliprect, m_sorted_layer[2], 0, 4);

	if (m_layerpri[2] >= 0x30)
	{
		m_k053936->zoom_draw(screen, bitmap, cliprect, m_roz_tilemap, 0, 1, 1);
		m_glfgreat_pixel = bitmap.pix16(0x80, 0x105);
	}

	m_k053245->sprites_draw(bitmap, cliprect, screen.priority());
	return 0;
}

UINT32 tmnt_state::screen_update_tmnt2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	double brt;
	int i, newdim, newen, cb, ce;

	newdim = m_dim_v | ((~m_dim_c & 0x10) >> 1);
	newen  = (m_k053251->get_priority(5) && m_k053251->get_priority(5) != 0x3e);

	if (newdim != m_lastdim || newen != m_lasten)
	{
		brt = 1.0;
		if (newen)
			brt -= (1.0 - PALETTE_DEFAULT_SHADOW_FACTOR) * newdim / 8;
		m_lastdim = newdim;
		m_lasten = newen;

		/*
		    Only affect the background and sprites, not text layer.
		    Instead of dimming each layer we dim the entire palette
		    except text colors because palette bases may change
		    anytime and there's no guarantee a dimmed color will be
		    reset properly.
		*/

		// find the text layer's palette range
		cb = m_layer_colorbase[m_sorted_layer[2]] << 4;
		ce = cb + 128;

		// dim all colors before it
		for (i = 0; i < cb; i++)
			m_palette->set_pen_contrast(i, brt);

		// reset all colors in range
		for (i = cb; i < ce; i++)
			m_palette->set_pen_contrast(i, 1.0);

		// dim all colors after it
		for (i = ce; i < 2048; i++)
			m_palette->set_pen_contrast(i, brt);

		// toggle shadow/highlight
		if (~m_dim_c & 0x10)
			m_palette->set_shadow_mode(1);
		else
			m_palette->set_shadow_mode(0);
	}

	screen_update_lgtnfght(screen, bitmap, cliprect);
	return 0;
}


UINT32 tmnt_state::screen_update_thndrx2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int bg_colorbase;

	bg_colorbase = m_k053251->get_palette_index(K053251_CI0);
	m_sprite_colorbase = m_k053251->get_palette_index(K053251_CI1);
	m_layer_colorbase[0] = m_k053251->get_palette_index(K053251_CI2);
	m_layer_colorbase[1] = m_k053251->get_palette_index(K053251_CI4);
	m_layer_colorbase[2] = m_k053251->get_palette_index(K053251_CI3);

	m_k052109->tilemap_update();

	m_sorted_layer[0] = 0;
	m_layerpri[0] = m_k053251->get_priority(K053251_CI2);
	m_sorted_layer[1] = 1;
	m_layerpri[1] = m_k053251->get_priority(K053251_CI4);
	m_sorted_layer[2] = 2;
	m_layerpri[2] = m_k053251->get_priority(K053251_CI3);

	konami_sortlayers3(m_sorted_layer, m_layerpri);

	screen.priority().fill(0, cliprect);
	bitmap.fill(16 * bg_colorbase, cliprect);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, m_sorted_layer[0], 0, 1);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, m_sorted_layer[1], 0, 2);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, m_sorted_layer[2], 0, 4);

	m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), -1, -1);
	return 0;
}



/***************************************************************************

  Housekeeping

***************************************************************************/

void tmnt_state::screen_eof_blswhstl(screen_device &screen, bool state)
{
	// on rising edge
	if (state)
	{
		m_k053245->clear_buffer();
	}
}
