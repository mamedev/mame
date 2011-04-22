/***************************************************************************

  Armed Formation video emulation

***************************************************************************/

#include "emu.h"
#include "includes/armedf.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILEMAP_MAPPER( armedf_scan_type1 )
{	/* col: 0..63; row: 0..31 */
	/* armed formation */
	return col * 32 + row;
}

static TILEMAP_MAPPER( armedf_scan_type2 )
{	/* col: 0..63; row: 0..31 */
	return 32 * (31 - row) + (col & 0x1f) + 0x800 * (col / 32);
}

static TILEMAP_MAPPER( armedf_scan_type3 )
{	/* col: 0..63; row: 0..31 */
	/* legion & legiono */
	return (col & 0x1f) * 32 + row + 0x800 * (col / 32);
}

static TILE_GET_INFO( get_tx_tile_info )
{
	armedf_state *state = machine.driver_data<armedf_state>();
	int tile_number = state->m_text_videoram[tile_index] & 0xff;
	int attributes;

	if (state->m_scroll_type == 1)
		attributes = state->m_text_videoram[tile_index + 0x800] & 0xff;
	else
		attributes = state->m_text_videoram[tile_index + 0x400] & 0xff;

	SET_TILE_INFO(
			0,
			tile_number + 256 * (attributes & 0x3),
			attributes >> 4,
			0);
}

static TILE_GET_INFO( get_legion_tx_tile_info )
{
	armedf_state *state = machine.driver_data<armedf_state>();

	int tile_number = state->m_text_videoram[tile_index] & 0xff;

	if(tile_index<0x10) tile_number=0x20;

	int attributes;

	attributes = state->m_text_videoram[tile_index + 0x400] & 0xff;

	tileinfo->category = 0;

	if((attributes & 0x3) == 3)
	{
		tileinfo->category = 1;
	}

	SET_TILE_INFO(
			0,
			tile_number + 256 * (attributes & 0x3),
			attributes >> 4,
			0);
}


static TILE_GET_INFO( get_fg_tile_info )
{
	armedf_state *state = machine.driver_data<armedf_state>();
	int data = state->m_fg_videoram[tile_index];
	SET_TILE_INFO(
			1,
			data&0x7ff,
			data>>11,
			0);
}


static TILE_GET_INFO( get_bg_tile_info )
{
	armedf_state *state = machine.driver_data<armedf_state>();
	int data = state->m_bg_videoram[tile_index];
	SET_TILE_INFO(
			2,
			data & 0x3ff,
			data >> 11,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( armedf )
{
	armedf_state *state = machine.driver_data<armedf_state>();

	state->m_sprite_offy = (state->m_scroll_type & 2 ) ? 0 : 128;  /* legion, legiono, crazy climber 2 */

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols, 16, 16, 64, 32);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_cols, 16, 16, 64, 32);

	switch (state->m_scroll_type)
	{
		case 1: /* armed formation */
			state->m_tx_tilemap = tilemap_create(machine, get_tx_tile_info, armedf_scan_type1, 8, 8, 64, 32);
			break;

		case 2: /* legion */
			state->m_tx_tilemap = tilemap_create(machine, get_legion_tx_tile_info, armedf_scan_type3, 8, 8, 64, 32);
			break;

		default:
			state->m_tx_tilemap = tilemap_create(machine, get_tx_tile_info, armedf_scan_type2, 8, 8, 64, 32);
			break;
	}

	tilemap_set_transparent_pen(state->m_bg_tilemap, 0xf);
	tilemap_set_transparent_pen(state->m_fg_tilemap, 0xf);
	tilemap_set_transparent_pen(state->m_tx_tilemap, 0xf);

	if (state->m_scroll_type != 1)
		tilemap_set_scrollx(state->m_tx_tilemap, 0, -128);
}

