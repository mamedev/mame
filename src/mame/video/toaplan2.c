/***************************************************************************

 Functions to emulate additional video hardware on several Toaplan2 games.
 The main video is handled by the GP9001 (see video gp9001.c)

 Extra-text RAM format

 Truxton 2, Fixeight and Raizing games have an extra-text layer.

  Text RAM format      $0000-1FFF (actually its probably $0000-0FFF)
  ---- --xx xxxx xxxx = Tile number
  xxxx xx-- ---- ---- = Color (0 - 3Fh) + 40h

  Text flip / ???      $0000-01EF (some games go to $01FF (excess?))
  ---x xxxx xxxx xxxx = ??? line something (line to draw ?) ???
  x--- ---- ---- ---- = flip for the Text tile

  Text X line-scroll ? $0000-01EF (some games go to $01FF (excess?))
  ---- ---x xxxx xxxx = X-Scroll for each line


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

	attrib = m_txvideoram16[tile_index];
	tile_number = attrib & 0x3ff;
	color = ((attrib >> 10) | 0x40) & 0x7f;
	SET_TILE_INFO_MEMBER(
			2,
			tile_number,
			color,
			0);
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/


static void truxton2_create_tx_tilemap(running_machine &machine)
{
	toaplan2_state *state = machine.driver_data<toaplan2_state>();

	state->m_tx_tilemap = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(toaplan2_state::get_text_tile_info),state), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	state->m_tx_tilemap->set_scroll_rows(8*32); /* line scrolling */
	state->m_tx_tilemap->set_scroll_cols(1);
	state->m_tx_tilemap->set_transparent_pen(0);
}

static void register_state_save(running_machine &machine)
{
	toaplan2_state *state = machine.driver_data<toaplan2_state>();

	state->save_item(NAME(state->m_tx_flip));
}

static void truxton2_postload(running_machine &machine)
{
	for (int i = 0; i < 1024; i++)
		machine.gfx[2]->mark_dirty(i);
}

VIDEO_START_MEMBER(toaplan2_state,toaplan2)
{

	/* cache the VDP device */
	m_vdp0 = machine().device<gp9001vdp_device>("gp9001vdp0");
	m_vdp1 = machine().device<gp9001vdp_device>("gp9001vdp1");

	/* our current VDP implementation needs this bitmap to work with */
	machine().primary_screen->register_screen_bitmap(m_custom_priority_bitmap);

	if (m_vdp0 != NULL)
	{
		m_secondary_render_bitmap.reset();
		m_vdp0->custom_priority_bitmap = &m_custom_priority_bitmap;
	}

	if (m_vdp1 != NULL)
	{
		machine().primary_screen->register_screen_bitmap(m_secondary_render_bitmap);
		m_vdp1->custom_priority_bitmap = &m_custom_priority_bitmap;
	}

	register_state_save(machine());
}

VIDEO_START_MEMBER(toaplan2_state,truxton2)
{

	VIDEO_START_CALL_MEMBER( toaplan2 );

	/* Create the Text tilemap for this game */
	machine().gfx[2]->set_source(reinterpret_cast<UINT8 *>(m_tx_gfxram16.target()));
	machine().save().register_postload(save_prepost_delegate(FUNC(truxton2_postload), &machine()));

	truxton2_create_tx_tilemap(machine());
	m_tx_tilemap->set_scrolldx(0x1d4 +1, 0x2a);
}

VIDEO_START_MEMBER(toaplan2_state,fixeightbl)
{

	VIDEO_START_CALL_MEMBER( toaplan2 );

	/* Create the Text tilemap for this game */
	truxton2_create_tx_tilemap(machine());

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

	m_tx_tilemap->set_scrolldx(0, 0);
}

VIDEO_START_MEMBER(toaplan2_state,bgaregga)
{

	VIDEO_START_CALL_MEMBER( toaplan2 );

	/* Create the Text tilemap for this game */
	truxton2_create_tx_tilemap(machine());
	m_tx_tilemap->set_scrolldx(0x1d4, 0x2a);
}

VIDEO_START_MEMBER(toaplan2_state,bgareggabl)
{

	VIDEO_START_CALL_MEMBER( toaplan2 );

	/* Create the Text tilemap for this game */
	truxton2_create_tx_tilemap(machine());
	m_tx_tilemap->set_scrolldx(0x04, 0x2a);
}

VIDEO_START_MEMBER(toaplan2_state,batrider)
{
	VIDEO_START_CALL_MEMBER( toaplan2 );

	m_vdp0->sp.use_sprite_buffer = 0; // disable buffering on this game

	/* Create the Text tilemap for this game */
	m_tx_gfxram16.allocate(RAIZING_TX_GFXRAM_SIZE/2);
	machine().gfx[2]->set_source(reinterpret_cast<UINT8 *>(m_tx_gfxram16.target()));
	machine().save().register_postload(save_prepost_delegate(FUNC(truxton2_postload), &machine()));

	truxton2_create_tx_tilemap(machine());
	m_tx_tilemap->set_scrolldx(0x1d4, 0x2a);

	/* Has special banking */
	m_vdp0->gp9001_gfxrom_is_banked = 1;
}

