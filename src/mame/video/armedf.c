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
	tileinfo.category = (attributes & 0x8) >> 3;

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
	tileinfo.category = (attributes & 0x8) >> 3;

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

	state->m_bg_tilemap->set_transparent_pen(0xf);
	state->m_fg_tilemap->set_transparent_pen(0xf);
	state->m_tx_tilemap->set_transparent_pen(0xf);

	if (state->m_scroll_type != 1)
		state->m_tx_tilemap->set_scrollx(0, -128);

	state->m_text_videoram = auto_alloc_array(machine, UINT8, 0x1000);
	memset(state->m_text_videoram, 0x00, 0x1000);
}

VIDEO_START( armedf )
{
	armedf_state *state = machine.driver_data<armedf_state>();

	state->m_sprite_offy = (state->m_scroll_type & 2 ) ? 0 : 128;  /* legion, legiono, crazy climber 2 */

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols, 16, 16, 64, 32);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_cols, 16, 16, 64, 32);

	state->m_tx_tilemap = tilemap_create(machine, get_armedf_tx_tile_info, armedf_scan_type1, 8, 8, 64, 32);

	state->m_bg_tilemap->set_transparent_pen(0xf);
	state->m_fg_tilemap->set_transparent_pen(0xf);
	state->m_tx_tilemap->set_transparent_pen(0xf);

	if (state->m_scroll_type != 1)
		state->m_tx_tilemap->set_scrollx(0, -128);

	state->m_text_videoram = auto_alloc_array(machine, UINT8, 0x1000);
	memset(state->m_text_videoram, 0x00, 0x1000);
}

/***************************************************************************

  Memory handlers

***************************************************************************/

READ8_MEMBER(armedf_state::nb1414m4_text_videoram_r)
{

	return m_text_videoram[offset];
}

WRITE8_MEMBER(armedf_state::nb1414m4_text_videoram_w)
{

	m_text_videoram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset & 0x7ff);
}

READ8_MEMBER(armedf_state::armedf_text_videoram_r)
{

	return m_text_videoram[offset];
}

WRITE8_MEMBER(armedf_state::armedf_text_videoram_w)
{
	m_text_videoram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset & 0x7ff);
}

WRITE16_MEMBER(armedf_state::armedf_fg_videoram_w)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(armedf_state::armedf_bg_videoram_w)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(armedf_state::terraf_fg_scrolly_w)
{
	if (ACCESSING_BITS_8_15)
	{
		m_fg_scrolly = ((data >> 8) & 0xff) | (m_fg_scrolly & 0x300);
		m_waiting_msb = 1;
	}
}

WRITE16_MEMBER(armedf_state::terraf_fg_scrollx_w)
{
	if (ACCESSING_BITS_8_15)
	{
		if (m_waiting_msb)
		{
			m_scroll_msb = data >> 8;
			m_fg_scrollx = (m_fg_scrollx & 0xff) | (((m_scroll_msb >> 4) & 3) << 8);
			m_fg_scrolly = (m_fg_scrolly & 0xff) | (((m_scroll_msb >> 0) & 3) << 8);
			//popmessage("%04X %04X %04X",data,m_fg_scrollx,m_fg_scrolly);
		}
		else
			m_fg_scrollx = ((data >> 8) & 0xff) | (m_fg_scrollx & 0x300);
	}
}

WRITE16_MEMBER(armedf_state::terraf_fg_scroll_msb_arm_w)
{
	if (ACCESSING_BITS_8_15)
		m_waiting_msb = 0;
}

WRITE16_MEMBER(armedf_state::armedf_fg_scrollx_w)
{
	COMBINE_DATA(&m_fg_scrollx);
}

WRITE16_MEMBER(armedf_state::armedf_fg_scrolly_w)
{
	COMBINE_DATA(&m_fg_scrolly);
}

WRITE16_MEMBER(armedf_state::armedf_bg_scrollx_w)
{
	COMBINE_DATA(&m_bg_scrollx);
	m_bg_tilemap->set_scrollx(0, m_bg_scrollx);
}

WRITE16_MEMBER(armedf_state::armedf_bg_scrolly_w)
{
	COMBINE_DATA(&m_bg_scrolly);
	m_bg_tilemap->set_scrolly(0, m_bg_scrolly);
}



/***************************************************************************

  Display refresh

***************************************************************************/

/* custom code to handle color cycling effect, handled by m_spr_pal_clut */
void armedf_drawgfx(running_machine &machine, bitmap_ind16 &dest_bmp,const rectangle &clip,const gfx_element *gfx,
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

	if (sx < clip.min_x)
	{ /* clip left */
		int pixels = clip.min_x-sx;
		sx += pixels;
		x_index_base += xinc*pixels;
	}
	if (sy < clip.min_y)
	{ /* clip top */
		int pixels = clip.min_y-sy;
		sy += pixels;
		y_index += yinc*pixels;
	}
	/* NS 980211 - fixed incorrect clipping */
	if (ex > clip.max_x+1)
	{ /* clip right */
		ex = clip.max_x+1;
	}
	if (ey > clip.max_y+1)
	{ /* clip bottom */
		ey = clip.max_y+1;
	}

	if (ex > sx)
	{ /* skip if inner loop doesn't draw anything */
		int x, y;

		{
			for (y = sy; y < ey; y++)
			{
				const UINT8 *source = source_base + y_index*gfx->line_modulo;
				UINT16 *dest = &dest_bmp.pix16(y);
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


static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority )
{
	armedf_state *state = machine.driver_data<armedf_state>();
	UINT16 *buffered_spriteram = state->m_spriteram->buffer();
	int offs;

	for (offs = 0; offs < state->m_spriteram->bytes() / 2; offs += 4)
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

SCREEN_UPDATE_IND16( armedf )
{
	armedf_state *state = screen.machine().driver_data<armedf_state>();
	int sprite_enable = state->m_vreg & 0x200;

	state->m_bg_tilemap->enable(state->m_vreg & 0x800);
	state->m_fg_tilemap->enable(state->m_vreg & 0x400);
	state->m_tx_tilemap->enable(state->m_vreg & 0x100);

	switch (state->m_scroll_type)
	{
		case 0:	/* terra force, kozure ookami */
		case 2: /* legion */
		case 3:	/* crazy climber */
			state->m_fg_tilemap->set_scrollx(0, (state->m_fg_scrollx & 0x3ff));
			state->m_fg_tilemap->set_scrolly(0, (state->m_fg_scrolly & 0x3ff));
			break;

		case 1: /* armed formation */
			state->m_fg_tilemap->set_scrollx(0, state->m_fg_scrollx);
			state->m_fg_tilemap->set_scrolly(0, state->m_fg_scrolly);
			break;

	}

	bitmap.fill(0xff, cliprect );

	state->m_tx_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_CATEGORY(1), 0);

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);

	if (sprite_enable)
		draw_sprites(screen.machine(), bitmap, cliprect, 2);

	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);

	if (sprite_enable)
		draw_sprites(screen.machine(), bitmap, cliprect, 1);

	if (sprite_enable)
		draw_sprites(screen.machine(), bitmap, cliprect, 0);

	state->m_tx_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_CATEGORY(0), 0);

	return 0;
}