/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_HANDLER( armedf_text_videoram_w )
{
	armedf_state *state = space->machine().driver_data<armedf_state>();
	COMBINE_DATA(&state->m_text_videoram[offset]);
	if (state->m_scroll_type == 1)
		tilemap_mark_tile_dirty(state->m_tx_tilemap, offset & 0x7ff);
	else
		tilemap_mark_tile_dirty(state->m_tx_tilemap, offset & 0xbff);
/*
  if (offset < 0x10)
        logerror("%04x %04x %04x %04x %04x %04x %04x %04x-%04x %04x %04x %04x %04x %04x %04x %04x (%04x)\n",
            state->m_text_videoram[0], state->m_text_videoram[1], state->m_text_videoram[2],
            state->m_text_videoram[3], state->m_text_videoram[4], state->m_text_videoram[5],
            state->m_text_videoram[6], state->m_text_videoram[7], state->m_text_videoram[8],
            state->m_text_videoram[9], state->m_text_videoram[10], state->m_text_videoram[11],
            state->m_text_videoram[12], state->m_text_videoram[13], state->m_text_videoram[14],
            state->m_text_videoram[15], offset);
*/
}

WRITE16_HANDLER( armedf_fg_videoram_w )
{
	armedf_state *state = space->machine().driver_data<armedf_state>();
	COMBINE_DATA(&state->m_fg_videoram[offset]);
	tilemap_mark_tile_dirty(state->m_fg_tilemap, offset);
}

WRITE16_HANDLER( armedf_bg_videoram_w )
{
	armedf_state *state = space->machine().driver_data<armedf_state>();
	COMBINE_DATA(&state->m_bg_videoram[offset]);
	tilemap_mark_tile_dirty(state->m_bg_tilemap, offset);
}

WRITE16_HANDLER( terraf_fg_scrolly_w )
{
	armedf_state *state = space->machine().driver_data<armedf_state>();
	if (ACCESSING_BITS_8_15)
	{
		state->m_fg_scrolly = ((data >> 8) & 0xff) | (state->m_fg_scrolly & 0x300);
		state->m_waiting_msb = 1;
	}
}

WRITE16_HANDLER( terraf_fg_scrollx_w )
{
	armedf_state *state = space->machine().driver_data<armedf_state>();
	if (ACCESSING_BITS_8_15)
	{
		if (state->m_waiting_msb)
		{
			state->m_scroll_msb = data >> 8;
			state->m_fg_scrollx = (state->m_fg_scrollx & 0xff) | (((state->m_scroll_msb >> 4) & 3) << 8);
			state->m_fg_scrolly = (state->m_fg_scrolly & 0xff) | (((state->m_scroll_msb >> 0) & 3) << 8);
			//popmessage("%04X %04X %04X",data,state->m_fg_scrollx,state->m_fg_scrolly);
		}
		else
			state->m_fg_scrollx = ((data >> 8) & 0xff) | (state->m_fg_scrollx & 0x300);
	}
}

WRITE16_HANDLER( terraf_fg_scroll_msb_arm_w )
{
	armedf_state *state = space->machine().driver_data<armedf_state>();
	if (ACCESSING_BITS_8_15)
		state->m_waiting_msb = 0;
}

WRITE16_HANDLER( armedf_fg_scrollx_w )
{
	armedf_state *state = space->machine().driver_data<armedf_state>();
	COMBINE_DATA(&state->m_fg_scrollx);
}

WRITE16_HANDLER( armedf_fg_scrolly_w )
{
	armedf_state *state = space->machine().driver_data<armedf_state>();
	COMBINE_DATA(&state->m_fg_scrolly);
}

WRITE16_HANDLER( armedf_bg_scrollx_w )
{
	armedf_state *state = space->machine().driver_data<armedf_state>();
	COMBINE_DATA(&state->m_bg_scrollx);
	tilemap_set_scrollx(state->m_bg_tilemap, 0, state->m_bg_scrollx);
}

