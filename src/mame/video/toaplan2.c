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

#define RAIZING_TX_GFXRAM_SIZE  0x8000  /* GFX data decode RAM size */



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(toaplan2_state::get_text_tile_info)
{
	int color, tile_number, attrib;

	attrib = m_tx_videoram[tile_index];
	tile_number = attrib & 0x3ff;
	color = attrib >> 10;
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
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(toaplan2_state::get_text_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_tx_tilemap->set_scroll_rows(8*32); /* line scrolling */
	m_tx_tilemap->set_scroll_cols(1);
	m_tx_tilemap->set_scrolldx(dx, dx_flipped);
	m_tx_tilemap->set_transparent_pen(0);
}

void toaplan2_state::truxton2_postload()
{
	m_gfxdecode->gfx(0)->mark_all_dirty();
}

VIDEO_START_MEMBER(toaplan2_state,toaplan2)
{
	/* our current VDP implementation needs this bitmap to work with */
	m_screen->register_screen_bitmap(m_custom_priority_bitmap);

	if (m_vdp0 != NULL)
	{
		m_secondary_render_bitmap.reset();
		m_vdp0->custom_priority_bitmap = &m_custom_priority_bitmap;
	}

	if (m_vdp1 != NULL)
	{
		m_screen->register_screen_bitmap(m_secondary_render_bitmap);
		m_vdp1->custom_priority_bitmap = &m_custom_priority_bitmap;
	}
}

VIDEO_START_MEMBER(toaplan2_state,truxton2)
{
	VIDEO_START_CALL_MEMBER( toaplan2 );

	/* Create the Text tilemap for this game */
	m_gfxdecode->gfx(0)->set_source(reinterpret_cast<UINT8 *>(m_tx_gfxram16.target()));
	machine().save().register_postload(save_prepost_delegate(FUNC(toaplan2_state::truxton2_postload), this));

	create_tx_tilemap(0x1d5, 0x16a);
}

VIDEO_START_MEMBER(toaplan2_state,fixeightbl)
{
	VIDEO_START_CALL_MEMBER( toaplan2 );

	/* Create the Text tilemap for this game */
	create_tx_tilemap();

	/* This bootleg has additional layer offsets on the VDP */
	m_vdp0->bg.extra_xoffset.normal  = -0x1d6  -26;
	m_vdp0->bg.extra_yoffset.normal  = -0x1ef  -15;

	m_vdp0->fg.extra_xoffset.normal  = -0x1d8  -22;
	m_vdp0->fg.extra_yoffset.normal  = -0x1ef  -15;

	m_vdp0->top.extra_xoffset.normal = -0x1da  -18;
	m_vdp0->top.extra_yoffset.normal = -0x1ef  -15;

	m_vdp0->sp.extra_xoffset.normal  = 8;//-0x1cc  -64;
	m_vdp0->sp.extra_yoffset.normal  = 8;//-0x1ef  -128;

	m_vdp0->init_scroll_regs();
}

VIDEO_START_MEMBER(toaplan2_state,bgaregga)
{
	VIDEO_START_CALL_MEMBER( toaplan2 );

	/* Create the Text tilemap for this game */
	create_tx_tilemap(0x1d4, 0x16b);
}

VIDEO_START_MEMBER(toaplan2_state,bgareggabl)
{
	VIDEO_START_CALL_MEMBER( toaplan2 );

	/* Create the Text tilemap for this game */
	create_tx_tilemap(4, 4);
}

VIDEO_START_MEMBER(toaplan2_state,batrider)
{
	VIDEO_START_CALL_MEMBER( toaplan2 );

	m_vdp0->sp.use_sprite_buffer = 0; // disable buffering on this game

	/* Create the Text tilemap for this game */
	m_tx_gfxram16.allocate(RAIZING_TX_GFXRAM_SIZE/2);
	m_gfxdecode->gfx(0)->set_source(reinterpret_cast<UINT8 *>(m_tx_gfxram16.target()));
	machine().save().register_postload(save_prepost_delegate(FUNC(toaplan2_state::truxton2_postload), this));

	create_tx_tilemap(0x1d4, 0x16b);

	/* Has special banking */
	m_vdp0->gp9001_gfxrom_is_banked = 1;
}

WRITE16_MEMBER(toaplan2_state::toaplan2_tx_videoram_w)
{
	COMBINE_DATA(&m_tx_videoram[offset]);
	if (offset < 64*32)
		m_tx_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(toaplan2_state::toaplan2_tx_linescroll_w)
{
	/*** Line-Scroll RAM for Text Layer ***/
	COMBINE_DATA(&m_tx_linescroll[offset]);

	m_tx_tilemap->set_scrollx(offset, m_tx_linescroll[offset]);
}

WRITE16_MEMBER(toaplan2_state::toaplan2_tx_gfxram16_w)
{
	/*** Dynamic GFX decoding for Truxton 2 / FixEight ***/

	UINT16 oldword = m_tx_gfxram16[offset];

	if (oldword != data)
	{
		COMBINE_DATA(&m_tx_gfxram16[offset]);
		m_gfxdecode->gfx(0)->mark_dirty(offset/32);
	}
}

WRITE16_MEMBER(toaplan2_state::batrider_textdata_dma_w)
{
	/*** Dynamic Text GFX decoding for Batrider ***/
	/*** Only done once during start-up ***/

	UINT16 *dest = m_tx_gfxram16;

	memcpy(dest, m_tx_videoram, m_tx_videoram.bytes());
	dest += (m_tx_videoram.bytes()/2);
	memcpy(dest, m_paletteram, m_paletteram.bytes());
	dest += (m_paletteram.bytes()/2);
	memcpy(dest, m_tx_lineselect, m_tx_lineselect.bytes());
	dest += (m_tx_lineselect.bytes()/2);
	memcpy(dest, m_tx_linescroll, m_tx_linescroll.bytes());
	dest += (m_tx_linescroll.bytes()/2);
	memcpy(dest, m_mainram16, m_mainram16.bytes());

	m_gfxdecode->gfx(0)->mark_all_dirty();
}

WRITE16_MEMBER(toaplan2_state::batrider_unknown_dma_w)
{
	// FIXME: In batrider and bbakraid, the text layer and palette RAM
	// are probably DMA'd from main RAM by writing here at every vblank,
	// rather than being directly accessible to the 68K like the other games
}

WRITE16_MEMBER(toaplan2_state::batrider_objectbank_w)
{
	if (ACCESSING_BITS_0_7)
	{
		data &= 0xf;
		if (m_vdp0->gp9001_gfxrom_bank[offset] != data)
		{
			m_vdp0->gp9001_gfxrom_bank[offset] = data;
			m_vdp0->gp9001_gfxrom_bank_dirty = 1;
		}
	}
}


// Dogyuun doesn't appear to require fancy mixing?
UINT32 toaplan2_state::screen_update_dogyuun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_vdp1)
	{
		bitmap.fill(0, cliprect);
		m_custom_priority_bitmap.fill(0, cliprect);
		m_vdp1->gp9001_render_vdp(bitmap, cliprect);
	}
	if (m_vdp0)
	{
	//  bitmap.fill(0, cliprect);
		m_custom_priority_bitmap.fill(0, cliprect);
		m_vdp0->gp9001_render_vdp(bitmap, cliprect);
	}


	return 0;
}


