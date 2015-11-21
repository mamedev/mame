// license:BSD-3-Clause
// copyright-holders:David Haywood,Paul Priest
/* video/macrossp.c */

#include "emu.h"
#include "includes/macrossp.h"

//#define DEBUG_KEYS 1

/*
Sprite list is drawn backwards, and priorities with backgrounds are not transitive

=== Vid Registers ===
[0] - tiles
0x000003ff - global scrollx
0x00000c00 - color mode
0x0000c000 - priority
0x03ff0000 - global scrolly
0x90000000 - enable? Always 0x9

[1] - ???
0xffff0000 - another scrolly register, mainly used when zooming. unused by emulation
0x0000ffff - another scrollx register, mainly used when zooming. unused by emulation

[2] - zoom params
0xf0000000 - zoom enable (== 0xe, not == 0x2). Presumably one bit for x and y enable
0x01ff0000 - incy (0x40 is 1:1, incx is in lineram. might be more bits)

Interesting test cases (macrossp, quizmoon doesn't use tilemap zoom):
1) Title screen logo zoom
2) Second level, as zoom into end of canyon
3) Second level, as doors open to revels tracks/blue background for boss
4) Boss should go under bridge on level 4 when he first appears

*/

/*** SCR A LAYER ***/

WRITE32_MEMBER(macrossp_state::macrossp_scra_videoram_w)
{
	COMBINE_DATA(&m_scra_videoram[offset]);

	m_scra_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(macrossp_state::get_macrossp_scra_tile_info)
{
	UINT32 attr, tileno, color;

	attr = m_scra_videoram[tile_index];
	tileno = attr & 0x0000ffff;

	switch (m_scra_videoregs[0] & 0x00000c00)
	{
		case 0x00000800:
			color = (attr & 0x000e0000) >> 15;
			break;

		case 0x00000400:
			color = (attr & 0x003e0000) >> 17;
			break;

		default:
			color = machine().rand() & 7;
			break;
	}

	SET_TILE_INFO_MEMBER(1, tileno, color, TILE_FLIPYX((attr & 0xc0000000) >> 30));
}

/*** SCR B LAYER ***/

WRITE32_MEMBER(macrossp_state::macrossp_scrb_videoram_w)
{
	COMBINE_DATA(&m_scrb_videoram[offset]);

	m_scrb_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(macrossp_state::get_macrossp_scrb_tile_info)
{
	UINT32 attr, tileno, color;

	attr = m_scrb_videoram[tile_index];
	tileno = attr & 0x0000ffff;

	switch (m_scrb_videoregs[0] & 0x00000c00)
	{
		case 0x00000800:
			color = (attr & 0x000e0000) >> 15;
			break;

		case 0x00000400:
			color = (attr & 0x003e0000) >> 17;
			break;

		default:
			color = machine().rand() & 7;
			break;
	}

	SET_TILE_INFO_MEMBER(2, tileno, color, TILE_FLIPYX((attr & 0xc0000000) >> 30));
}

/*** SCR C LAYER ***/

WRITE32_MEMBER(macrossp_state::macrossp_scrc_videoram_w)
{
	COMBINE_DATA(&m_scrc_videoram[offset]);

	m_scrc_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(macrossp_state::get_macrossp_scrc_tile_info)
{
	UINT32 attr, tileno, color;

	attr = m_scrc_videoram[tile_index];
	tileno = attr & 0x0000ffff;

	switch (m_scrc_videoregs[0] & 0x00000c00)
	{
		case 0x00000800:
			color = (attr & 0x000e0000) >> 15;
			break;

		case 0x00000400:
			color = (attr & 0x003e0000) >> 17;
			break;

		default:
			color = machine().rand() & 7;
			break;
	}

	SET_TILE_INFO_MEMBER(3, tileno, color, TILE_FLIPYX((attr & 0xc0000000) >> 30));
}

/*** TEXT LAYER ***/

WRITE32_MEMBER(macrossp_state::macrossp_text_videoram_w)
{
	COMBINE_DATA(&m_text_videoram[offset]);

	m_text_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(macrossp_state::get_macrossp_text_tile_info)
{
	UINT32 tileno, colour;

	tileno = m_text_videoram[tile_index] & 0x0000ffff;
	colour = (m_text_videoram[tile_index] & 0x00fe0000) >> 17;

	SET_TILE_INFO_MEMBER(4, tileno, colour, 0);
}



/*** VIDEO START / UPDATE ***/

void macrossp_state::video_start()
{
	m_spriteram_old = auto_alloc_array_clear(machine(), UINT32, m_spriteram.bytes() / 4);
	m_spriteram_old2 = auto_alloc_array_clear(machine(), UINT32, m_spriteram.bytes() / 4);

	m_text_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(macrossp_state::get_macrossp_text_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_scra_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(macrossp_state::get_macrossp_scra_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_scrb_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(macrossp_state::get_macrossp_scrb_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_scrc_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(macrossp_state::get_macrossp_scrc_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);

	m_text_tilemap->set_transparent_pen(0);
	m_scra_tilemap->set_transparent_pen(0);
	m_scrb_tilemap->set_transparent_pen(0);
	m_scrc_tilemap->set_transparent_pen(0);

	m_gfxdecode->gfx(0)->set_granularity(64);
	m_gfxdecode->gfx(1)->set_granularity(64);
	m_gfxdecode->gfx(2)->set_granularity(64);
	m_gfxdecode->gfx(3)->set_granularity(64);

	save_pointer(NAME(m_spriteram_old), m_spriteram.bytes() / 4);
	save_pointer(NAME(m_spriteram_old2), m_spriteram.bytes() / 4);
}



void macrossp_state::draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	//  UINT32 *source = m_spriteram;
	UINT32 *source = (m_spriteram_old2 + m_spriteram.bytes() / 4) - 3; /* buffers by two frames */
	UINT32 *finish = m_spriteram_old2;

	/* reverse order */
	while (source >= finish)
	{
		/*

		 --hh hhyy yyyy yyyy   CCww wwxx xxxx xxxx

		 ---- --zz zzzz zzzz   ---- --ZZ ZZZZ ZZZZ

		 fFa- pp-- cccc c---   tttt tttt tttt tttt

		 */


		int wide = (source[0] & 0x00003c00) >> 10;
		int high = (source[0] & 0x3c000000) >> 26;

		int xpos = (source[0] & 0x000003ff) >> 0;
		int ypos = (source[0] & 0x03ff0000) >> 16;

		int xzoom = (source[1] & 0x000003ff) >> 0; /* 0x100 is zoom factor of 1.0 */
		int yzoom = (source[1] & 0x03ff0000) >> 16;

		int col;
		int tileno = (source[2] & 0x0000ffff) >> 0;

		int flipx = (source[2] & 0x40000000) >> 30;
		int flipy = (source[2] & 0x80000000) >> 31;

		int alpha = (source[2] & 0x20000000)?0x80:0xff; /* alpha blending enable? */

		int loopno = 0;

		int xcnt, ycnt;
		int xoffset, yoffset;

		int pri = (source[2] & 0x0c000000) >> 26;
		int primask = 0;
		if(pri <= 0) primask |= GFX_PMASK_1;
		if(pri <= 1) primask |= GFX_PMASK_2;
		if(pri <= 2) primask |= GFX_PMASK_4;
		if(pri <= 3) primask |= GFX_PMASK_8;

		switch (source[0] & 0x0000c000)
		{
			case 0x00008000:
				col = (source[2] & 0x00380000) >> 17;
				break;

			case 0x00004000:
				col = (source[2] & 0x00f80000) >> 19;
				break;

			default:
				col = machine().rand();
				break;
		}

		if (xpos > 0x1ff) xpos -=0x400;
		if (ypos > 0x1ff) ypos -=0x400;

		/* loop params */
		int ymin = 0;
		int ymax = high+1;
		int yinc = 1;
		int yoffst = 0;
		if(flipy) {
			yoffst = (high * yzoom * 16);
			ymin = high;
			ymax = -1;
			yinc = -1;
		}

		int xmin = 0;
		int xmax = wide+1;
		int xinc = 1;
		int xoffst = 0;
		if(flipx) {
			xoffst = (wide * xzoom * 16);
			xmin = wide;
			xmax = -1;
			xinc = -1;
		}

		yoffset = yoffst;
		for (ycnt = ymin; ycnt != ymax; ycnt += yinc)
		{
			xoffset = xoffst;
			for (xcnt = xmin; xcnt != xmax; xcnt += xinc)
			{
				int fudged_xzoom = xzoom<<8;
				int fudged_yzoom = yzoom<<8;

				/* cover seams as don't know exactly how many pixels on target will cover, and can't specify fractional offsets to start */
				if(xzoom < 0x100) fudged_xzoom += 0x600;
				if(yzoom < 0x100) fudged_yzoom += 0x600;

				gfx->prio_zoom_alpha(bitmap,cliprect,tileno+loopno,col,
										flipx,flipy,xpos+(xoffset>>8),ypos+(yoffset>>8),
										fudged_xzoom,fudged_yzoom,
										screen.priority(),primask,0,alpha);

				xoffset += ((xzoom*16) * xinc);
				loopno++;
			}
			yoffset += ((yzoom*16) * yinc);
		}

		source -= 3;
	}
}


void macrossp_state::draw_layer( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int layer, int line, int pri )
{
	tilemap_t *tm;
	UINT32 *vr;
	UINT32 *lr;

	switch (layer)
	{
		case 0:
		default:
			tm = m_scra_tilemap;
			vr = m_scra_videoregs;
			lr = m_scra_linezoom;
			break;

		case 1:
			tm = m_scrb_tilemap;
			vr = m_scrb_videoregs;
			lr = m_scrb_linezoom;
			break;

		case 2:
			tm = m_scrc_tilemap;
			vr = m_scrc_videoregs;
			lr = m_scrc_linezoom;
			break;
	}

	if ((vr[2] & 0xf0000000) == 0xe0000000) /* zoom enable (guess, surely wrong) */
	{
		int startx=0, starty=0, incy, incx;

		startx = ((vr[0] & 0x000003ff) << 16 );
		starty = ((vr[0] & 0x03ff0000) >> 0);
		incy   = (vr[2] & 0x01ff0000) >> 6;

		if (line&1)
			incx = (lr[line/2] & 0x0000ffff)>>0;
		else
			incx = (lr[line/2] & 0xffff0000)>>16;

		incx <<= 10;

		/* scroll register contain position relative to the center of the screen, so adjust */
		startx -= (368/2) * (incx - 0x10000);
		starty -= (240/2) * (incy - 0x10000);

// previous logic, which gives mostly comparable results, vr[1] is now unused
//      startx = (vr[1] & 0x0000ffff) << 16;
//      starty = (vr[1] & 0xffff0000) >> 0;
//      startx -= (368/2) * incx;
//      starty -= (240/2) * incy;

		tm->draw_roz(screen, bitmap, cliprect,
				startx,starty,incx,0,0,incy,
				1,  /* wraparound */
				0, 1<<pri);
	}
	else
	{
		tm->set_scrollx(0, ((vr[0] & 0x000003ff) >> 0 ) );
		tm->set_scrolly(0, ((vr[0] & 0x03ff0000) >> 16) );
		tm->draw(screen, bitmap, cliprect, 0, 1<<pri);
	}
}

UINT32 macrossp_state::screen_update_macrossp(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int layerpri[3];
	int sprites = true;
	int backgrounds = true;

	rectangle clip;
	const rectangle &visarea = screen.visible_area();
	clip = visarea;

	/* 0 <= layerpri <= 2 */
	layerpri[0] = (m_scra_videoregs[0] & 0x0000c000) >> 14;
	layerpri[1] = (m_scrb_videoregs[0] & 0x0000c000) >> 14;
	layerpri[2] = (m_scrc_videoregs[0] & 0x0000c000) >> 14;

	screen.priority().fill(0, cliprect);
	bitmap.fill(m_palette->black_pen(), cliprect);

#ifdef DEBUG_KEYS
	const input_code lay_keys[8] = {KEYCODE_Q, KEYCODE_W, KEYCODE_E, KEYCODE_R, KEYCODE_T};
	bool lay_debug = false;
	for (int pri = 0; pri <= 4; pri++)
	{
		if(machine().input().code_pressed(lay_keys[pri])) {
			lay_debug = true;
		}
	}
	if(machine().input().code_pressed(KEYCODE_G)) {
		sprites = false;
	}
	if(machine().input().code_pressed(KEYCODE_H)) {
		backgrounds = false;
	}
#endif

	for(int pri = 0; pri <= 3; pri++) {
#ifdef DEBUG_KEYS
		if(lay_debug && !machine().input().code_pressed(lay_keys[pri]))
			continue;
#endif

		if(!backgrounds)
			continue;

		for (int y=0; y<240; y++) {
			clip.min_y = clip.max_y = y;

			/* quizmoon map requires that layer 2 be drawn over layer 3 when same pri */
			for(int layer = 2; layer >= 0; layer--) {
				if(layerpri[layer] == pri) {
					draw_layer(screen, bitmap, clip, layer, y, pri);
				}
			}
		}

	}

#ifdef DEBUG_KEYS
	if(!lay_debug && !machine().input().code_pressed(lay_keys[4]))
#endif
		m_text_tilemap->draw(screen, bitmap, cliprect, 0, 8);

	if(sprites) draw_sprites(screen, bitmap, cliprect);


#if 0
popmessage  ("scra - %08x %08x %08x\nscrb - %08x %08x %08x\nscrc - %08x %08x %08x",
m_scra_videoregs[0]&0xffffffff, // yyyyxxxx
m_scra_videoregs[1], // ??? more scrolling?
m_scra_videoregs[2], // 08 - 0b

m_scrb_videoregs[0]&0xffffffff, // 00 - 03
m_scrb_videoregs[1], // 04 - 07
m_scrb_videoregs[2], // 08 - 0b

m_scrc_videoregs[0]&0xffffffff, // 00 - 03
m_scrc_videoregs[1], // 04 - 07
m_scrc_videoregs[2]);// 08 - 0b
#endif
	return 0;
}

void macrossp_state::screen_eof_macrossp(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		/* looks like sprites are *two* frames ahead, like nmk16 */
		memcpy(m_spriteram_old2, m_spriteram_old, m_spriteram.bytes());
		memcpy(m_spriteram_old, m_spriteram, m_spriteram.bytes());
	}
}