WRITE16_HANDLER( armedf_bg_scrolly_w )
{
	armedf_state *state = space->machine().driver_data<armedf_state>();
	COMBINE_DATA(&state->m_bg_scrolly);
	tilemap_set_scrolly(state->m_bg_tilemap, 0, state->m_bg_scrolly);
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites( running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, int priority )
{
	UINT16 *buffered_spriteram = machine.generic.buffered_spriteram.u16;
	armedf_state *state = machine.driver_data<armedf_state>();
	int offs;

	for (offs = 0; offs < machine.generic.spriteram_size / 2; offs += 4)
	{
		int code = buffered_spriteram[offs + 1]; /* ??YX?TTTTTTTTTTT */
		int flipx = code & 0x2000;
		int flipy = code & 0x1000;
		int color = (buffered_spriteram[offs + 2] >> 8) & 0x1f;
		int sx = buffered_spriteram[offs + 3];
		int sy = state->m_sprite_offy + 240 - (buffered_spriteram[offs + 0] & 0x1ff);

		if (flip_screen_get(machine))
		{
			sx = 320 - sx + 176;	/* don't ask where 176 comes from, just tried it out */
			sy = 240 - sy + 1;	/* don't ask where 1 comes from, just tried it out */
			flipx = !flipx;		/* the values seem to result in pixel-correct placement */
			flipy = !flipy;		/* in all the games supported by this driver */
		}

		if (((buffered_spriteram[offs + 0] & 0x3000) >> 12) == priority)
		{
			drawgfx_transpen(bitmap,cliprect,machine.gfx[3],
				code & 0xfff,
				color,
				flipx,flipy,
				sx,sy,15);
		}
	}
}

SCREEN_UPDATE( armedf )
{
	armedf_state *state = screen->machine().driver_data<armedf_state>();
	int sprite_enable = state->m_vreg & 0x200;

	tilemap_set_enable(state->m_bg_tilemap, state->m_vreg & 0x800);
	tilemap_set_enable(state->m_fg_tilemap, state->m_vreg & 0x400);
	tilemap_set_enable(state->m_tx_tilemap, state->m_vreg & 0x100);

	switch (state->m_scroll_type)
	{
		case 0:	/* terra force, kozure ookami */
		case 2: /* legion */
		case 3:	/* crazy climber */
			tilemap_set_scrollx(state->m_fg_tilemap, 0, (state->m_fg_scrollx & 0x3ff));
			tilemap_set_scrolly(state->m_fg_tilemap, 0, (state->m_fg_scrolly & 0x3ff));
			break;

		case 1: /* armed formation */
			tilemap_set_scrollx(state->m_fg_tilemap, 0, state->m_fg_scrollx);
			tilemap_set_scrolly(state->m_fg_tilemap, 0, state->m_fg_scrolly);
			break;

	}


	bitmap_fill(bitmap, cliprect , 0xff);

	if(state->m_scroll_type == 2) /* legion / legiono */
	{
		tilemap_draw(bitmap, cliprect, state->m_tx_tilemap, 1, 0);
	}

	if (state->m_vreg & 0x0800)
		tilemap_draw(bitmap, cliprect, state->m_bg_tilemap, 0, 0);
#if 0
	if(state->m_vreg & 0x0800)
	{
		tilemap_draw(bitmap, cliprect, state->m_bg_tilemap, 0, 0);
	}
	else
	{
		bitmap_fill(bitmap, cliprect , get_black_pen(screen->machine()) & 0x0f);
	}
#endif

	if ((state->m_vreg & 0x0030) == 0x0030)
		tilemap_draw(bitmap, cliprect, state->m_tx_tilemap, 0, 0);

	if (sprite_enable)
		draw_sprites(screen->machine(), bitmap, cliprect, 2);

	if ((state->m_vreg & 0x0030) == 0x0020)
		tilemap_draw(bitmap, cliprect, state->m_tx_tilemap, 0, 0);

	tilemap_draw(bitmap, cliprect, state->m_fg_tilemap, 0, 0);

	if ((state->m_vreg & 0x0030) == 0x0010)
		tilemap_draw(bitmap, cliprect, state->m_tx_tilemap, 0, 0);

	if (sprite_enable)
		draw_sprites(screen->machine(), bitmap, cliprect, 1);

	if ((state->m_vreg & 0x0030) == 0x0000)
		tilemap_draw(bitmap, cliprect, state->m_tx_tilemap, 0, 0);

	if (sprite_enable)
		draw_sprites(screen->machine(), bitmap, cliprect, 0);


	return 0;
}


SCREEN_EOF( armedf )
{
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	buffer_spriteram16_w(space, 0, 0, 0xffff);
}
