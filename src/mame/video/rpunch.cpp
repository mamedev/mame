// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

  video/rpunch.cpp

  Functions to emulate the video hardware of the machine.

****************************************************************************/

#include "emu.h"
#include "includes/rpunch.h"


/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(rpunch_state::get_bg0_tile_info)
{
	const u16 data = m_videoram[tile_index];
	int code;
	if (m_videoflags & 0x0400)  code = (data & 0x0fff) | 0x2000;
	else                        code = (data & 0x1fff);

	tileinfo.set(0,
			code,
			((m_videoflags & 0x0010) >> 1) | ((data >> 13) & 7),
			0);
}

TILE_GET_INFO_MEMBER(rpunch_state::get_bg1_tile_info)
{
	const u16 data = m_videoram[0x1000 | tile_index];
	int code;
	if (m_videoflags & 0x0800)  code = (data & 0x0fff) | 0x2000;
	else                        code = (data & 0x1fff);

	tileinfo.set(1,
			code,
			((m_videoflags & 0x0020) >> 2) | ((data >> 13) & 7),
			0);
}


/*************************************
 *
 *  Video system start
 *
 *************************************/

TIMER_CALLBACK_MEMBER(rpunch_state::crtc_interrupt_gen)
{
	m_maincpu->set_input_line(1, HOLD_LINE);
	if (param != 0)
		m_crtc_timer->adjust(m_screen->frame_period() / param, 0, m_screen->frame_period() / param);
}


VIDEO_START_MEMBER(rpunch_state,rpunch)
{
	m_videoflags = 0;
	m_sprite_xoffs = 0;

	/* allocate tilemaps for the backgrounds */
	m_background[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(rpunch_state::get_bg0_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 64, 64);
	m_background[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(rpunch_state::get_bg1_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 64, 64);

	/* configure the tilemaps */
	m_background[1]->set_transparent_pen(15);

	m_pixmap = std::make_unique<bitmap_ind16>(512, 256);

	const rectangle pixmap_rect(0,511,0,255);
	m_pixmap->fill(0xf, pixmap_rect);

	/* reset the timer */
	m_crtc_timer = timer_alloc(FUNC(rpunch_state::crtc_interrupt_gen), this);

	save_item(NAME(*m_pixmap));
}


VIDEO_START_MEMBER(rpunch_state,svolley)
{
	VIDEO_START_CALL_MEMBER(rpunch);
	m_background[0]->set_scrolldx(8, 0); // aligns middle net sprite with bg as shown in reference
	m_sprite_xoffs = -4;
}


/*************************************
 *
 *  Write handlers
 *
 *************************************/

u8 rpunch_state::pixmap_r(offs_t offset)
{
	const int sy = offset >> 8;
	const int sx = (offset & 0xff) << 1;

	return ((m_pixmap->pix(sy & 0xff, sx & ~1) & 0xf) << 4) | (m_pixmap->pix(sy & 0xff, sx |  1) & 0xf);
}

void rpunch_state::pixmap_w(offs_t offset, u8 data)
{
	const int sy = offset >> 8;
	const int sx = (offset & 0xff) << 1;

	m_pixmap->pix(sy & 0xff, sx & ~1) = ((data & 0xf0) >> 4);
	m_pixmap->pix(sy & 0xff, sx |  1) = (data & 0x0f);
}

void rpunch_state::videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	const int tmap = offset >> 12;
	const int tile_index = offset & 0xfff;
	COMBINE_DATA(&m_videoram[offset]);
	m_background[tmap]->mark_tile_dirty(tile_index);
}


void rpunch_state::videoreg_w(offs_t offset, u16 data, u16 mem_mask)
{
	const int oldword = m_videoflags;
	COMBINE_DATA(&m_videoflags);

	if (m_videoflags != oldword)
	{
		/* invalidate tilemaps */
		if ((oldword ^ m_videoflags) & 0x0410)
			m_background[0]->mark_all_dirty();
		if ((oldword ^ m_videoflags) & 0x0820)
			m_background[1]->mark_all_dirty();
	}
}


void rpunch_state::scrollreg_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7 && ACCESSING_BITS_8_15)
		switch (offset)
		{
			case 0:
				m_background[0]->set_scrolly(0, data & 0x1ff);
				break;

			case 1:
				m_background[0]->set_scrollx(0, data & 0x1ff);
				break;

			case 2:
				m_background[1]->set_scrolly(0, data & 0x1ff);
				break;

			case 3:
				m_background[1]->set_scrollx(0, data & 0x1ff);
				break;
		}
}


void rpunch_state::gga_w(offs_t offset, u8 data)
{
	m_gga->write(offset >> 5, data & 0xff);
}


void rpunch_state::gga_data_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		/* only register we know about.... */
		case 0x0b:
			m_crtc_timer->adjust(m_screen->time_until_vblank_start(), (data == 0xc0) ? 2 : 1);
			break;

		default:
			logerror("CRTC register %02X = %02X\n", offset, data);
			break;
	}
}


void rpunch_state::sprite_ctrl_w(offs_t offset, u8 data)
{
	if (offset == 0)
	{
		m_sprite_num = data & 0x3f;
		logerror("Number of sprites = %02X\n", data & 0x3f);
	}
	else
	{
		m_sprite_pri = data & 0x3f;
		logerror("Sprite priority point = %02X\n", data & 0x3f);
	}
}


/*************************************
 *
 *  Sprite routines
 *
 *************************************/

void rpunch_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* draw the sprites */
	int offs = m_sprite_num;
	while (offs > 0)
	{
		offs--;
		const u32 ram = offs << 2;
		u32 pri = 0; // above everything except direct mapped bitmap
		/* this seems like the most plausible explanation */
		if (offs < m_sprite_pri)
			pri |= GFX_PMASK_2; // behind foreground

		const u16 data1 = m_spriteram[ram + 1];
		const u32 code = data1 & 0x7ff;

		const u16 data0 = m_spriteram[ram + 0];
		const u16 data2 = m_spriteram[ram + 2];
		int x = (data2 & 0x1ff) + 8;
		int y = 513 - (data0 & 0x1ff);
		const bool xflip = data1 & 0x1000;
		const bool yflip = data1 & 0x0800;
		const u32 color = ((data1 >> 13) & 7) | ((m_videoflags & 0x0040) >> 3);

		if (x > cliprect.max_x) x -= 512;
		if (y > cliprect.max_y) y -= 512;

		m_gfxdecode->gfx(2)->prio_transpen(bitmap,cliprect,
				code, color + (m_sprite_palette / 16), xflip, yflip, x + m_sprite_xoffs, y,
				m_screen->priority(), pri, 15);
	}
}


/*************************************
 *
 *  Bitmap routines
 *
 *************************************/

void rpunch_state::draw_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const u32 colourbase = 512 + ((m_videoflags & 0x000f) << 4);

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		u16 const *const src = &m_pixmap->pix(y & 0xff);
		u16 *const dst = &bitmap.pix(y);
		for(int x = cliprect.min_x / 4; x <= cliprect.max_x; x++)
		{
			const u16 pix = src[(x + 4) & 0x1ff];
			if ((pix & 0xf) != 0xf)
				dst[x] = colourbase + pix;
		}
	}
}


/*************************************
 *
 *  Main screen refresh
 *
 *************************************/

u32 rpunch_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	m_background[0]->draw(screen, bitmap, cliprect, 0,1);
	m_background[1]->draw(screen, bitmap, cliprect, 0,2);
	draw_sprites(bitmap, cliprect);
	draw_bitmap(bitmap, cliprect);
	return 0;
}
