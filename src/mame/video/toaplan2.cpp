// license:BSD-3-Clause
// copyright-holders:Quench, Yochizo, David Haywood
/***************************************************************************

 Functions to emulate additional video hardware on several Toaplan2 games.
 The main video is handled by the GP9001 (see video/gp9001.c)

 Extra-text RAM format

 Truxton 2, Fixeight and Raizing games have an extra-text layer.

  Text RAM format      $0000-1FFF (actually its probably $0000-0FFF)
  ---- --xx xxxx xxxx = Tile number
  xxxx xx-- ---- ---- = Color (0 - 3Fh) + 40h

  Line select / flip   $0000-01EF (some games go to $01FF (excess?))
  ---x xxxx xxxx xxxx = Line select for each line
  x--- ---- ---- ---- = X flip for each line ???

  Line scroll          $0000-01EF (some games go to $01FF (excess?))
  ---- ---x xxxx xxxx = X scroll for each line


***************************************************************************/

#include "emu.h"
#include "includes/toaplan2.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(toaplan2_state::get_text_tile_info)
{
	const u16 attrib = m_tx_videoram[tile_index];
	const u32 tile_number = attrib & 0x3ff;
	const u32 color = attrib >> 10;
	SET_TILE_INFO_MEMBER(0,
			tile_number,
			color,
			0);
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/


void toaplan2_state::create_tx_tilemap(int dx, int dx_flipped)
{
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(toaplan2_state::get_text_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_tx_tilemap->set_scroll_rows(8*32); /* line scrolling */
	m_tx_tilemap->set_scroll_cols(1);
	m_tx_tilemap->set_scrolldx(dx, dx_flipped);
	m_tx_tilemap->set_transparent_pen(0);
}

void toaplan2_state::device_post_load()
{
	if (m_tx_gfxram != nullptr)
		m_gfxdecode->gfx(0)->mark_all_dirty();
}

VIDEO_START_MEMBER(toaplan2_state,toaplan2)
{
	/* our current VDP implementation needs this bitmap to work with */
	m_screen->register_screen_bitmap(m_custom_priority_bitmap);

	if (m_vdp[0] != nullptr)
	{
		m_secondary_render_bitmap.reset();
		m_vdp[0]->custom_priority_bitmap = &m_custom_priority_bitmap;
	}

	if (m_vdp[1] != nullptr)
	{
		m_screen->register_screen_bitmap(m_secondary_render_bitmap);
		m_vdp[1]->custom_priority_bitmap = &m_custom_priority_bitmap;
	}
}

VIDEO_START_MEMBER(toaplan2_state,truxton2)
{
	VIDEO_START_CALL_MEMBER(toaplan2);

	/* Create the Text tilemap for this game */
	m_gfxdecode->gfx(0)->set_source(reinterpret_cast<u8 *>(m_tx_gfxram.target()));

	create_tx_tilemap(0x1d5, 0x16a);
}

VIDEO_START_MEMBER(toaplan2_state,fixeightbl)
{
	VIDEO_START_CALL_MEMBER(toaplan2);

	/* Create the Text tilemap for this game */
	create_tx_tilemap();

	/* This bootleg has additional layer offsets on the VDP */
	m_vdp[0]->set_tm_extra_offsets(0, -0x1d6 - 26, -0x1ef - 15, 0, 0);
	m_vdp[0]->set_tm_extra_offsets(1, -0x1d8 - 22, -0x1ef - 15, 0, 0);
	m_vdp[0]->set_tm_extra_offsets(2, -0x1da - 18, -0x1ef - 15, 0, 0);
	m_vdp[0]->set_sp_extra_offsets(8/*-0x1cc - 64*/, 8/*-0x1ef - 128*/, 0, 0);

	m_vdp[0]->init_scroll_regs();
}

VIDEO_START_MEMBER(toaplan2_state,bgaregga)
{
	VIDEO_START_CALL_MEMBER(toaplan2);

	/* Create the Text tilemap for this game */
	create_tx_tilemap(0x1d4, 0x16b);
}

VIDEO_START_MEMBER(toaplan2_state,bgareggabl)
{
	VIDEO_START_CALL_MEMBER(toaplan2);

	/* Create the Text tilemap for this game */
	create_tx_tilemap(4, 4);
}

VIDEO_START_MEMBER(toaplan2_state,batrider)
{
	VIDEO_START_CALL_MEMBER(toaplan2);

	m_vdp[0]->disable_sprite_buffer(); // disable buffering on this game

	/* Create the Text tilemap for this game */
	m_gfxdecode->gfx(0)->set_source(reinterpret_cast<u8 *>(m_tx_gfxram.target()));

	create_tx_tilemap(0x1d4, 0x16b);

	/* Has special banking */
	save_item(NAME(m_gfxrom_bank));
}

void toaplan2_state::tx_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_tx_videoram[offset]);
	if (offset < 64*32)
		m_tx_tilemap->mark_tile_dirty(offset);
}

