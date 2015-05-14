// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina, Pierpaolo Prazzoli
/***************************************************************************

  - BG layer 32x128 , 8x8 tiles 4bpp , 2 palettes  (2nd is black )
  - TXT layer 32x32 , 8x8 tiles 4bpp , 2 palettes (2nd is black)
  - Sprites 16x16 3bpp, 8 palettes (0-3 are black)

  'Special' effects :

  - spotlight - gfx(BG+Sprites) outside spotlight is using black pals
                spotlight masks are taken from ROM pr8
                simulated using bitmaps and custom clipping rect

  - lightning - BG color change (darkening ?) - simple analog circ.
                            simulated by additional palette

In debug build press 'w' for spotlight and 'e' for lightning

***************************************************************************/
#include "emu.h"
#include "includes/pitnrun.h"



TILE_GET_INFO_MEMBER(pitnrun_state::get_tile_info1)
{
	int code = m_videoram[tile_index];
	SET_TILE_INFO_MEMBER(0,
		code,
		0,
		0);
}

TILE_GET_INFO_MEMBER(pitnrun_state::get_tile_info2)
{
	int code = m_videoram2[tile_index];
	SET_TILE_INFO_MEMBER(1,
		code + (m_char_bank<<8),
		m_color_select&1,
		0);
}

WRITE8_MEMBER(pitnrun_state::videoram_w)
{
	m_videoram[offset] = data;
	m_fg ->mark_all_dirty();
}

WRITE8_MEMBER(pitnrun_state::videoram2_w)
{
	m_videoram2[offset] = data;
	m_bg ->mark_all_dirty();
}

WRITE8_MEMBER(pitnrun_state::char_bank_select)
{
	if(m_char_bank!=data)
	{
		m_bg ->mark_all_dirty();
		m_char_bank=data;
	}
}


WRITE8_MEMBER(pitnrun_state::scroll_w)
{
	m_scroll = (m_scroll & (0xff<<((offset)?0:8))) |( data<<((offset)?8:0));
	m_bg->set_scrollx(0, m_scroll);
}

WRITE8_MEMBER(pitnrun_state::ha_w)
{
	m_ha=data;
}

WRITE8_MEMBER(pitnrun_state::h_heed_w)
{
	m_h_heed=data;
}

WRITE8_MEMBER(pitnrun_state::v_heed_w)
{
	m_v_heed=data;
}

WRITE8_MEMBER(pitnrun_state::color_select_w)
{
	m_color_select=data;
	machine().tilemap().mark_all_dirty();
}

void pitnrun_state::spotlights()
{
	int x,y,i,b,datapix;
	UINT8 *ROM = memregion("user1")->base();
	for(i=0;i<4;i++)
		for(y=0;y<128;y++)
		for(x=0;x<16;x++)
		{
		datapix=ROM[128*16*i+x+y*16];
		for(b=0;b<8;b++)
		{
			m_tmp_bitmap[i]->pix16(y, x*8+(7-b)) = (datapix&1);
			datapix>>=1;
		}
		}
}


PALETTE_INIT_MEMBER(pitnrun_state, pitnrun)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;
	int bit0,bit1,bit2,r,g,b;
	for (i = 0;i < 32*3; i++)
	{
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i,rgb_t(r,g,b));
	}

	/* fake bg palette for lightning effect*/
	for(i=2*16;i<2*16+16;i++)
	{
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		r/=3;
		g/=3;
		b/=3;

		palette.set_pen_color(i+16,(r>0xff)?0xff:r,(g>0xff)?0xff:g,(b>0xff)?0xff:b);

	}
}

void pitnrun_state::video_start()
{
	m_fg = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(pitnrun_state::get_tile_info1),this),TILEMAP_SCAN_ROWS,8,8,32,32 );
	m_bg = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(pitnrun_state::get_tile_info2),this),TILEMAP_SCAN_ROWS,8,8,32*4,32 );
	m_fg->set_transparent_pen(0 );
	m_tmp_bitmap[0] = auto_bitmap_ind16_alloc(machine(),128,128);
	m_tmp_bitmap[1] = auto_bitmap_ind16_alloc(machine(),128,128);
	m_tmp_bitmap[2] = auto_bitmap_ind16_alloc(machine(),128,128);
	m_tmp_bitmap[3] = auto_bitmap_ind16_alloc(machine(),128,128);
	spotlights();

	save_item(NAME(m_h_heed));
	save_item(NAME(m_v_heed));
	save_item(NAME(m_ha));
	save_item(NAME(m_scroll));
	save_item(NAME(m_char_bank));
	save_item(NAME(m_color_select));
}

void pitnrun_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 *spriteram = m_spriteram;
	int sx, sy, flipx, flipy, offs,pal;

	for (offs = 0 ; offs < 0x100; offs+=4)
	{
		pal=spriteram[offs+2]&0x3;

		sy = 256-spriteram[offs+0]-16;
		sx = spriteram[offs+3];
		flipy = (spriteram[offs+1]&0x80)>>7;
		flipx = (spriteram[offs+1]&0x40)>>6;

		if (flip_screen_x())
		{
			sx = 256 - sx;
			flipx = !flipx;
		}
		if (flip_screen_y())
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
			(spriteram[offs+1]&0x3f)+((spriteram[offs+2]&0x80)>>1)+((spriteram[offs+2]&0x40)<<1),
			pal,
			flipx,flipy,
			sx,sy,0);
	}
}

UINT32 pitnrun_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int dx=0,dy=0;
	rectangle myclip=cliprect;

#ifdef MAME_DEBUG
	if (machine().input().code_pressed_once(KEYCODE_Q))
	{
		UINT8 *ROM = memregion("maincpu")->base();
		ROM[0x84f6]=0; /* lap 0 - normal */
	}

	if (machine().input().code_pressed_once(KEYCODE_W))
	{
		UINT8 *ROM = memregion("maincpu")->base();
		ROM[0x84f6]=6; /* lap 6 = spotlight */
	}

	if (machine().input().code_pressed_once(KEYCODE_E))
	{
		UINT8 *ROM = memregion("maincpu")->base();
		ROM[0x84f6]=2; /* lap 3 (trial 2)= lightnings */
		ROM[0x8102]=1;
	}
#endif

	bitmap.fill(0, cliprect);

	if(!(m_ha&4))
		m_bg->draw(screen, bitmap, cliprect, 0,0);
	else
	{
		dx=128-m_h_heed+((m_ha&8)<<5)+3;
		dy=128-m_v_heed+((m_ha&0x10)<<4);

		if (flip_screen_x())
			dx=128-dx+16;

		if (flip_screen_y())
			dy=128-dy;

		myclip.set(dx, dx+127, dy, dy+127);
		myclip &= cliprect;

		m_bg->draw(screen, bitmap, myclip, 0,0);
	}

	draw_sprites(bitmap,myclip);

	if(m_ha&4)
		copybitmap_trans(bitmap,*m_tmp_bitmap[m_ha&3],flip_screen_x(),flip_screen_y(),dx,dy,myclip, 1);
	m_fg->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}
