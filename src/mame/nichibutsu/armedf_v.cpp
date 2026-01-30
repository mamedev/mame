// license:BSD-3-Clause
// copyright-holders:David Haywood, Phil Stroffolino, Carlos A. Lozano
/***************************************************************************

  Armed Formation video emulation

***************************************************************************/

#include "emu.h"
#include "armedf.h"


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
	return 32 * (col & 0x1f) + row + 0x800 * (col / 32);
}

TILE_GET_INFO_MEMBER(armedf_state::get_nb1414m4_tx_tile_info)
{
	u8 tile_number = m_text_videoram[tile_index] & 0xff;
	u8 attributes;

	/* TODO: Armed F doesn't seem to use the NB1414M4! */
	//if (m_scroll_type == 1)
	//  attributes = m_text_videoram[tile_index + 0x800] & 0xff;
	//else
	{
		attributes = m_text_videoram[tile_index + 0x400] & 0xff;

		// TODO: skip drawing the NB1414M4 params, how the HW actually handles this?
		if (tile_index < 0x12)
			tile_number = attributes = 0x00;
	}

	/* bit 3 controls priority, (0) nb1414m4 has priority over all the other video layers */
	tileinfo.category = (attributes & 0x8) >> 3;

	tileinfo.set(0,
			tile_number + 256 * (attributes & 0x3),
			attributes >> 4,
			0);
}

TILE_GET_INFO_MEMBER(armedf_state::get_armedf_tx_tile_info)
{
	u8 tile_number = m_text_videoram[tile_index] & 0xff;
	u8 attributes;

	/* TODO: Armed F doesn't seem to use the NB1414M4! */
	//if (m_scroll_type == 1)
		attributes = m_text_videoram[tile_index + 0x800] & 0xff;
	//else
	//{
	//  attributes = m_text_videoram[tile_index + 0x400] & 0xff;
//
	//  if (tile_index < 0x12) /* don't draw the NB1414M4 params! TODO: could be a better fix */
	//      tile_number = attributes = 0x00;
	//}

	/* bit 3 controls priority, (0) nb1414m4 has priority over all the other video layers */
	tileinfo.category = (attributes & 0x8) >> 3;

	tileinfo.set(0,
			tile_number + 256 * (attributes & 0x3),
			attributes >> 4,
			0);
}


TILE_GET_INFO_MEMBER(armedf_state::get_fg_tile_info)
{
	const u16 data = m_fg_videoram[tile_index];
	tileinfo.set(1,
			data & 0x7ff,
			data >> 11,
			0);
}


TILE_GET_INFO_MEMBER(armedf_state::get_bg_tile_info)
{
	const u16 data = m_bg_videoram[tile_index];
	tileinfo.set(2,
			data & 0x3ff,
			data >> 11,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(armedf_state,terraf)
{
	m_sprite_offy = (m_scroll_type & 2) ? 0 : 128;  /* legion, legiono, crazy climber 2 */

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(armedf_state::get_bg_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 64, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(armedf_state::get_fg_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 64, 32);

	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(armedf_state::get_nb1414m4_tx_tile_info)),
			(m_scroll_type == 2) ? tilemap_mapper_delegate(*this, FUNC(armedf_state::armedf_scan_type3)) : tilemap_mapper_delegate(*this, FUNC(armedf_state::armedf_scan_type2)), 8, 8, 64, 32);

	m_bg_tilemap->set_transparent_pen(0xf);
	m_fg_tilemap->set_transparent_pen(0xf);
	m_tx_tilemap->set_transparent_pen(0xf);

	if (m_scroll_type != 1)
		m_tx_tilemap->set_scrollx(0, -128);
}

VIDEO_START_MEMBER(armedf_state,armedf)
{
	m_sprite_offy = (m_scroll_type & 2) ? 0 : 128;  /* legion, legiono, crazy climber 2 */

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(armedf_state::get_bg_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 64, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(armedf_state::get_fg_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 64, 32);

	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(armedf_state::get_armedf_tx_tile_info)), tilemap_mapper_delegate(*this, FUNC(armedf_state::armedf_scan_type1)), 8, 8, 64, 32);

	m_bg_tilemap->set_transparent_pen(0xf);
	m_fg_tilemap->set_transparent_pen(0xf);
	m_tx_tilemap->set_transparent_pen(0xf);

	if (m_scroll_type != 1)
		m_tx_tilemap->set_scrollx(0, -128);
}

