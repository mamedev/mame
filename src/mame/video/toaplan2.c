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

#define CPU_2_NONE		0x00
#define CPU_2_Z80		0x5a
#define CPU_2_HD647180	0xa5
#define CPU_2_V25		0xff



UINT16 *toaplan2_txvideoram16;		/* Video ram for extra text layer */
UINT16 *toaplan2_txvideoram16_offs;	/* Text layer tile flip and positon ? */
UINT16 *toaplan2_txscrollram16;		/* Text layer scroll ? */
UINT16 *toaplan2_tx_gfxram16;			/* Text Layer RAM based tiles */
UINT16 *raizing_tx_gfxram16;			/* Text Layer RAM based tiles (Batrider) */

size_t toaplan2_tx_vram_size;		 /* 0x2000 Text layer RAM size */
size_t toaplan2_tx_offs_vram_size;	 /* 0x200 Text layer tile flip and positon ? */
size_t toaplan2_tx_scroll_vram_size; /* 0x200 Text layer scroll ? */
size_t batrider_paletteram16_size;



static int display_tx;
static UINT8 tx_flip = 0;

static tilemap_t *tx_tilemap;	/* Tilemap for extra-text-layer */

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_text_tile_info )
{
	int color, tile_number, attrib;

	attrib = toaplan2_txvideoram16[tile_index];
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
	tx_tilemap = tilemap_create(machine, get_text_tile_info,tilemap_scan_rows,8,8,64,32);
	tilemap_set_scroll_rows(tx_tilemap,8*32);	/* line scrolling */
	tilemap_set_scroll_cols(tx_tilemap,1);
	tilemap_set_transparent_pen(tx_tilemap,0);
}

static void register_state_save(running_machine *machine)
{
	state_save_register_global(machine, tx_flip);
}


VIDEO_START( toaplan2 )
{
	int width = machine->primary_screen->width();
	int height = machine->primary_screen->height();

	/* cache the VDP device */
	toaplan2_state *state = machine->driver_data<toaplan2_state>();
	state->vdp0 = machine->device<gp9001vdp_device>("gp9001vdp0");
	state->vdp1 = machine->device<gp9001vdp_device>("gp9001vdp1");

	/* our current VDP implementation needs this bitmap to work with */
	gp9001_custom_priority_bitmap = auto_bitmap_alloc(machine, width, height, BITMAP_FORMAT_INDEXED8);

	gp9001_displog = 0; // debug flag

	display_tx = 1;

	register_state_save(machine);
}

VIDEO_START( truxton2 )
{
	VIDEO_START_CALL( toaplan2 );

	/* Create the Text tilemap for this game */
	truxton2_create_tx_tilemap(machine);
	if (machine->gfx[2]->srcdata == NULL)
		gfx_element_set_source(machine->gfx[2], (UINT8 *)toaplan2_tx_gfxram16);
	tilemap_set_scrolldx(tx_tilemap, 0x1d4 +1, 0x2a);
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

	tilemap_set_scrolldx(tx_tilemap, 0, 0);
}

VIDEO_START( bgaregga )
{
	VIDEO_START_CALL( toaplan2 );

	/* Create the Text tilemap for this game */
	truxton2_create_tx_tilemap(machine);
	tilemap_set_scrolldx(tx_tilemap, 0x1d4, 0x2a);
}

VIDEO_START( batrider )
{
	toaplan2_state *state = machine->driver_data<toaplan2_state>();
	VIDEO_START_CALL( toaplan2 );

	state->vdp0->spriteram16_n = state->vdp0->spriteram16_new;

	/* Create the Text tilemap for this game */
	raizing_tx_gfxram16 = auto_alloc_array_clear(machine, UINT16, RAIZING_TX_GFXRAM_SIZE/2);
	state_save_register_global_pointer(machine, raizing_tx_gfxram16, RAIZING_TX_GFXRAM_SIZE/2);
	gfx_element_set_source(machine->gfx[2], (UINT8 *)raizing_tx_gfxram16);
	truxton2_create_tx_tilemap(machine);
	tilemap_set_scrolldx(tx_tilemap, 0x1d4, 0x2a);

	/* Has special banking */
	state->vdp0->gp9001_gfxrom_is_banked = 1;
}

READ16_HANDLER( toaplan2_txvideoram16_r )
{
	return toaplan2_txvideoram16[offset];
}

