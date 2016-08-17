// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

  video/rpunch.c

  Functions to emulate the video hardware of the machine.

****************************************************************************/

#include "emu.h"
#include "includes/rpunch.h"


#define BITMAP_WIDTH    304
#define BITMAP_HEIGHT   224
#define BITMAP_XOFFSET  4


/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(rpunch_state::get_bg0_tile_info)
{
	UINT16 *videoram = m_videoram;
	int data = videoram[tile_index];
	int code;
	if (m_videoflags & 0x0400)  code = (data & 0x0fff) | 0x2000;
	else                        code = (data & 0x1fff);

	SET_TILE_INFO_MEMBER(0,
			code,
			((m_videoflags & 0x0010) >> 1) | ((data >> 13) & 7),
			0);
}

TILE_GET_INFO_MEMBER(rpunch_state::get_bg1_tile_info)
{
	UINT16 *videoram = m_videoram;
	int data = videoram[0x2000 / 2 + tile_index];
	int code;
	if (m_videoflags & 0x0800)  code = (data & 0x0fff) | 0x2000;
	else                        code = (data & 0x1fff);

	SET_TILE_INFO_MEMBER(1,
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
	m_sprite_xoffs = 0;

	/* allocate tilemaps for the backgrounds */
	m_background[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(rpunch_state::get_bg0_tile_info),this),TILEMAP_SCAN_COLS,8,8,64,64);
	m_background[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(rpunch_state::get_bg1_tile_info),this),TILEMAP_SCAN_COLS,8,8,64,64);

	/* configure the tilemaps */
	m_background[1]->set_transparent_pen(15);

	if (m_bitmapram)
		memset(m_bitmapram, 0xff, m_bitmapram.bytes());

	/* reset the timer */
	m_crtc_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(rpunch_state::crtc_interrupt_gen),this));
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

WRITE16_MEMBER(rpunch_state::rpunch_videoram_w)
{
	UINT16 *videoram = m_videoram;
	int tmap = offset >> 12;
	int tile_index = offset & 0xfff;
	COMBINE_DATA(&videoram[offset]);
	m_background[tmap]->mark_tile_dirty(tile_index);
}


WRITE16_MEMBER(rpunch_state::rpunch_videoreg_w)
{
	int oldword = m_videoflags;
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


WRITE16_MEMBER(rpunch_state::rpunch_scrollreg_w)
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


WRITE16_MEMBER(rpunch_state::rpunch_crtc_data_w)
{
	if (ACCESSING_BITS_0_7)
	{
		data &= 0xff;
		switch (m_crtc_register)
		{
			/* only register we know about.... */
			case 0x0b:
				m_crtc_timer->adjust(m_screen->time_until_vblank_start(), (data == 0xc0) ? 2 : 1);
				break;

			default:
				logerror("CRTC register %02X = %02X\n", m_crtc_register, data & 0xff);
				break;
		}
	}
}


WRITE16_MEMBER(rpunch_state::rpunch_crtc_register_w)
{
	if (ACCESSING_BITS_0_7)
		m_crtc_register = data & 0xff;
}


WRITE16_MEMBER(rpunch_state::rpunch_ins_w)
{
	if (ACCESSING_BITS_0_7)
	{
		if (offset == 0)
		{
			m_gins = data & 0x3f;
			logerror("GINS = %02X\n", data & 0x3f);
		}
		else
		{
			m_bins = data & 0x3f;
			logerror("BINS = %02X\n", data & 0x3f);
		}
	}
}


/*************************************
 *
 *  Sprite routines
 *
 *************************************/

void rpunch_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int start, int stop)
{
	UINT16 *spriteram16 = m_spriteram;
	int offs;

	start *= 4;
	stop *= 4;

	/* draw the sprites */
	for (offs = start; offs < stop; offs += 4)
	{
		int data1 = spriteram16[offs + 1];
		int code = data1 & 0x7ff;

		int data0 = spriteram16[offs + 0];
		int data2 = spriteram16[offs + 2];
		int x = (data2 & 0x1ff) + 8;
		int y = 513 - (data0 & 0x1ff);
		int xflip = data1 & 0x1000;
		int yflip = data1 & 0x0800;
		int color = ((data1 >> 13) & 7) | ((m_videoflags & 0x0040) >> 3);

		if (x >= BITMAP_WIDTH) x -= 512;
		if (y >= BITMAP_HEIGHT) y -= 512;

		m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
				code, color + (m_sprite_palette / 16), xflip, yflip, x+m_sprite_xoffs, y, 15);
	}
}


/*************************************
 *
 *  Bitmap routines
 *
 *************************************/

void rpunch_state::draw_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int colourbase;
	int xxx=512/4;
	int yyy=256;
	int x,y,count;

	colourbase = 512 + ((m_videoflags & 15) * 16);

	count = 0;

	for (y=0;y<yyy;y++)
	{
		for(x=0;x<xxx;x++)
		{
			int coldat;
			coldat = (m_bitmapram[count]>>12)&0xf; if (coldat!=15) bitmap.pix16(y, ((x*4+0)-4)&0x1ff) = coldat+colourbase;
			coldat = (m_bitmapram[count]>>8 )&0xf; if (coldat!=15) bitmap.pix16(y, ((x*4+1)-4)&0x1ff) = coldat+colourbase;
			coldat = (m_bitmapram[count]>>4 )&0xf; if (coldat!=15) bitmap.pix16(y, ((x*4+2)-4)&0x1ff) = coldat+colourbase;
			coldat = (m_bitmapram[count]>>0 )&0xf; if (coldat!=15) bitmap.pix16(y, ((x*4+3)-4)&0x1ff) = coldat+colourbase;
			count++;
		}
	}
}


/*************************************
 *
 *  Main screen refresh
 *
 *************************************/

UINT32 rpunch_state::screen_update_rpunch(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int effbins;

	/* this seems like the most plausible explanation */
	effbins = (m_bins > m_gins) ? m_gins : m_bins;

	m_background[0]->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap, cliprect, 0, effbins);
	m_background[1]->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap, cliprect, effbins, m_gins);
	if (m_bitmapram)
		draw_bitmap(bitmap, cliprect);
	return 0;
}