/***************************************************************************

  Memory handlers

***************************************************************************/

u8 armedf_state::text_videoram_r(offs_t offset)
{
	return m_text_videoram[offset];
}

void armedf_state::armedf_text_videoram_w(offs_t offset, u8 data)
{
	m_text_videoram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset & 0x7ff);
}

void armedf_state::terraf_text_videoram_w(offs_t offset, u8 data)
{
	m_text_videoram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset & 0xbff); // yes, 0xbff
}

void armedf_state::fg_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

void armedf_state::bg_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

void armedf_state::terrafb_fg_scrolly_w(u8 data)
{
	m_fg_scrolly = (data & 0xff) | (m_fg_scrolly & 0x300);
	m_waiting_msb = 1;
}

void armedf_state::terrafb_fg_scrollx_w(u8 data)
{
	if (m_waiting_msb)
	{
		m_scroll_msb = data;
		m_fg_scrollx = (m_fg_scrollx & 0xff) | (((m_scroll_msb >> 4) & 3) << 8);
		m_fg_scrolly = (m_fg_scrolly & 0xff) | (((m_scroll_msb >> 0) & 3) << 8);
		//popmessage("%04X %04X %04X",data,m_fg_scrollx,m_fg_scrolly);
	}
	else
		m_fg_scrollx = (data & 0xff) | (m_fg_scrollx & 0x300);
}

void armedf_state::terrafb_fg_scroll_msb_arm_w(u8 data)
{
	m_waiting_msb = 0;
}

void armedf_state::fg_scrollx_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_fg_scrollx);
}

void armedf_state::fg_scrolly_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_fg_scrolly);
}

void armedf_state::bg_scrollx_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bg_scrollx);
	m_bg_tilemap->set_scrollx(0, m_bg_scrollx);
}

void armedf_state::bg_scrolly_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bg_scrolly);
	m_bg_tilemap->set_scrolly(0, m_bg_scrolly);
}



/***************************************************************************

  Display refresh

***************************************************************************/

/* custom code to handle color cycling effect, handled by m_spr_pal_clut */
void armedf_state::armedf_drawgfx(bitmap_ind16 &dest_bmp, const rectangle &clip, gfx_element *gfx,
		u32 code, u32 color, u32 clut, int flipx, int flipy, int offsx, int offsy, bitmap_ind8 &primap, u32 pmask, int transparent_color)
{
	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	const pen_t *pal = &m_palette->pen(gfx->colorbase() + gfx->granularity() * (color % gfx->colors()));
	const u8 *source_base = gfx->get_data(code % gfx->elements());
	int x_index_base, y_index, sx, sy, ex, ey;
	int xinc, yinc;

	xinc = flipx ? -1 : 1;
	yinc = flipy ? -1 : 1;

	x_index_base = flipx ? gfx->width()-1 : 0;
	y_index = flipy ? gfx->height()-1 : 0;

	// start coordinates
	sx = offsx;
	sy = offsy;

	// end coordinates
	ex = sx + gfx->width();
	ey = sy + gfx->height();

	if (sx < clip.min_x)
	{
		// clip left
		int pixels = clip.min_x-sx;
		sx += pixels;
		x_index_base += xinc*pixels;
	}
	if (sy < clip.min_y)
	{
		// clip top
		int pixels = clip.min_y-sy;
		sy += pixels;
		y_index += yinc*pixels;
	}
	if (ex > clip.max_x+1)
	{
		// clip right
		ex = clip.max_x+1;
	}
	if (ey > clip.max_y+1)
	{
		// clip bottom
		ey = clip.max_y+1;
	}

	// skip if inner loop doesn't draw anything
	if (ex > sx)
	{
		for (int y = sy; y < ey; y++)
		{
			u8 const *const source = source_base + y_index*gfx->rowbytes();
			u16 *const dest = &dest_bmp.pix(y);
			u8 *const destpri = &primap.pix(y);
			int x_index = x_index_base;
			for (int x = sx; x < ex; x++)
			{
				int c = (source[x_index] & ~0xf) | ((m_spr_pal_clut[clut*0x10+(source[x_index] & 0xf)]) & 0xf);
				if (c != transparent_color)
				{
					if (((1 << (destpri[x] & 0x1f)) & pmask) == 0)
						dest[x] = pal[c];
					destpri[x] = 0x1f;
				}
				x_index += xinc;
			}
			y_index += yinc;
		}
	}
}


