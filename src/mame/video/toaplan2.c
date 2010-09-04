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
#include "cpu/m68000/m68000.h"
#include "sound/3812intf.h"
#include "includes/toaplan2.h"

#define RAIZING_TX_GFXRAM_SIZE  0x8000	/* GFX data decode RAM size */



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_text_tile_info )
{
	toaplan2_state *state = machine->driver_data<toaplan2_state>();
	int color, tile_number, attrib;

	attrib = state->txvideoram16[tile_index];
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


static void truxton2_create_tx_tilemap(running_machine *machine)
{
	toaplan2_state *state = machine->driver_data<toaplan2_state>();

	state->tx_tilemap = tilemap_create(machine, get_text_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
	tilemap_set_scroll_rows(state->tx_tilemap, 8*32);	/* line scrolling */
	tilemap_set_scroll_cols(state->tx_tilemap, 1);
	tilemap_set_transparent_pen(state->tx_tilemap, 0);
}

static void register_state_save(running_machine *machine)
{
	toaplan2_state *state = machine->driver_data<toaplan2_state>();

	state_save_register_global(machine, state->tx_flip);
}


VIDEO_START( toaplan2 )
{
	toaplan2_state *state = machine->driver_data<toaplan2_state>();
	int width = machine->primary_screen->width();
	int height = machine->primary_screen->height();

	/* cache the VDP device */
	state->vdp0 = machine->device<gp9001vdp_device>("gp9001vdp0");
	state->vdp1 = machine->device<gp9001vdp_device>("gp9001vdp1");

	/* our current VDP implementation needs this bitmap to work with */
	state->custom_priority_bitmap = auto_bitmap_alloc(machine, width, height, BITMAP_FORMAT_INDEXED8);
	state->displog = 0; // debug flag

	if (state->vdp0 != NULL)
	{
		state->secondary_render_bitmap = NULL;
		state->vdp0->custom_priority_bitmap = state->custom_priority_bitmap;
		state->vdp0->displog = &state->displog;
	}

	if (state->vdp1 != NULL)
	{
		state->secondary_render_bitmap = auto_bitmap_alloc(machine, width, height, BITMAP_FORMAT_INDEXED16);
		state->vdp1->custom_priority_bitmap = state->custom_priority_bitmap;
		state->vdp1->displog = &state->displog;
	}

	state->display_tx = 1;

	register_state_save(machine);
}

VIDEO_START( truxton2 )
{
	toaplan2_state *state = machine->driver_data<toaplan2_state>();

	VIDEO_START_CALL( toaplan2 );

	/* Create the Text tilemap for this game */
	truxton2_create_tx_tilemap(machine);
	if (machine->gfx[2]->srcdata == NULL)
		gfx_element_set_source(machine->gfx[2], (UINT8 *)state->tx_gfxram16);
	tilemap_set_scrolldx(state->tx_tilemap, 0x1d4 +1, 0x2a);
}

VIDEO_START( fixeighb )
{
	toaplan2_state *state = machine->driver_data<toaplan2_state>();

	VIDEO_START_CALL( truxton2 );

	/* This bootleg has additional layer offsets on the VDP */
	state->vdp0->extra_xoffset[0]=-26;
	state->vdp0->extra_xoffset[1]=-22;
	state->vdp0->extra_xoffset[2]=-18;
	state->vdp0->extra_xoffset[3]=8;

	state->vdp0->extra_yoffset[0]=-15;
	state->vdp0->extra_yoffset[1]=-15;
	state->vdp0->extra_yoffset[2]=-15;
	state->vdp0->extra_yoffset[3]=8;

	tilemap_set_scrolldx(state->tx_tilemap, 0, 0);
}

VIDEO_START( bgaregga )
{
	toaplan2_state *state = machine->driver_data<toaplan2_state>();

	VIDEO_START_CALL( toaplan2 );

	/* Create the Text tilemap for this game */
	truxton2_create_tx_tilemap(machine);
	tilemap_set_scrolldx(state->tx_tilemap, 0x1d4, 0x2a);
}

VIDEO_START( batrider )
{
	toaplan2_state *state = machine->driver_data<toaplan2_state>();
	VIDEO_START_CALL( toaplan2 );

	state->vdp0->spriteram16_n = state->vdp0->spriteram16_new;

	/* Create the Text tilemap for this game */
	state->tx_gfxram16 = auto_alloc_array_clear(machine, UINT16, RAIZING_TX_GFXRAM_SIZE/2);
	state_save_register_global_pointer(machine, state->tx_gfxram16, RAIZING_TX_GFXRAM_SIZE/2);
	gfx_element_set_source(machine->gfx[2], (UINT8 *)state->tx_gfxram16);
	truxton2_create_tx_tilemap(machine);
	tilemap_set_scrolldx(state->tx_tilemap, 0x1d4, 0x2a);

	/* Has special banking */
	state->vdp0->gp9001_gfxrom_is_banked = 1;
}

READ16_HANDLER( toaplan2_txvideoram16_r )
{
	toaplan2_state *state = space->machine->driver_data<toaplan2_state>();

	return state->txvideoram16[offset];
}

WRITE16_HANDLER( toaplan2_txvideoram16_w )
{
	toaplan2_state *state = space->machine->driver_data<toaplan2_state>();

	COMBINE_DATA(&state->txvideoram16[offset]);
	if (offset < (state->tx_vram_size/4))
		tilemap_mark_tile_dirty(state->tx_tilemap, offset);
}

READ16_HANDLER( toaplan2_txvideoram16_offs_r )
{
	toaplan2_state *state = space->machine->driver_data<toaplan2_state>();

	return state->txvideoram16_offs[offset];
}

WRITE16_HANDLER( toaplan2_txvideoram16_offs_w )
{
	/* Besides containing flip, function of this RAM is still unknown */
	/* This is however line related as per line-scroll RAM below */
	/* Maybe specifies which line to draw text info (line number data is */
	/*   opposite when flip bits are on) */

	toaplan2_state *state = space->machine->driver_data<toaplan2_state>();
	UINT16 oldword = state->txvideoram16_offs[offset];

	if (oldword != data)
	{
		if (offset == 0)			/* Wrong ! */
		{
			if (data & 0x8000)		/* Flip off */
			{
				state->tx_flip = 0;
				tilemap_set_flip(state->tx_tilemap, state->tx_flip);
				tilemap_set_scrolly(state->tx_tilemap, 0, 0);
			}
			else					/* Flip on */
			{
				state->tx_flip = (TILEMAP_FLIPY | TILEMAP_FLIPX);
				tilemap_set_flip(state->tx_tilemap, state->tx_flip);
				tilemap_set_scrolly(state->tx_tilemap, 0, -16);
			}
		}
		COMBINE_DATA(&state->txvideoram16_offs[offset]);
	}
//  logerror("Writing %04x to text offs RAM offset %04x\n",data,offset);
}

READ16_HANDLER( toaplan2_txscrollram16_r )
{
	toaplan2_state *state = space->machine->driver_data<toaplan2_state>();

	return state->txscrollram16[offset];
}

WRITE16_HANDLER( toaplan2_txscrollram16_w )
{
	/*** Line-Scroll RAM for Text Layer ***/

	toaplan2_state *state = space->machine->driver_data<toaplan2_state>();
	int data_tx = data;

	tilemap_set_scrollx(state->tx_tilemap, offset, data_tx);

//  logerror("Writing %04x to text scroll RAM offset %04x\n",data,offset);
	COMBINE_DATA(&state->txscrollram16[offset]);
}

READ16_HANDLER( toaplan2_tx_gfxram16_r )
{
	toaplan2_state *state = space->machine->driver_data<toaplan2_state>();

	return state->tx_gfxram16[offset];
}

WRITE16_HANDLER( toaplan2_tx_gfxram16_w )
{
	/*** Dynamic GFX decoding for Truxton 2 / FixEight ***/

	toaplan2_state *state = space->machine->driver_data<toaplan2_state>();
	UINT16 oldword = state->tx_gfxram16[offset];

	if (oldword != data)
	{
		int code = offset/32;
		COMBINE_DATA(&state->tx_gfxram16[offset]);
		gfx_element_mark_dirty(space->machine->gfx[2], code);
	}
}

READ16_HANDLER( raizing_tx_gfxram16_r )
{
	toaplan2_state *state = space->machine->driver_data<toaplan2_state>();

	offset += 0x3400/2;
	return state->tx_gfxram16[offset];
}

WRITE16_HANDLER( raizing_tx_gfxram16_w )
{
	/*** Dynamic Text GFX decoding for Batrider ***/

	toaplan2_state *state = space->machine->driver_data<toaplan2_state>();
	UINT16 oldword;

	offset += 0x3400/2;
	oldword = state->tx_gfxram16[offset];
	if (oldword != data)
	{
		COMBINE_DATA(&state->tx_gfxram16[offset]);
	}
}

WRITE16_HANDLER( batrider_textdata_decode )
{
	/*** Dynamic Text GFX decoding for Batrider ***/
	/*** Only done once during start-up ***/

	toaplan2_state *state = space->machine->driver_data<toaplan2_state>();
	int code;
	UINT16 *dest = (UINT16 *)state->tx_gfxram16;

	memcpy(dest, state->txvideoram16, state->tx_vram_size);
	dest += (state->tx_vram_size/2);
	memcpy(dest, space->machine->generic.paletteram.u16, state->paletteram16_size);
	dest += (state->paletteram16_size/2);
	memcpy(dest, state->txvideoram16_offs, state->tx_offs_vram_size);
	dest += (state->tx_offs_vram_size/2);
	memcpy(dest, state->txscrollram16, state->tx_scroll_vram_size);

	/* Decode text characters; force them to update immediately */
	for (code = 0; code < 1024; code++)
		gfx_element_decode(space->machine->gfx[2], code);
}

WRITE16_HANDLER( batrider_objectbank_w )
{
	toaplan2_state *state = space->machine->driver_data<toaplan2_state>();

	if (ACCESSING_BITS_0_7)
	{
		data &= 0xf;
		if (state->vdp0->gp9001_gfxrom_bank[offset] != data)
		{
			state->vdp0->gp9001_gfxrom_bank[offset] = data;
			state->vdp0->gp9001_gfxrom_bank_dirty = 1;
		}
	}
}

// Dogyuun doesn't appear to require fancy mixing?
VIDEO_UPDATE( toaplan2_dual )
{
	toaplan2_state *state = screen->machine->driver_data<toaplan2_state>();

	if (state->vdp1)
	{
		gp9001_log_vram(state->vdp1, screen->machine);

		bitmap_fill(bitmap,cliprect,0);
		bitmap_fill(state->custom_priority_bitmap, cliprect, 0);
		state->vdp1->gp9001_render_vdp(screen->machine, bitmap, cliprect);
	}
	if (state->vdp0)
	{
		gp9001_log_vram(state->vdp0, screen->machine);

	//  bitmap_fill(bitmap,cliprect,0);
		bitmap_fill(state->custom_priority_bitmap, cliprect, 0);
		state->vdp0->gp9001_render_vdp(screen->machine, bitmap, cliprect);
	}


	return 0;
}


// renders to 2 bitmaps, and mixes output
VIDEO_UPDATE( toaplan2_mixed )
{
	toaplan2_state *state = screen->machine->driver_data<toaplan2_state>();

//  bitmap_fill(bitmap,cliprect,0);
//  bitmap_fill(gp9001_custom_priority_bitmap, cliprect, 0);

	if (state->vdp0)
	{
		gp9001_log_vram(state->vdp0, screen->machine);

		bitmap_fill(bitmap,cliprect,0);
		bitmap_fill(state->custom_priority_bitmap, cliprect, 0);
		state->vdp0->gp9001_render_vdp(screen->machine, bitmap, cliprect);
	}
	if (state->vdp1)
	{
		gp9001_log_vram(state->vdp1, screen->machine);

		bitmap_fill(state->secondary_render_bitmap,cliprect,0);
		bitmap_fill(state->custom_priority_bitmap, cliprect, 0);
		state->vdp1->gp9001_render_vdp(screen->machine, state->secondary_render_bitmap, cliprect);
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

	if (state->vdp0 && state->vdp1)
	{
		int width = screen->width();
		int height = screen->height();
		int y,x;
		UINT16* src_vdp0; // output buffer of vdp0
		UINT16* src_vdp1; // output buffer of vdp1

		for (y=0;y<height;y++)
		{
			src_vdp0 = BITMAP_ADDR16(bitmap, y, 0);
			src_vdp1 = BITMAP_ADDR16(state->secondary_render_bitmap, y, 0);

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

VIDEO_UPDATE( toaplan2 )
{
	toaplan2_state *state = screen->machine->driver_data<toaplan2_state>();

	if (state->vdp0)
	{
		running_device *screen1  = screen->machine->device("screen");

		gp9001_log_vram(state->vdp0, screen->machine);

		if (screen == screen1)
		{
			bitmap_fill(bitmap,cliprect,0);
			bitmap_fill(state->custom_priority_bitmap, cliprect, 0);
			state->vdp0->gp9001_render_vdp(screen->machine, bitmap, cliprect);
		}
	}

	/* debug code, render 2nd VDP to 2nd screen if they exist for test */
#ifdef DUAL_SCREEN_VDPS
	if (state->vdp1)
	{
		running_device *screen2 = screen->machine->device("screen2");

		gp9001_log_vram(state->vdp1, screen->machine);

		if (screen == screen2)
		{
			bitmap_fill(bitmap,cliprect,0);
			bitmap_fill(state->custom_priority_bitmap, cliprect, 0);
			state->vdp1->gp9001_render_vdp(screen->machine, bitmap, cliprect);
		}
	}
#endif

	return 0;
}

VIDEO_UPDATE( truxton2 )
{
	toaplan2_state *state = screen->machine->driver_data<toaplan2_state>();

	VIDEO_UPDATE_CALL(toaplan2);
	tilemap_draw(bitmap, cliprect, state->tx_tilemap, 0, 0);
	return 0;
}


VIDEO_UPDATE( batrider )
{
	toaplan2_state *state = screen->machine->driver_data<toaplan2_state>();

	VIDEO_UPDATE_CALL( toaplan2 );

	int line;
	rectangle clip;
	const rectangle &visarea = screen->visible_area();

	clip.min_x = visarea.min_x;
	clip.max_x = visarea.max_x;
	clip.min_y = visarea.min_y;
	clip.max_y = visarea.max_y;

	/* used for 'for use in' and '8ing' screen on bbakraid, raizing on batrider */
	for (line = 0; line < 256;line++)
	{
		if (state->tx_flip)
		{
			clip.min_y = clip.max_y = 256 - line;
			tilemap_set_scrolly(state->tx_tilemap, 0, 256 - line + state->txvideoram16_offs[256 - line]);
		}
		else
		{
			clip.min_y = clip.max_y = line;
			tilemap_set_scrolly(state->tx_tilemap, 0,     - line + state->txvideoram16_offs[      line]);
		}
		tilemap_draw(bitmap, &clip, state->tx_tilemap, 0, 0);
	}
	return 0;
}



VIDEO_UPDATE( dogyuun )
{
#ifdef DUAL_SCREEN_VDPS
	VIDEO_UPDATE_CALL( toaplan2 );
#else
	VIDEO_UPDATE_CALL( toaplan2_dual );
#endif

	return 0;
}

VIDEO_UPDATE( batsugun )
{
#ifdef DUAL_SCREEN_VDPS
	VIDEO_UPDATE_CALL( toaplan2 );
#else
	VIDEO_UPDATE_CALL( toaplan2_mixed );
#endif

	return 0;
}

VIDEO_EOF( toaplan2 )
{
	toaplan2_state *state = machine->driver_data<toaplan2_state>();
	if (state->vdp0) state->vdp0->gp9001_video_eof();
	if (state->vdp1) state->vdp1->gp9001_video_eof();
}