WRITE16_HANDLER( toaplan2_txvideoram16_w )
{
	COMBINE_DATA(&toaplan2_txvideoram16[offset]);
	if (offset < (toaplan2_tx_vram_size/4))
		tilemap_mark_tile_dirty(tx_tilemap,offset);
}

READ16_HANDLER( toaplan2_txvideoram16_offs_r )
{
	return toaplan2_txvideoram16_offs[offset];
}
WRITE16_HANDLER( toaplan2_txvideoram16_offs_w )
{
	/* Besides containing flip, function of this RAM is still unknown */
	/* This is however line related as per line-scroll RAM below */
	/* Maybe specifies which line to draw text info (line number data is */
	/*   opposite when flip bits are on) */

	UINT16 oldword = toaplan2_txvideoram16_offs[offset];

	if (oldword != data)
	{
		if (offset == 0)			/* Wrong ! */
		{
			if (data & 0x8000)		/* Flip off */
			{
				tx_flip = 0;
				tilemap_set_flip(tx_tilemap, tx_flip);
				tilemap_set_scrolly(tx_tilemap, 0, 0);
			}
			else					/* Flip on */
			{
				tx_flip = (TILEMAP_FLIPY | TILEMAP_FLIPX);
				tilemap_set_flip(tx_tilemap, tx_flip);
				tilemap_set_scrolly(tx_tilemap, 0, -16);
			}
		}
		COMBINE_DATA(&toaplan2_txvideoram16_offs[offset]);
	}
//  logerror("Writing %04x to text offs RAM offset %04x\n",data,offset);
}

READ16_HANDLER( toaplan2_txscrollram16_r )
{
	return toaplan2_txscrollram16[offset];
}
WRITE16_HANDLER( toaplan2_txscrollram16_w )
{
	/*** Line-Scroll RAM for Text Layer ***/

	int data_tx = data;

	tilemap_set_scrollx(tx_tilemap, offset, data_tx);

//  logerror("Writing %04x to text scroll RAM offset %04x\n",data,offset);
	COMBINE_DATA(&toaplan2_txscrollram16[offset]);
}

READ16_HANDLER( toaplan2_tx_gfxram16_r )
{
	return toaplan2_tx_gfxram16[offset];
}

WRITE16_HANDLER( toaplan2_tx_gfxram16_w )
{
	/*** Dynamic GFX decoding for Truxton 2 / FixEight ***/

	UINT16 oldword = toaplan2_tx_gfxram16[offset];

	if (oldword != data)
	{
		int code = offset/32;
		COMBINE_DATA(&toaplan2_tx_gfxram16[offset]);
		gfx_element_mark_dirty(space->machine->gfx[2], code);
	}
}

READ16_HANDLER( raizing_tx_gfxram16_r )
{
	offset += 0x3400/2;
	return raizing_tx_gfxram16[offset];
}
WRITE16_HANDLER( raizing_tx_gfxram16_w )
{
	/*** Dynamic Text GFX decoding for Batrider ***/

	UINT16 oldword = raizing_tx_gfxram16[offset + (0x3400 / 2)];

	if (oldword != data)
	{
		offset += 0x3400/2;
		COMBINE_DATA(&raizing_tx_gfxram16[offset]);
	}
}

WRITE16_HANDLER( batrider_textdata_decode )
{
	/*** Dynamic Text GFX decoding for Batrider ***/
	/*** Only done once during start-up ***/

	int code;
	UINT16 *dest = (UINT16 *)raizing_tx_gfxram16;

	memcpy(dest, toaplan2_txvideoram16, toaplan2_tx_vram_size);
	dest += (toaplan2_tx_vram_size/2);
	memcpy(dest, space->machine->generic.paletteram.u16, batrider_paletteram16_size);
	dest += (batrider_paletteram16_size/2);
	memcpy(dest, toaplan2_txvideoram16_offs, toaplan2_tx_offs_vram_size);
	dest += (toaplan2_tx_offs_vram_size/2);
	memcpy(dest, toaplan2_txscrollram16, toaplan2_tx_scroll_vram_size);

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
			bitmap_fill(gp9001_custom_priority_bitmap, cliprect, 0);
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
			bitmap_fill(gp9001_custom_priority_bitmap, cliprect, 0);
			state->vdp1->gp9001_render_vdp(screen->machine, bitmap, cliprect);
		}
	}
#endif

	return 0;
}

VIDEO_UPDATE( truxton2 )
{
	VIDEO_UPDATE_CALL(toaplan2);
	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
	return 0;
}


