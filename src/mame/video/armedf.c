// license:BSD-3-Clause
// copyright-holders:David Haywood, Phil Stroffolino, Carlos A. Lozano
/***************************************************************************

  Armed Formation video emulation

***************************************************************************/

#include "emu.h"
#include "includes/armedf.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILEMAP_MAPPER_MEMBER(armedf_state::armedf_scan_type1)
{   /* col: 0..63; row: 0..31 */
	/* armed formation */
	return col * 32 + row;
}

TILEMAP_MAPPER_MEMBER(armedf_state::armedf_scan_type2)
{   /* col: 0..63; row: 0..31 */
	return 32 * (31 - row) + (col & 0x1f) + 0x800 * (col / 32);
}

TILEMAP_MAPPER_MEMBER(armedf_state::armedf_scan_type3)
{   /* col: 0..63; row: 0..31 */
	/* legion & legiono */
	return (col & 0x1f) * 32 + row + 0x800 * (col / 32);
}

TILE_GET_INFO_MEMBER(armedf_state::get_nb1414m4_tx_tile_info)
{
	int tile_number = m_text_videoram[tile_index] & 0xff;
	int attributes;

	/* TODO: Armed F doesn't seem to use the NB1414M4! */
	//if (m_scroll_type == 1)
	//  attributes = m_text_videoram[tile_index + 0x800] & 0xff;
	//else
	{
		attributes = m_text_videoram[tile_index + 0x400] & 0xff;

		if(tile_index < 0x12) /* don't draw the NB1414M4 params! TODO: could be a better fix */
			tile_number = attributes = 0x00;
	}

	/* bit 3 controls priority, (0) nb1414m4 has priority over all the other video layers */
	tileinfo.category = (attributes & 0x8) >> 3;

	SET_TILE_INFO_MEMBER(0,
			tile_number + 256 * (attributes & 0x3),
			attributes >> 4,
			0);
}

TILE_GET_INFO_MEMBER(armedf_state::get_armedf_tx_tile_info)
{
	int tile_number = m_text_videoram[tile_index] & 0xff;
	int attributes;

	/* TODO: Armed F doesn't seem to use the NB1414M4! */
	//if (m_scroll_type == 1)
		attributes = m_text_videoram[tile_index + 0x800] & 0xff;
	//else
	//{
	//  attributes = m_text_videoram[tile_index + 0x400] & 0xff;
//
	//  if(tile_index < 0x12) /* don't draw the NB1414M4 params! TODO: could be a better fix */
	//      tile_number = attributes = 0x00;
	//}

	/* bit 3 controls priority, (0) nb1414m4 has priority over all the other video layers */
	tileinfo.category = (attributes & 0x8) >> 3;

	SET_TILE_INFO_MEMBER(0,
			tile_number + 256 * (attributes & 0x3),
			attributes >> 4,
			0);
}


TILE_GET_INFO_MEMBER(armedf_state::get_fg_tile_info)
{
	int data = m_fg_videoram[tile_index];
	SET_TILE_INFO_MEMBER(1,
			data&0x7ff,
			data>>11,
			0);
}


TILE_GET_INFO_MEMBER(armedf_state::get_bg_tile_info)
{
	int data = m_bg_videoram[tile_index];
	SET_TILE_INFO_MEMBER(2,
			data & 0x3ff,
			data >> 11,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(armedf_state,terraf)
{
	m_sprite_offy = (m_scroll_type & 2 ) ? 0 : 128;  /* legion, legiono, crazy climber 2 */

	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(armedf_state::get_bg_tile_info),this), TILEMAP_SCAN_COLS, 16, 16, 64, 32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(armedf_state::get_fg_tile_info),this), TILEMAP_SCAN_COLS, 16, 16, 64, 32);

	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(armedf_state::get_nb1414m4_tx_tile_info),this),
		(m_scroll_type == 2) ? tilemap_mapper_delegate(FUNC(armedf_state::armedf_scan_type3),this) : tilemap_mapper_delegate(FUNC(armedf_state::armedf_scan_type2),this), 8, 8, 64, 32);

	m_bg_tilemap->set_transparent_pen(0xf);
	m_fg_tilemap->set_transparent_pen(0xf);
	m_tx_tilemap->set_transparent_pen(0xf);

	if (m_scroll_type != 1)
		m_tx_tilemap->set_scrollx(0, -128);

	m_text_videoram = auto_alloc_array(machine(), UINT8, 0x1000);
	memset(m_text_videoram, 0x00, 0x1000);

	save_pointer(NAME(m_text_videoram), 0x1000);
}