void armedf_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &primap )
{
	u16 *buffered_spriteram = m_spriteram->buffer();

	for (int offs = (m_spriteram->bytes() / 2) - 4; offs >= 0; offs -= 4)
	{
		u32 pmask = 0;
		int code = buffered_spriteram[offs + 1]; /* ??YX?TTTTTTTTTTT */
		int flipx = code & 0x2000;
		int flipy = code & 0x1000;
		int color = (buffered_spriteram[offs + 2] >> 8) & 0x1f;
		int clut = (buffered_spriteram[offs + 2]) & 0x7f;
		int sx = buffered_spriteram[offs + 3];
		int sy = m_sprite_offy + 240 - (buffered_spriteram[offs + 0] & 0x1ff);
		int pri = ((buffered_spriteram[offs + 0] & 0x3000) >> 12);
		if (pri == 3) // mimic previous driver behavior, correct?
			continue;

		if (pri >= 1)
			pmask |= GFX_PMASK_4;
		if (pri >= 2)
			pmask |= GFX_PMASK_2;
		/*
		if (pri >= 3)
		    pmask |= GFX_PMASK_1;
		*/

		if (flip_screen())
		{
			sx = 320 - sx + 176;    /* don't ask where 176 comes from, just tried it out */
			sy = 240 - sy + 1;  /* don't ask where 1 comes from, just tried it out */
			flipx = !flipx;     /* the values seem to result in pixel-correct placement */
			flipy = !flipy;     /* in all the games supported by this driver */
		}

		armedf_drawgfx(bitmap,cliprect,m_gfxdecode->gfx(3),
			code & 0xfff,
			color,clut,
			flipx,flipy,
			sx,sy,primap,pmask,15);
	}
}

u32 armedf_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int sprite_enable = m_vreg & 0x200;

	m_bg_tilemap->enable(m_vreg & 0x800);
	m_fg_tilemap->enable(m_vreg & 0x400);
	m_tx_tilemap->enable(m_vreg & 0x100);

	switch (m_scroll_type)
	{
		case 0: /* terra force, kozure ookami */
		case 2: /* legion */
		case 3: /* crazy climber 2 */
			m_fg_tilemap->set_scrollx(0, (m_fg_scrollx & 0x3ff));
			m_fg_tilemap->set_scrolly(0, (m_fg_scrolly & 0x3ff));
			break;

		case 1: /* armed formation */
			m_fg_tilemap->set_scrollx(0, m_fg_scrollx);
			m_fg_tilemap->set_scrolly(0, m_fg_scrolly);
			break;
	}

	screen.priority().fill(0, cliprect);
	bitmap.fill(0xff, cliprect);

	m_tx_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(1), 1);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 1);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 2);
	m_tx_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(0), 4);

	if (sprite_enable)
		draw_sprites(bitmap, cliprect, screen.priority());

	return 0;
}