WRITE16_MEMBER(toaplan2_state::toaplan2_txvideoram16_w)
{

	COMBINE_DATA(&m_txvideoram16[offset]);
	if (offset < m_txvideoram16.bytes()/4)
		m_tx_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(toaplan2_state::toaplan2_txvideoram16_offs_w)
{
	// FIXME: implement line select and per-line flipping for all games
	// see SCREEN_UPDATE_IND16( batrider )

	UINT16 oldword = m_txvideoram16_offs[offset];

	if (oldword != data)
	{
		if (offset == 0)            /* Wrong ! */
		{
			if (data & 0x8000)      /* Flip off */
			{
				m_tx_flip = 0;
				m_tx_tilemap->set_flip(m_tx_flip);
				m_tx_tilemap->set_scrolly(0, 0);
			}
			else                    /* Flip on */
			{
				m_tx_flip = (TILEMAP_FLIPY | TILEMAP_FLIPX);
				m_tx_tilemap->set_flip(m_tx_flip);
				m_tx_tilemap->set_scrolly(0, -16);
			}
		}
		COMBINE_DATA(&m_txvideoram16_offs[offset]);
	}
//  logerror("Writing %04x to text offs RAM offset %04x\n",data,offset);
}

WRITE16_MEMBER(toaplan2_state::toaplan2_txscrollram16_w)
{
	/*** Line-Scroll RAM for Text Layer ***/

	int data_tx = data;

	m_tx_tilemap->set_scrollx(offset, data_tx);

//  logerror("Writing %04x to text scroll RAM offset %04x\n",data,offset);
	COMBINE_DATA(&m_txscrollram16[offset]);
}

WRITE16_MEMBER(toaplan2_state::toaplan2_tx_gfxram16_w)
{
	/*** Dynamic GFX decoding for Truxton 2 / FixEight ***/

	UINT16 oldword = m_tx_gfxram16[offset];

	if (oldword != data)
	{
		int code = offset/32;
		COMBINE_DATA(&m_tx_gfxram16[offset]);
		machine().gfx[2]->mark_dirty(code);
	}
}

WRITE16_MEMBER(toaplan2_state::batrider_textdata_dma_w)
{
	/*** Dynamic Text GFX decoding for Batrider ***/
	/*** Only done once during start-up ***/

	UINT16 *dest = m_tx_gfxram16;

	memcpy(dest, m_txvideoram16, m_txvideoram16.bytes());
	dest += (m_txvideoram16.bytes()/2);
	memcpy(dest, m_generic_paletteram_16, m_generic_paletteram_16.bytes());
	dest += (m_generic_paletteram_16.bytes()/2);
	memcpy(dest, m_txvideoram16_offs, m_txvideoram16_offs.bytes());
	dest += (m_txvideoram16_offs.bytes()/2);
	memcpy(dest, m_txscrollram16, m_txscrollram16.bytes());
	dest += (m_txscrollram16.bytes()/2);
	memcpy(dest, m_mainram16, m_mainram16.bytes());

	for (int i = 0; i < 1024; i++)
		machine().gfx[2]->mark_dirty(i);
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
UINT32 toaplan2_state::screen_update_toaplan2_dual(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{

	if (m_vdp1)
	{
		bitmap.fill(0, cliprect);
		m_custom_priority_bitmap.fill(0, cliprect);
		m_vdp1->gp9001_render_vdp(machine(), bitmap, cliprect);
	}
	if (m_vdp0)
	{
	//  bitmap.fill(0, cliprect);
		m_custom_priority_bitmap.fill(0, cliprect);
		m_vdp0->gp9001_render_vdp(machine(), bitmap, cliprect);
	}


	return 0;
}


// renders to 2 bitmaps, and mixes output
UINT32 toaplan2_state::screen_update_toaplan2_mixed(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{

//  bitmap.fill(0, cliprect);
//  gp9001_custom_priority_bitmap->fill(0, cliprect);

	if (m_vdp0)
	{
		bitmap.fill(0, cliprect);
		m_custom_priority_bitmap.fill(0, cliprect);
		m_vdp0->gp9001_render_vdp(machine(), bitmap, cliprect);
	}
	if (m_vdp1)
	{
		m_secondary_render_bitmap.fill(0, cliprect);
		m_custom_priority_bitmap.fill(0, cliprect);
		m_vdp1->gp9001_render_vdp(machine(), m_secondary_render_bitmap, cliprect);
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

	if (m_vdp0)
	{
		bitmap.fill(0, cliprect);
		m_custom_priority_bitmap.fill(0, cliprect);
		m_vdp0->gp9001_render_vdp(machine(), bitmap, cliprect);
	}

	return 0;
}

UINT32 toaplan2_state::screen_update_truxton2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen_update_toaplan2(screen, bitmap, cliprect);
	m_tx_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}


UINT32 toaplan2_state::screen_update_batrider(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen_update_toaplan2(screen, bitmap, cliprect);

	int line;
	rectangle clip;
	const rectangle &visarea = screen.visible_area();

	clip = visarea;

	/* used for 'for use in' and '8ing' screen on bbakraid, raizing on batrider */
	for (line = 0; line < 256;line++)
	{
		if (m_tx_flip)
		{
			clip.min_y = clip.max_y = 256 - line;
			m_tx_tilemap->set_scrolly(0, 256 - line + m_txvideoram16_offs[256 - line]);
		}
		else
		{
			clip.min_y = clip.max_y = line;
			m_tx_tilemap->set_scrolly(0,     - line + m_txvideoram16_offs[      line]);
		}
		m_tx_tilemap->draw(bitmap, clip, 0, 0);
	}
	return 0;
}



UINT32 toaplan2_state::screen_update_dogyuun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen_update_toaplan2_dual(screen, bitmap, cliprect);
	return 0;
}

UINT32 toaplan2_state::screen_update_batsugun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen_update_toaplan2_mixed(screen, bitmap, cliprect);
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