VIDEO_UPDATE( batrider )
{
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
		clip.min_y = clip.max_y = line;
		tilemap_set_scrolly(tx_tilemap,0,toaplan2_txvideoram16_offs[line&0xff]-line);
		tilemap_draw(bitmap,&clip,tx_tilemap,0,0);
	}
	return 0;
}



/* How do the dual VDP games mix? The internal mixing of each VDP chip is independent, if you view only a single
   VDP then the priorities for that VDP are correct, however, it is completely unclear how the priorities of the
   two VDPs should actually mix together, as a result these games are broken for now. */
VIDEO_UPDATE( dogyuun )
{
#ifdef DUAL_SCREEN_VDPS
	VIDEO_UPDATE_CALL( toaplan2 );
#else
	toaplan2_state *state = screen->machine->driver_data<toaplan2_state>();

	bitmap_fill(bitmap,cliprect,0);
	bitmap_fill(gp9001_custom_priority_bitmap, cliprect, 0);

	state->vdp1->toaplan2_draw_custom_tilemap( screen->machine, bitmap, state->vdp1->bg_tilemap, toaplan2_primap1, batsugun_prienable0);
	state->vdp0->toaplan2_draw_custom_tilemap( screen->machine, bitmap, state->vdp0->bg_tilemap, toaplan2_primap1, batsugun_prienable0);
	state->vdp1->toaplan2_draw_custom_tilemap( screen->machine, bitmap, state->vdp1->fg_tilemap, toaplan2_primap1, batsugun_prienable0);
	state->vdp0->toaplan2_draw_custom_tilemap( screen->machine, bitmap, state->vdp0->fg_tilemap, toaplan2_primap1, batsugun_prienable0);
	state->vdp1->toaplan2_draw_custom_tilemap( screen->machine, bitmap, state->vdp1->top_tilemap, toaplan2_primap1, batsugun_prienable0);
	state->vdp0->toaplan2_draw_custom_tilemap( screen->machine, bitmap, state->vdp0->top_tilemap, toaplan2_primap1, batsugun_prienable0);

	state->vdp1->draw_sprites(screen->machine,bitmap,cliprect, toaplan2_sprprimap1);
	state->vdp0->draw_sprites(screen->machine,bitmap,cliprect, toaplan2_sprprimap1);
#endif

	return 0;
}

VIDEO_UPDATE( batsugun )
{
	toaplan2_state *state = screen->machine->driver_data<toaplan2_state>();
	state->vdp1->tile_limit = 0x1fff; // 0x2000-0x3fff seem to be for sprites only? (corruption on level 1 otherwise)


#ifdef DUAL_SCREEN_VDPS
	VIDEO_UPDATE_CALL( toaplan2 );
#else
	bitmap_fill(bitmap,cliprect,0);
	bitmap_fill(gp9001_custom_priority_bitmap, cliprect, 0);


	state->vdp1->toaplan2_draw_custom_tilemap( screen->machine, bitmap, state->vdp1->bg_tilemap, toaplan2_primap1, batsugun_prienable0);
	state->vdp0->toaplan2_draw_custom_tilemap( screen->machine, bitmap, state->vdp0->bg_tilemap, toaplan2_primap1, batsugun_prienable0);
	state->vdp1->toaplan2_draw_custom_tilemap( screen->machine, bitmap, state->vdp1->fg_tilemap, toaplan2_primap1, batsugun_prienable0);
	state->vdp0->toaplan2_draw_custom_tilemap( screen->machine, bitmap, state->vdp0->fg_tilemap, toaplan2_primap1, batsugun_prienable0);
	state->vdp1->toaplan2_draw_custom_tilemap( screen->machine, bitmap, state->vdp1->top_tilemap, toaplan2_primap1, batsugun_prienable0);
	state->vdp0->toaplan2_draw_custom_tilemap( screen->machine, bitmap, state->vdp0->top_tilemap, toaplan2_primap1, batsugun_prienable0);

	state->vdp1->draw_sprites(screen->machine,bitmap,cliprect, toaplan2_sprprimap1);
	state->vdp0->draw_sprites(screen->machine,bitmap,cliprect, toaplan2_sprprimap1);
#endif

	return 0;
}

VIDEO_EOF( toaplan2 )
{
	toaplan2_state *state = machine->driver_data<toaplan2_state>();
	if (state->vdp0) state->vdp0->gp9001_video_eof();
	if (state->vdp1) state->vdp1->gp9001_video_eof();
}