void toaplan2_state::tx_linescroll_w(offs_t offset, u16 data, u16 mem_mask)
{
	/*** Line-Scroll RAM for Text Layer ***/
	COMBINE_DATA(&m_tx_linescroll[offset]);

	m_tx_tilemap->set_scrollx(offset, m_tx_linescroll[offset]);
}

void toaplan2_state::tx_gfxram_w(offs_t offset, u16 data, u16 mem_mask)
{
	/*** Dynamic GFX decoding for Truxton 2 / FixEight ***/

	const u16 oldword = m_tx_gfxram[offset];

	if (oldword != data)
	{
		COMBINE_DATA(&m_tx_gfxram[offset]);
		m_gfxdecode->gfx(0)->mark_dirty(offset/32);
	}
}

void toaplan2_state::batrider_tx_gfxram_w(offs_t offset, u16 data, u16 mem_mask)
{
	/*** Dynamic GFX decoding for Batrider / Battle Bakraid ***/

	const u16 oldword = m_tx_gfxram[offset];

	if (oldword != data)
	{
		COMBINE_DATA(&m_tx_gfxram[offset]);
		m_gfxdecode->gfx(0)->mark_dirty(offset/16);
	}
}

void toaplan2_state::batrider_textdata_dma_w(u16 data)
{
	/*** Dynamic Text GFX decoding for Batrider ***/
	/*** Only done once during start-up ***/
	m_dma_space->set_bank(1);
	for (int i = 0; i < (0x8000 >> 1); i++)
	{
		m_dma_space->write16(i, m_mainram[i]);
	}
}

void toaplan2_state::batrider_pal_text_dma_w(u16 data)
{
	// FIXME: In batrider and bbakraid, the text layer and palette RAM
	// are probably DMA'd from main RAM by writing here at every vblank,
	// rather than being directly accessible to the 68K like the other games
	m_dma_space->set_bank(0);
	for (int i = 0; i < (0x3400 >> 1); i++)
	{
		m_dma_space->write16(i, m_mainram[i]);
	}
}

void toaplan2_state::batrider_objectbank_w(offs_t offset, u8 data)
{
	data &= 0xf;
	if (m_gfxrom_bank[offset] != data)
	{
		m_gfxrom_bank[offset] = data;
		m_vdp[0]->set_dirty();
	}
}

void toaplan2_state::batrider_bank_cb(u8 layer, u32 &code)
{
	code = (m_gfxrom_bank[code >> 15] << 15) | (code & 0x7fff);
}

// Dogyuun doesn't appear to require fancy mixing?
u32 toaplan2_state::screen_update_dogyuun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	if (m_vdp[1])
	{
		m_custom_priority_bitmap.fill(0, cliprect);
		m_vdp[1]->render_vdp(bitmap, cliprect);
	}
	if (m_vdp[0])
	{
		m_custom_priority_bitmap.fill(0, cliprect);
		m_vdp[0]->render_vdp(bitmap, cliprect);
	}

	return 0;
}


