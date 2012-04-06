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

#define RAIZING_TX_GFXRAM_SIZE  0x8000	/* GFX data decode RAM size */



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_text_tile_info )
{
	toaplan2_state *state = machine.driver_data<toaplan2_state>();
	int color, tile_number, attrib;

	attrib = state->m_txvideoram16[tile_index];
	tile_number = attrib & 0x3ff;
	color = ((attrib >> 10) | 0x40) & 0x7f;
	SET_TILE_INFO(
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

	state->m_tx_tilemap = tilemap_create(machine, get_text_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
	state->m_tx_tilemap->set_scroll_rows(8*32);	/* line scrolling */
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
		gfx_element_mark_dirty(machine.gfx[2], i);
}

VIDEO_START( toaplan2 )
{
	toaplan2_state *state = machine.driver_data<toaplan2_state>();

	/* cache the VDP device */
	state->m_vdp0 = machine.device<gp9001vdp_device>("gp9001vdp0");
	state->m_vdp1 = machine.device<gp9001vdp_device>("gp9001vdp1");

	/* our current VDP implementation needs this bitmap to work with */
	machine.primary_screen->register_screen_bitmap(state->m_custom_priority_bitmap);

	if (state->m_vdp0 != NULL)
	{
		state->m_secondary_render_bitmap.reset();
		state->m_vdp0->custom_priority_bitmap = &state->m_custom_priority_bitmap;
	}

	if (state->m_vdp1 != NULL)
	{
		machine.primary_screen->register_screen_bitmap(state->m_secondary_render_bitmap);
		state->m_vdp1->custom_priority_bitmap = &state->m_custom_priority_bitmap;
	}

	register_state_save(machine);
}

VIDEO_START( truxton2 )
{
	toaplan2_state *state = machine.driver_data<toaplan2_state>();

	VIDEO_START_CALL( toaplan2 );

	/* Create the Text tilemap for this game */
	gfx_element_set_source(machine.gfx[2], (UINT8 *)state->m_tx_gfxram16);
	machine.save().register_postload(save_prepost_delegate(FUNC(truxton2_postload), &machine));

	truxton2_create_tx_tilemap(machine);
	state->m_tx_tilemap->set_scrolldx(0x1d4 +1, 0x2a);
}

VIDEO_START( fixeightbl )
{
	toaplan2_state *state = machine.driver_data<toaplan2_state>();

	VIDEO_START_CALL( toaplan2 );

	/* Create the Text tilemap for this game */
	truxton2_create_tx_tilemap(machine);

	/* This bootleg has additional layer offsets on the VDP */
	state->m_vdp0->bg.extra_xoffset.normal  = -0x1d6  -26;
	state->m_vdp0->bg.extra_yoffset.normal  = -0x1ef  -15;

	state->m_vdp0->fg.extra_xoffset.normal  = -0x1d8  -22;
	state->m_vdp0->fg.extra_yoffset.normal  = -0x1ef  -15;

	state->m_vdp0->top.extra_xoffset.normal = -0x1da  -18;
	state->m_vdp0->top.extra_yoffset.normal = -0x1ef  -15;

	state->m_vdp0->sp.extra_xoffset.normal  = 8;//-0x1cc  -64;
	state->m_vdp0->sp.extra_yoffset.normal  = 8;//-0x1ef  -128;

	state->m_vdp0->init_scroll_regs();

	state->m_tx_tilemap->set_scrolldx(0, 0);
}

VIDEO_START( bgaregga )
{
	toaplan2_state *state = machine.driver_data<toaplan2_state>();

	VIDEO_START_CALL( toaplan2 );

	/* Create the Text tilemap for this game */
	truxton2_create_tx_tilemap(machine);
	state->m_tx_tilemap->set_scrolldx(0x1d4, 0x2a);
}

VIDEO_START( bgareggabl )
{
	toaplan2_state *state = machine.driver_data<toaplan2_state>();

	VIDEO_START_CALL( toaplan2 );

	/* Create the Text tilemap for this game */
	truxton2_create_tx_tilemap(machine);
	state->m_tx_tilemap->set_scrolldx(0x04, 0x2a);
}

VIDEO_START( batrider )
{
	toaplan2_state *state = machine.driver_data<toaplan2_state>();
	VIDEO_START_CALL( toaplan2 );

	state->m_vdp0->sp.use_sprite_buffer = 0; // disable buffering on this game

	/* Create the Text tilemap for this game */
	state->m_tx_gfxram16 = auto_alloc_array_clear(machine, UINT16, RAIZING_TX_GFXRAM_SIZE/2);
	state->save_pointer(NAME(state->m_tx_gfxram16), RAIZING_TX_GFXRAM_SIZE/2);
	gfx_element_set_source(machine.gfx[2], (UINT8 *)state->m_tx_gfxram16);
	machine.save().register_postload(save_prepost_delegate(FUNC(truxton2_postload), &machine));

	truxton2_create_tx_tilemap(machine);
	state->m_tx_tilemap->set_scrolldx(0x1d4, 0x2a);

	/* Has special banking */
	state->m_vdp0->gp9001_gfxrom_is_banked = 1;
}

WRITE16_MEMBER(toaplan2_state::toaplan2_txvideoram16_w)
{

	COMBINE_DATA(&m_txvideoram16[offset]);
	if (offset < (m_tx_vram_size/4))
		m_tx_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(toaplan2_state::toaplan2_txvideoram16_offs_w)
{
	// FIXME: implement line select and per-line flipping for all games
	// see SCREEN_UPDATE_IND16( batrider )

	UINT16 oldword = m_txvideoram16_offs[offset];

	if (oldword != data)
	{
		if (offset == 0)			/* Wrong ! */
		{
			if (data & 0x8000)		/* Flip off */
			{
				m_tx_flip = 0;
				m_tx_tilemap->set_flip(m_tx_flip);
				m_tx_tilemap->set_scrolly(0, 0);
			}
			else					/* Flip on */
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
		gfx_element_mark_dirty(machine().gfx[2], code);
	}
}

WRITE16_MEMBER(toaplan2_state::batrider_textdata_dma_w)
{
	/*** Dynamic Text GFX decoding for Batrider ***/
	/*** Only done once during start-up ***/

	UINT16 *dest = m_tx_gfxram16;

	memcpy(dest, m_txvideoram16, m_tx_vram_size);
	dest += (m_tx_vram_size/2);
	memcpy(dest, m_generic_paletteram_16, m_paletteram_size);
	dest += (m_paletteram_size/2);
	memcpy(dest, m_txvideoram16_offs, m_tx_offs_vram_size);
	dest += (m_tx_offs_vram_size/2);
	memcpy(dest, m_txscrollram16, m_tx_scroll_vram_size);
	dest += (m_tx_scroll_vram_size/2);
	memcpy(dest, m_mainram16, m_mainram_overlap_size);

	for (int i = 0; i < 1024; i++)
		gfx_element_mark_dirty(machine().gfx[2], i);
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
SCREEN_UPDATE_IND16( toaplan2_dual )
{
	toaplan2_state *state = screen.machine().driver_data<toaplan2_state>();

	if (state->m_vdp1)
	{
		bitmap.fill(0, cliprect);
		state->m_custom_priority_bitmap.fill(0, cliprect);
		state->m_vdp1->gp9001_render_vdp(screen.machine(), bitmap, cliprect);
	}
	if (state->m_vdp0)
	{
	//  bitmap.fill(0, cliprect);
		state->m_custom_priority_bitmap.fill(0, cliprect);
		state->m_vdp0->gp9001_render_vdp(screen.machine(), bitmap, cliprect);
	}


	return 0;
}


// renders to 2 bitmaps, and mixes output
SCREEN_UPDATE_IND16( toaplan2_mixed )
{
	toaplan2_state *state = screen.machine().driver_data<toaplan2_state>();

//  bitmap.fill(0, cliprect);
//  gp9001_custom_priority_bitmap->fill(0, cliprect);

	if (state->m_vdp0)
	{
		bitmap.fill(0, cliprect);
		state->m_custom_priority_bitmap.fill(0, cliprect);
		state->m_vdp0->gp9001_render_vdp(screen.machine(), bitmap, cliprect);
	}
	if (state->m_vdp1)
	{
		state->m_secondary_render_bitmap.fill(0, cliprect);
		state->m_custom_priority_bitmap.fill(0, cliprect);
		state->m_vdp1->gp9001_render_vdp(screen.machine(), state->m_secondary_render_bitmap, cliprect);
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

	if (state->m_vdp0 && state->m_vdp1)
	{
		int width = screen.width();
		int height = screen.height();
		int y,x;
		UINT16* src_vdp0; // output buffer of vdp0
		UINT16* src_vdp1; // output buffer of vdp1

		for (y=0;y<height;y++)
		{
			src_vdp0 = &bitmap.pix16(y);
			src_vdp1 = &state->m_secondary_render_bitmap.pix16(y);

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

SCREEN_UPDATE_IND16( toaplan2 )
{
	toaplan2_state *state = screen.machine().driver_data<toaplan2_state>();

	if (state->m_vdp0)
	{
		bitmap.fill(0, cliprect);
		state->m_custom_priority_bitmap.fill(0, cliprect);
		state->m_vdp0->gp9001_render_vdp(screen.machine(), bitmap, cliprect);
	}

	return 0;
}

SCREEN_UPDATE_IND16( truxton2 )
{
	toaplan2_state *state = screen.machine().driver_data<toaplan2_state>();

	SCREEN_UPDATE16_CALL(toaplan2);
	state->m_tx_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}


SCREEN_UPDATE_IND16( batrider )
{
	toaplan2_state *state = screen.machine().driver_data<toaplan2_state>();

	SCREEN_UPDATE16_CALL( toaplan2 );

	int line;
	rectangle clip;
	const rectangle &visarea = screen.visible_area();

	clip = visarea;

	/* used for 'for use in' and '8ing' screen on bbakraid, raizing on batrider */
	for (line = 0; line < 256;line++)
	{
		if (state->m_tx_flip)
		{
			clip.min_y = clip.max_y = 256 - line;
			state->m_tx_tilemap->set_scrolly(0, 256 - line + state->m_txvideoram16_offs[256 - line]);
		}
		else
		{
			clip.min_y = clip.max_y = line;
			state->m_tx_tilemap->set_scrolly(0,     - line + state->m_txvideoram16_offs[      line]);
		}
		state->m_tx_tilemap->draw(bitmap, clip, 0, 0);
	}
	return 0;
}



SCREEN_UPDATE_IND16( dogyuun )
{
	SCREEN_UPDATE16_CALL( toaplan2_dual );
	return 0;
}

SCREEN_UPDATE_IND16( batsugun )
{
	SCREEN_UPDATE16_CALL( toaplan2_mixed );
	return 0;
}

SCREEN_VBLANK( toaplan2 )
{
	// rising edge
	if (vblank_on)
	{
		toaplan2_state *state = screen.machine().driver_data<toaplan2_state>();
		if (state->m_vdp0) state->m_vdp0->gp9001_screen_eof();
		if (state->m_vdp1) state->m_vdp1->gp9001_screen_eof();
	}
}
