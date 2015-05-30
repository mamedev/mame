// license:BSD-3-Clause
// copyright-holders:David Haywood
/* video/macrossp.c */

#include "emu.h"
#include "includes/macrossp.h"


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



void macrossp_state::draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, int priority )
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	//  UINT32 *source = m_spriteram;
	UINT32 *source = m_spriteram_old2; /* buffers by two frames */
	UINT32 *finish = source + m_spriteram.bytes() / 4;

	while (source < finish)
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

		if (pri == priority)
		{
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

			if (!flipx)
			{
				if (!flipy)
				{
					/* noxflip, noyflip */
					yoffset = 0; /* I'm doing this so rounding errors are cumulative, still looks a touch crappy when multiple sprites used together */
					for (ycnt = 0; ycnt <= high; ycnt++)
					{
						xoffset = 0;
						for (xcnt = 0; xcnt <= wide; xcnt++)
						{
							gfx->zoom_alpha(bitmap,cliprect,tileno+loopno,col,flipx,flipy,xpos+xoffset,ypos+yoffset,xzoom*0x100,yzoom*0x100,0,alpha);

							xoffset += ((xzoom*16 + (1<<7)) >> 8);
							loopno++;
						}
						yoffset += ((yzoom*16 + (1<<7)) >> 8);
					}
				}
				else
				{
					/* noxflip, flipy */
					yoffset = ((high * yzoom * 16) >> 8);
					for (ycnt = high; ycnt >= 0; ycnt--)
					{
						xoffset = 0;
						for (xcnt = 0; xcnt <= wide; xcnt++)
						{
							gfx->zoom_alpha(bitmap,cliprect,tileno+loopno,col,flipx,flipy,xpos+xoffset,ypos+yoffset,xzoom*0x100,yzoom*0x100,0,alpha);

							xoffset += ((xzoom * 16 + (1 << 7)) >> 8);
							loopno++;
						}
						yoffset -= ((yzoom * 16 + (1 << 7)) >> 8);
					}
				}
			}
			else
			{
				if (!flipy)
				{
					/* xflip, noyflip */
					yoffset = 0;
					for (ycnt = 0; ycnt <= high; ycnt++)
					{
						xoffset = ((wide*xzoom*16) >> 8);
						for (xcnt = wide; xcnt >= 0; xcnt--)
						{
							gfx->zoom_alpha(bitmap,cliprect,tileno+loopno,col,flipx,flipy,xpos+xoffset,ypos+yoffset,xzoom*0x100,yzoom*0x100,0,alpha);

							xoffset -= ((xzoom * 16 + (1 << 7)) >> 8);
							loopno++;
						}
						yoffset += ((yzoom * 16 + (1 << 7)) >> 8);
					}
				}
				else
				{
					/* xflip, yflip */
					yoffset = ((high * yzoom * 16) >> 8);
					for (ycnt = high; ycnt >= 0; ycnt--)
					{
						xoffset = ((wide * xzoom * 16) >> 8);
						for (xcnt = wide; xcnt >=0 ; xcnt--)
						{
							gfx->zoom_alpha(bitmap,cliprect,tileno+loopno,col,flipx,flipy,xpos+xoffset,ypos+yoffset,xzoom*0x100,yzoom*0x100,0,alpha);

							xoffset -= ((xzoom * 16 + (1 << 7)) >> 8);
							loopno++;
						}
						yoffset -= ((yzoom * 16 + (1 << 7)) >> 8);
					}
				}
			}
		}
		source += 3;
	}
}


void macrossp_state::draw_layer( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int layer, int line )
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
		int startx, starty, inc, linc;

		startx = (vr[1] & 0x0000ffff) << 16;
		starty = (vr[1] & 0xffff0000) >> 0;
		inc = (vr[2] & 0x00ff0000) >> 6;

		if (line&1)
			linc = (lr[line/2] & 0x0000ffff)>>0;
		else
			linc = (lr[line/2] & 0xffff0000)>>16;


		linc <<= 10;

		/* WRONG! */
		/* scroll register contain position relative to the center of the screen, so adjust */
		startx -= (368/2) * linc;
		starty -= (240/2) * inc;

		tm->draw_roz(screen, bitmap, cliprect,
				startx,starty,linc,0,0,inc,
				1,  /* wraparound */
				0,0);
	}
	else
	{
		tm->set_scrollx(0, ((vr[0] & 0x000003ff) >> 0 ) );
		tm->set_scrolly(0, ((vr[0] & 0x03ff0000) >> 16) );
		tm->draw(screen, bitmap, cliprect, 0, 0);
	}
}

/* useful function to sort the three tile layers by priority order */
void macrossp_state::sortlayers(int *layer,int *pri)
{
#define SWAP(a,b) \
	if (pri[a] >= pri[b]) \
	{ \
		int t; \
		t = pri[a]; pri[a] = pri[b]; pri[b] = t; \
		t = layer[a]; layer[a] = layer[b]; layer[b] = t; \
	}

	SWAP(0,1)
	SWAP(0,2)
	SWAP(1,2)
}

UINT32 macrossp_state::screen_update_macrossp(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int layers[3],layerpri[3];

	bitmap.fill(m_palette->black_pen(), cliprect);

	layers[0] = 0;
	layerpri[0] = (m_scra_videoregs[0] & 0x0000c000) >> 14;
	layers[1] = 1;
	layerpri[1] = (m_scrb_videoregs[0] & 0x0000c000) >> 14;
	layers[2] = 2;
	layerpri[2] = (m_scrc_videoregs[0] & 0x0000c000) >> 14;

	sortlayers(layers, layerpri);

	rectangle clip;
	const rectangle &visarea = screen.visible_area();
	clip = visarea;

	for (int y=0;y<240;y++)
	{
		clip.min_y = clip.max_y = y;
		draw_layer(screen, bitmap, clip, layers[0], y);
	}

	draw_sprites(bitmap, cliprect, 0);

	for (int y=0;y<240;y++)
	{
		clip.min_y = clip.max_y = y;
		draw_layer(screen, bitmap, clip, layers[1], y);
	}


	draw_sprites(bitmap, cliprect, 1);

	for (int y=0;y<240;y++)
	{
		clip.min_y = clip.max_y = y;
		draw_layer(screen, bitmap, clip, layers[2], y);
	}


	draw_sprites(bitmap, cliprect, 2);
	draw_sprites(bitmap, cliprect, 3);
	m_text_tilemap->draw(screen, bitmap, cliprect, 0, 0);

#if 0
popmessage  ("scra - %08x %08x %08x\nscrb - %08x %08x %08x\nscrc - %08x %08x %08x",
m_scra_videoregs[0]&0xffff33ff, // yyyyxxxx
m_scra_videoregs[1], // ??? more scrolling?
m_scra_videoregs[2], // 08 - 0b

m_scrb_videoregs[0]&0xffff33ff, // 00 - 03
m_scrb_videoregs[1], // 04 - 07
m_scrb_videoregs[2], // 08 - 0b

m_scrc_videoregs[0]&0xffff33ff, // 00 - 03
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