VIDEO_START_MEMBER(armedf_state,armedf)
{
	m_sprite_offy = (m_scroll_type & 2 ) ? 0 : 128;  /* legion, legiono, crazy climber 2 */

	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(armedf_state::get_bg_tile_info),this), TILEMAP_SCAN_COLS, 16, 16, 64, 32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(armedf_state::get_fg_tile_info),this), TILEMAP_SCAN_COLS, 16, 16, 64, 32);

	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(armedf_state::get_armedf_tx_tile_info),this), tilemap_mapper_delegate(FUNC(armedf_state::armedf_scan_type1),this), 8, 8, 64, 32);

	m_bg_tilemap->set_transparent_pen(0xf);
	m_fg_tilemap->set_transparent_pen(0xf);
	m_tx_tilemap->set_transparent_pen(0xf);

	if (m_scroll_type != 1)
		m_tx_tilemap->set_scrollx(0, -128);

	m_text_videoram = auto_alloc_array(machine(), UINT8, 0x1000);
	memset(m_text_videoram, 0x00, 0x1000);

	save_pointer(NAME(m_text_videoram), 0x1000);
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
void armedf_state::armedf_drawgfx(bitmap_ind16 &dest_bmp,const rectangle &clip,gfx_element *gfx,
							UINT32 code,UINT32 color, UINT32 clut,int flipx,int flipy,int offsx,int offsy,
							int transparent_color)
{
	const pen_t *pal = &m_palette->pen(gfx->colorbase() + gfx->granularity() * (color % gfx->colors()));
	const UINT8 *source_base = gfx->get_data(code % gfx->elements());
	int x_index_base, y_index, sx, sy, ex, ey;
	int xinc, yinc;

	xinc = flipx ? -1 : 1;
	yinc = flipy ? -1 : 1;

	x_index_base = flipx ? gfx->width()-1 : 0;
	y_index = flipy ? gfx->height()-1 : 0;

	/* start coordinates */
	sx = offsx;
	sy = offsy;

	/* end coordinates */
	ex = sx + gfx->width();
	ey = sy + gfx->height();

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
				const UINT8 *source = source_base + y_index*gfx->rowbytes();
				UINT16 *dest = &dest_bmp.pix16(y);
				int x_index = x_index_base;
				for (x = sx; x < ex; x++)
				{
					int c = (source[x_index] & ~0xf) | ((m_spr_pal_clut[clut*0x10+(source[x_index] & 0xf)]) & 0xf);
					if (c != transparent_color)
						dest[x] = pal[c];

					x_index += xinc;
				}
				y_index += yinc;
			}
		}
	}
}


void armedf_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int priority )
{
	UINT16 *buffered_spriteram = m_spriteram->buffer();
	int offs;

	for (offs = 0; offs < m_spriteram->bytes() / 2; offs += 4)
	{
		int code = buffered_spriteram[offs + 1]; /* ??YX?TTTTTTTTTTT */
		int flipx = code & 0x2000;
		int flipy = code & 0x1000;
		int color = (buffered_spriteram[offs + 2] >> 8) & 0x1f;
		int clut = (buffered_spriteram[offs + 2]) & 0x7f;
		int sx = buffered_spriteram[offs + 3];
		int sy = m_sprite_offy + 240 - (buffered_spriteram[offs + 0] & 0x1ff);

		if (flip_screen())
		{
			sx = 320 - sx + 176;    /* don't ask where 176 comes from, just tried it out */
			sy = 240 - sy + 1;  /* don't ask where 1 comes from, just tried it out */
			flipx = !flipx;     /* the values seem to result in pixel-correct placement */
			flipy = !flipy;     /* in all the games supported by this driver */
		}

		if (((buffered_spriteram[offs + 0] & 0x3000) >> 12) == priority)
		{
			armedf_drawgfx(bitmap,cliprect,m_gfxdecode->gfx(3),
				code & 0xfff,
				color,clut,
				flipx,flipy,
				sx,sy,15);
		}
	}
}

UINT32 armedf_state::screen_update_armedf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int sprite_enable = m_vreg & 0x200;

	m_bg_tilemap->enable(m_vreg & 0x800);
	m_fg_tilemap->enable(m_vreg & 0x400);
	m_tx_tilemap->enable(m_vreg & 0x100);

	switch (m_scroll_type)
	{
		case 0: /* terra force, kozure ookami */
		case 2: /* legion */
		case 3: /* crazy climber */
			m_fg_tilemap->set_scrollx(0, (m_fg_scrollx & 0x3ff));
			m_fg_tilemap->set_scrolly(0, (m_fg_scrolly & 0x3ff));
			break;

		case 1: /* armed formation */
			m_fg_tilemap->set_scrollx(0, m_fg_scrollx);
			m_fg_tilemap->set_scrolly(0, m_fg_scrolly);
			break;

	}

	bitmap.fill(0xff, cliprect );

	m_tx_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(1), 0);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	if (sprite_enable)
		draw_sprites(bitmap, cliprect, 2);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	if (sprite_enable)
		draw_sprites(bitmap, cliprect, 1);

	m_tx_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(0), 0);

	if (sprite_enable)
		draw_sprites(bitmap, cliprect, 0);

	return 0;
}