// renders to 2 bitmaps, and mixes output
UINT32 toaplan2_state::screen_update_batsugun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
//  bitmap.fill(0, cliprect);
//  gp9001_custom_priority_bitmap->fill(0, cliprect);

	if (m_vdp0)
	{
		bitmap.fill(0, cliprect);
		m_custom_priority_bitmap.fill(0, cliprect);
		m_vdp0->gp9001_render_vdp(bitmap, cliprect);
	}
	if (m_vdp1)
	{
		m_secondary_render_bitmap.fill(0, cliprect);
		m_custom_priority_bitmap.fill(0, cliprect);
		m_vdp1->gp9001_render_vdp(m_secondary_render_bitmap, cliprect);
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

	if (m_vdp0 && m_vdp1)
	{
		int width = screen.width();
		int height = screen.height();
		int y,x;
		UINT16* src_vdp0; // output buffer of vdp0
		UINT16* src_vdp1; // output buffer of vdp1

		for (y=0;y<height;y++)
		{
			src_vdp0 = &bitmap.pix16(y);
			src_vdp1 = &m_secondary_render_bitmap.pix16(y);

			for (x=0;x<width;x++)
			{
				UINT16 GPU0_LUTaddr = src_vdp0[x];
				UINT16 GPU1_LUTaddr = src_vdp1[x];

				// these equations is derived from the PAL, but doesn't seem to work?

				int COMPARISON = ((GPU0_LUTaddr & 0x0780) > (GPU1_LUTaddr & 0x0780));

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


UINT32 toaplan2_state::screen_update_toaplan2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	m_custom_priority_bitmap.fill(0, cliprect);
	m_vdp0->gp9001_render_vdp(bitmap, cliprect);

	return 0;
}


/* fixeightbl and bgareggabl do not use the lineselect or linescroll tables */
UINT32 toaplan2_state::screen_update_bootleg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen_update_toaplan2(screen, bitmap, cliprect);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0);
	return 0;
}


UINT32 toaplan2_state::screen_update_truxton2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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



void toaplan2_state::screen_eof_toaplan2(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		if (m_vdp0) m_vdp0->gp9001_screen_eof();
		if (m_vdp1) m_vdp1->gp9001_screen_eof();
	}
}
