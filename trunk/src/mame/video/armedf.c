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

static TILE_GET_INFO( get_nb1414m4_tx_tile_info )
{
	armedf_state *state = machine.driver_data<armedf_state>();
	int tile_number = state->m_text_videoram[tile_index] & 0xff;
	int attributes;

	/* TODO: Armed F doesn't seem to use the NB1414M4! */
	//if (state->m_scroll_type == 1)
	//  attributes = state->m_text_videoram[tile_index + 0x800] & 0xff;
	//else
	{
		attributes = state->m_text_videoram[tile_index + 0x400] & 0xff;

		if(tile_index < 0x12) /* don't draw the NB1414M4 params! TODO: could be a better fix */
			tile_number = attributes = 0x00;
	}

	/* bit 3 controls priority, (0) nb1414m4 has priority over all the other video layers */
	tileinfo->category = (attributes & 0x8) >> 3;

	SET_TILE_INFO(
			0,
			tile_number + 256 * (attributes & 0x3),
			attributes >> 4,
			0);
}

static TILE_GET_INFO( get_armedf_tx_tile_info )
{
	armedf_state *state = machine.driver_data<armedf_state>();
	int tile_number = state->m_text_videoram[tile_index] & 0xff;
	int attributes;

	/* TODO: Armed F doesn't seem to use the NB1414M4! */
	//if (state->m_scroll_type == 1)
		attributes = state->m_text_videoram[tile_index + 0x800] & 0xff;
	//else
	//{
	//  attributes = state->m_text_videoram[tile_index + 0x400] & 0xff;
//
	//  if(tile_index < 0x12) /* don't draw the NB1414M4 params! TODO: could be a better fix */
	//      tile_number = attributes = 0x00;
	//}

	/* bit 3 controls priority, (0) nb1414m4 has priority over all the other video layers */
	tileinfo->category = (attributes & 0x8) >> 3;

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

VIDEO_START( terraf )
{
	armedf_state *state = machine.driver_data<armedf_state>();

	state->m_sprite_offy = (state->m_scroll_type & 2 ) ? 0 : 128;  /* legion, legiono, crazy climber 2 */

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols, 16, 16, 64, 32);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_cols, 16, 16, 64, 32);

	state->m_tx_tilemap = tilemap_create(machine, get_nb1414m4_tx_tile_info, (state->m_scroll_type == 2) ? armedf_scan_type3 : armedf_scan_type2, 8, 8, 64, 32);

	tilemap_set_transparent_pen(state->m_bg_tilemap, 0xf);
	tilemap_set_transparent_pen(state->m_fg_tilemap, 0xf);
	tilemap_set_transparent_pen(state->m_tx_tilemap, 0xf);

	if (state->m_scroll_type != 1)
		tilemap_set_scrollx(state->m_tx_tilemap, 0, -128);

	state->m_text_videoram = auto_alloc_array(machine, UINT8, 0x1000);
}

VIDEO_START( armedf )
{
	armedf_state *state = machine.driver_data<armedf_state>();

	state->m_sprite_offy = (state->m_scroll_type & 2 ) ? 0 : 128;  /* legion, legiono, crazy climber 2 */

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols, 16, 16, 64, 32);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_cols, 16, 16, 64, 32);

	state->m_tx_tilemap = tilemap_create(machine, get_armedf_tx_tile_info, armedf_scan_type1, 8, 8, 64, 32);

	tilemap_set_transparent_pen(state->m_bg_tilemap, 0xf);
	tilemap_set_transparent_pen(state->m_fg_tilemap, 0xf);
	tilemap_set_transparent_pen(state->m_tx_tilemap, 0xf);

	if (state->m_scroll_type != 1)
		tilemap_set_scrollx(state->m_tx_tilemap, 0, -128);

	state->m_text_videoram = auto_alloc_array(machine, UINT8, 0x1000);
}

/***************************************************************************

  Memory handlers

***************************************************************************/

READ8_HANDLER( nb1414m4_text_videoram_r )
{
	armedf_state *state = space->machine().driver_data<armedf_state>();

	return state->m_text_videoram[offset];
}

