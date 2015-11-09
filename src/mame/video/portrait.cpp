// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff, Pierpaolo Prazzoli
/***************************************************************************

  Portraits
  video hardware emulation

***************************************************************************/

#include "emu.h"
#include "includes/portrait.h"


WRITE8_MEMBER(portrait_state::bgvideo_write)
{
	m_background->mark_tile_dirty(offset/2);
	m_bgvideoram[offset] = data;
}

WRITE8_MEMBER(portrait_state::fgvideo_write)
{
	m_foreground->mark_tile_dirty(offset/2);
	m_fgvideoram[offset] = data;
}

inline void portrait_state::get_tile_info( tile_data &tileinfo, int tile_index, const UINT8 *source )
{
	int attr    = source[tile_index*2+0];
	int tilenum = source[tile_index*2+1];
	int flags   = 0;
	int color   = 0;

	/* or 0x10 ? */
	if( attr & 0x20 ) flags = TILE_FLIPY;

	switch( attr & 7 )
	{
		case 1:
			tilenum += 0x200;
			break;
		case 3:
			tilenum += 0x300;
			break;
		case 5:
			tilenum += 0x100;
			break;
	}

	if (tilenum<0x100)
		color = ((tilenum&0xff)>>1)+0x00;
	else
		color = ((tilenum&0xff)>>1)+0x80;

	SET_TILE_INFO_MEMBER(0, tilenum, color, flags );
}

TILE_GET_INFO_MEMBER(portrait_state::get_bg_tile_info)
{
	get_tile_info(tileinfo, tile_index, m_bgvideoram );
}

TILE_GET_INFO_MEMBER(portrait_state::get_fg_tile_info)
{
	get_tile_info(tileinfo, tile_index, m_fgvideoram );
}

void portrait_state::video_start()
{
	m_background = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(portrait_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32 );
	m_foreground = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(portrait_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32 );

	m_foreground->set_transparent_pen(7);

	save_item(NAME(m_scroll));
}



PALETTE_INIT_MEMBER(portrait_state, portrait)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;
	UINT8* lookup = memregion("tileattr")->base();

/*
    for (i = 0;i < 0x40;i++)
    {
        int r,g,b,data;
        data = color_prom[0];


        r = (data >> 0) & 0x7;
        g = (data >> 3) & 0x3;
        b = (data >> 5) & 0x7;

        palette.set_indirect_color(i, rgb_t(pal3bit(r), pal2bit(g), pal3bit(b)));

        color_prom++;
    }
*/

	for (i=0;i<0x20;i++)
	{
		int r,g,b,data;
		data = (color_prom[0]<<0) | (color_prom[0x20]<<8);

		r = (data >> 0) & 0x1f;
		g = (data >> 5) & 0x1f;
		b = (data >> 10) & 0x1f;

		palette.set_indirect_color(i, rgb_t(pal5bit(r), pal5bit(g), pal5bit(b)));

		// ?? the lookup seems to reference 0x3f colours, unless 1 bit is priority or similar?
		palette.set_indirect_color(i+0x20, rgb_t(pal5bit(r>>1), pal5bit(g>>1), pal5bit(b>>1)));

		color_prom++;
	}



	for (i = 0;i < 0x800;i++)
	{
		UINT8 ctabentry = lookup[i]&0x3f;
		palette.set_pen_indirect(i, ctabentry);
	}
}


void portrait_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *source = m_spriteram;
	UINT8 *finish = source + 0x200;

	while( source < finish )
	{
		int sy      = source[0];
		int sx      = source[1];
		int attr    = source[2];
			/* xx-x---- ?
			 * --x----- flipy
			 * ----x--- msb source[0]
			 * -----x-- msb source[1]
			 */
		int tilenum = source[3];

		int color = ((tilenum&0xff)>>1)+0x00;

		int fy = attr & 0x20;

		if(attr & 0x04) sx |= 0x100;

		if(attr & 0x08) sy |= 0x100;

		sx += (source - m_spriteram) - 8;
		sx &= 0x1ff;

		sy = (512 - 64) - sy;

		/* wrong! */
		switch( attr & 0xc0 )
		{
		case 0:
			break;

		case 0x40:
			sy -= m_scroll;
			break;

		case 0x80:
			sy -= m_scroll;
			break;

		case 0xc0:
			break;

		}

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				tilenum,color,
				0,fy,
				sx,sy,7);

		source += 0x10;
	}
}

UINT32 portrait_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle cliprect_scroll, cliprect_no_scroll;

	cliprect_scroll = cliprect_no_scroll = cliprect;

	cliprect_no_scroll.min_x = cliprect_no_scroll.max_x - 111;
	cliprect_scroll.max_x    = cliprect_scroll.min_x    + 319;

	m_background->set_scrolly(0, 0);
	m_foreground->set_scrolly(0, 0);
	m_background->draw(screen, bitmap, cliprect_no_scroll, 0, 0);
	m_foreground->draw(screen, bitmap, cliprect_no_scroll, 0, 0);

	m_background->set_scrolly(0, m_scroll);
	m_foreground->set_scrolly(0, m_scroll);
	m_background->draw(screen, bitmap, cliprect_scroll, 0, 0);
	m_foreground->draw(screen, bitmap, cliprect_scroll, 0, 0);

	draw_sprites(bitmap,cliprect);
	return 0;
}