// renders to 2 bitmaps, and mixes output
u32 toaplan2_state::screen_update_batsugun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
//  bitmap.fill(0, cliprect);
//  gp9001_custom_priority_bitmap->fill(0, cliprect);

	if (m_vdp[0])
	{
		bitmap.fill(0, cliprect);
		m_custom_priority_bitmap.fill(0, cliprect);
		m_vdp[0]->render_vdp(bitmap, cliprect);
	}
	if (m_vdp[1])
	{
		m_secondary_render_bitmap.fill(0, cliprect);
		m_custom_priority_bitmap.fill(0, cliprect);
		m_vdp[1]->render_vdp(m_secondary_render_bitmap, cliprect);
	}

	// key test places in batsugun
	// level 2 - the two layers of clouds (will appear under background, or over ships if wrong)
	// level 3 - the special effect 'layer' which should be under everything (will appear over background if wrong)
	// level 4(?) - the large clouds (will obscure player if wrong)
	// high score entry - letters will be missing if wrong
	// end credits - various issues if wrong, clouds like level 2
	//
	// when implemented based directly on the PAL equation it doesn't work, however, my own equations roughly based
	// on that do.
	//

	if (m_vdp[0] && m_vdp[1])
	{
		for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			u16* src_vdp0 = &bitmap.pix16(y);
			const u16* src_vdp1 = &m_secondary_render_bitmap.pix16(y);

			for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
			{
				const u16 GPU0_LUTaddr = src_vdp0[x];
				const u16 GPU1_LUTaddr = src_vdp1[x];

				// these equations is derived from the PAL, but doesn't seem to work?

				const bool COMPARISON = ((GPU0_LUTaddr & 0x0780) > (GPU1_LUTaddr & 0x0780));

				// note: GPU1_LUTaddr & 0x000f - transparency check for vdp1? (gfx are 4bpp, the low 4 bits of the lookup would be the pixel data value)
#if 0
				int result =
							((GPU0_LUTaddr & 0x0008) & !COMPARISON)
						| ((GPU0_LUTaddr & 0x0008) & !(GPU1_LUTaddr & 0x000f))
						| ((GPU0_LUTaddr & 0x0004) & !COMPARISON)
						| ((GPU0_LUTaddr & 0x0004) & !(GPU1_LUTaddr & 0x000f))
						| ((GPU0_LUTaddr & 0x0002) & !COMPARISON)
						| ((GPU0_LUTaddr & 0x0002) & !(GPU1_LUTaddr & 0x000f))
						| ((GPU0_LUTaddr & 0x0001) & !COMPARISON)
						| ((GPU0_LUTaddr & 0x0001) & !(GPU1_LUTaddr & 0x000f));

				if (result) src_vdp0[x] = GPU0_LUTaddr;
				else src_vdp0[x] = GPU1_LUTaddr;
#endif
				// this seems to work tho?
				if (!(GPU1_LUTaddr & 0x000f))
				{
					src_vdp0[x] = GPU0_LUTaddr;
				}
				else
				{
					if (!(GPU0_LUTaddr & 0x000f))
					{
						src_vdp0[x] = GPU1_LUTaddr; // bg pen
					}
					else
					{
						if (COMPARISON)
						{
							src_vdp0[x] = GPU1_LUTaddr;
						}
						else
						{
							src_vdp0[x] = GPU0_LUTaddr;
						}

					}
				}
			}
		}
	}

	return 0;
}


u32 toaplan2_state::screen_update_toaplan2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	m_custom_priority_bitmap.fill(0, cliprect);
	m_vdp[0]->render_vdp(bitmap, cliprect);

	return 0;
}


/* fixeightbl and bgareggabl do not use the lineselect or linescroll tables */
u32 toaplan2_state::screen_update_bootleg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen_update_toaplan2(screen, bitmap, cliprect);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0);
	return 0;
}


u32 toaplan2_state::screen_update_truxton2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen_update_toaplan2(screen, bitmap, cliprect);

	rectangle clip = cliprect;

	/* it seems likely that flipx can be set per line! */
	/* however, none of the games does it, and emulating it in the */
	/* MAME tilemap system without being ultra slow would be tricky */
	m_tx_tilemap->set_flip(m_tx_lineselect[0] & 0x8000 ? 0 : TILEMAP_FLIPX);

	/* line select is used for 'for use in' and '8ing' screen on bbakraid, 'Raizing' logo on batrider */
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		clip.min_y = clip.max_y = y;
		m_tx_tilemap->set_scrolly(0, m_tx_lineselect[y] - y);
		m_tx_tilemap->draw(screen, bitmap, clip, 0);
	}
	return 0;
}


WRITE_LINE_MEMBER(toaplan2_state::screen_vblank)
{
	// rising edge
	if (state)
	{
		if (m_vdp[0]) m_vdp[0]->screen_eof();
		if (m_vdp[1]) m_vdp[1]->screen_eof();
	}
}