WRITE8_HANDLER( nb1414m4_text_videoram_w )
{
	armedf_state *state = space->machine().driver_data<armedf_state>();

	state->m_text_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_tx_tilemap, offset & 0x7ff);
}

READ8_HANDLER( armedf_text_videoram_r )
{
	armedf_state *state = space->machine().driver_data<armedf_state>();

	return state->m_text_videoram[offset];
}

WRITE8_HANDLER( armedf_text_videoram_w )
{
	armedf_state *state = space->machine().driver_data<armedf_state>();
	state->m_text_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_tx_tilemap, offset & 0x7ff);
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

/* custom code to handle color cycling effect, handled by m_spr_pal_clut */
void armedf_drawgfx(running_machine &machine, bitmap_t *dest_bmp,const rectangle *clip,const gfx_element *gfx,
							UINT32 code,UINT32 color, UINT32 clut,int flipx,int flipy,int offsx,int offsy,
							int transparent_color)
{
	armedf_state *state = machine.driver_data<armedf_state>();
	const pen_t *pal = &gfx->machine().pens[gfx->color_base + gfx->color_granularity * (color % gfx->total_colors)];
	const UINT8 *source_base = gfx_element_get_data(gfx, code % gfx->total_elements);
	int x_index_base, y_index, sx, sy, ex, ey;
	int xinc, yinc;

	xinc = flipx ? -1 : 1;
	yinc = flipy ? -1 : 1;

	x_index_base = flipx ? gfx->width-1 : 0;
	y_index = flipy ? gfx->height-1 : 0;

	/* start coordinates */
	sx = offsx;
	sy = offsy;

	/* end coordinates */
	ex = sx + gfx->width;
	ey = sy + gfx->height;

	if (clip)
	{
		if (sx < clip->min_x)
		{ /* clip left */
			int pixels = clip->min_x-sx;
			sx += pixels;
			x_index_base += xinc*pixels;
		}
		if (sy < clip->min_y)
		{ /* clip top */
			int pixels = clip->min_y-sy;
			sy += pixels;
			y_index += yinc*pixels;
		}
		/* NS 980211 - fixed incorrect clipping */
		if (ex > clip->max_x+1)
		{ /* clip right */
			ex = clip->max_x+1;
		}
		if (ey > clip->max_y+1)
		{ /* clip bottom */
			ey = clip->max_y+1;
		}
	}

	if (ex > sx)
	{ /* skip if inner loop doesn't draw anything */
		int x, y;

		{
			for (y = sy; y < ey; y++)
			{
				const UINT8 *source = source_base + y_index*gfx->line_modulo;
				UINT16 *dest = BITMAP_ADDR16(dest_bmp, y, 0);
				int x_index = x_index_base;
				for (x = sx; x < ex; x++)
				{
					int c = (source[x_index] & ~0xf) | ((state->m_spr_pal_clut[clut*0x10+(source[x_index] & 0xf)]) & 0xf);
					if (c != transparent_color)
						dest[x] = pal[c];

					x_index += xinc;
				}
				y_index += yinc;
			}
		}
	}
}


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
		int clut = (buffered_spriteram[offs + 2]) & 0x7f;
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
			armedf_drawgfx(machine,bitmap,cliprect,machine.gfx[3],
				code & 0xfff,
				color,clut,
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

	tilemap_draw(bitmap, cliprect, state->m_tx_tilemap, TILEMAP_DRAW_CATEGORY(1), 0);

	tilemap_draw(bitmap, cliprect, state->m_bg_tilemap, 0, 0);

	if (sprite_enable)
		draw_sprites(screen->machine(), bitmap, cliprect, 2);

	tilemap_draw(bitmap, cliprect, state->m_fg_tilemap, 0, 0);

	if (sprite_enable)
		draw_sprites(screen->machine(), bitmap, cliprect, 1);

	if (sprite_enable)
		draw_sprites(screen->machine(), bitmap, cliprect, 0);

	tilemap_draw(bitmap, cliprect, state->m_tx_tilemap, TILEMAP_DRAW_CATEGORY(0), 0);

	return 0;
}


SCREEN_EOF( armedf )
{
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	buffer_spriteram16_w(space, 0, 0, 0xffff);
}
