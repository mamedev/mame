// license:BSD-3-Clause
// copyright-holders:David Haywood,Paul Priest
/* Jaleco MegaSystem 32 Video Hardware */

/* The Video Hardware is Similar to the Non-MS32 Version of Tetris Plus 2 */

/* Plenty to do, see list in drivers/ms32.c */

/*

priority should be given to
(a) dekluding the priorities, the kludge for kirarast made it easier to emulate the rest of it until then
(b) working out the background registers correctly ...
*/


#include "emu.h"
#include "includes/ms32.h"


// kirarast, tp2m32, and 47pie2 require the sprites in a different order

/********** Tilemaps **********/

TILE_GET_INFO_MEMBER(ms32_state::get_ms32_tx_tile_info)
{
	int tileno, colour;

	tileno = m_txram[tile_index *2]   & 0xffff;
	colour = m_txram[tile_index *2+1] & 0x000f;

	SET_TILE_INFO_MEMBER(3,tileno,colour,0);
}

TILE_GET_INFO_MEMBER(ms32_state::get_ms32_roz_tile_info)
{
	int tileno,colour;

	tileno = m_rozram[tile_index *2]   & 0xffff;
	colour = m_rozram[tile_index *2+1] & 0x000f;

	SET_TILE_INFO_MEMBER(1,tileno,colour,0);
}

TILE_GET_INFO_MEMBER(ms32_state::get_ms32_bg_tile_info)
{
	int tileno,colour;

	tileno = m_bgram[tile_index *2]   & 0xffff;
	colour = m_bgram[tile_index *2+1] & 0x000f;

	SET_TILE_INFO_MEMBER(2,tileno,colour,0);
}

TILE_GET_INFO_MEMBER(ms32_state::get_ms32_extra_tile_info)
{
	int tileno,colour;

	tileno = m_f1superb_extraram[tile_index *2]   & 0xffff;
	colour = m_f1superb_extraram[tile_index *2+1] & 0x000f;

	SET_TILE_INFO_MEMBER(4,tileno,colour+0x50,0);
}



void ms32_state::video_start()
{
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ms32_state::get_ms32_tx_tile_info),this),TILEMAP_SCAN_ROWS,8, 8,64,64);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ms32_state::get_ms32_bg_tile_info),this),TILEMAP_SCAN_ROWS,16,16,64,64);
	m_bg_tilemap_alt = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ms32_state::get_ms32_bg_tile_info),this),TILEMAP_SCAN_ROWS,16,16,256,16); // alt layout, controller by register?
	m_roz_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ms32_state::get_ms32_roz_tile_info),this),TILEMAP_SCAN_ROWS,16,16,128,128);


	/* set up tile layers */
	m_screen->register_screen_bitmap(m_temp_bitmap_tilemaps);
	m_screen->register_screen_bitmap(m_temp_bitmap_sprites);
	m_screen->register_screen_bitmap(m_temp_bitmap_sprites_pri); // not actually being used for rendering, we embed pri info in the raw colour bitmap

	m_temp_bitmap_tilemaps.fill(0);
	m_temp_bitmap_sprites.fill(0);
	m_temp_bitmap_sprites_pri.fill(0);

	m_tx_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_transparent_pen(0);
	m_bg_tilemap_alt->set_transparent_pen(0);
	m_roz_tilemap->set_transparent_pen(0);

	m_reverse_sprite_order = 1;

	/* i hate per game patches...how should priority really work? tetrisp2.c ? i can't follow it */
	if (!strcmp(machine().system().name,"kirarast"))    m_reverse_sprite_order = 0;
	if (!strcmp(machine().system().name,"tp2m32"))  m_reverse_sprite_order = 0;
	if (!strcmp(machine().system().name,"47pie2"))  m_reverse_sprite_order = 0;
	if (!strcmp(machine().system().name,"47pie2o")) m_reverse_sprite_order = 0;
	if (!strcmp(machine().system().name,"hayaosi3"))    m_reverse_sprite_order = 0;
	if (!strcmp(machine().system().name,"bnstars")) m_reverse_sprite_order = 0;
	if (!strcmp(machine().system().name,"wpksocv2"))    m_reverse_sprite_order = 0;

	// tp2m32 doesn't set the brightness registers so we need sensible defaults
	m_brt[0] = m_brt[1] = 0xffff;

	save_item(NAME(m_irqreq));
	save_item(NAME(m_temp_bitmap_tilemaps));
	save_item(NAME(m_temp_bitmap_sprites));
	save_item(NAME(m_temp_bitmap_sprites_pri));
	save_item(NAME(m_tilemaplayoutcontrol));
	save_item(NAME(m_reverse_sprite_order));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_brt));
	save_item(NAME(m_brt_r));
	save_item(NAME(m_brt_g));
	save_item(NAME(m_brt_b));
}

VIDEO_START_MEMBER(ms32_state,f1superb)
{
	ms32_state::video_start();

	m_extra_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ms32_state::get_ms32_extra_tile_info),this),TILEMAP_SCAN_ROWS,2048,1,1,0x400);
}

/********** PALETTE WRITES **********/


void ms32_state::update_color(int color)
{
	int r,g,b;

	/* I'm not sure how the brightness should be applied, currently I'm only
	   affecting bg & sprites, not fg.
	   The second brightness control might apply to shadows, see gametngk.
	 */
	if (~color & 0x4000)
	{
		r = ((m_palram[color*2] & 0xff00) >>8 ) * m_brt_r / 0x100;
		g = ((m_palram[color*2] & 0x00ff) >>0 ) * m_brt_g / 0x100;
		b = ((m_palram[color*2+1] & 0x00ff) >>0 ) * m_brt_b / 0x100;
	}
	else
	{
		r = ((m_palram[color*2] & 0xff00) >>8 );
		g = ((m_palram[color*2] & 0x00ff) >>0 );
		b = ((m_palram[color*2+1] & 0x00ff) >>0 );
	}

	m_palette->set_pen_color(color,rgb_t(r,g,b));
}

WRITE32_MEMBER(ms32_state::ms32_brightness_w)
{
	int oldword = m_brt[offset];
	COMBINE_DATA(&m_brt[offset]);

	if (m_brt[offset] != oldword)
	{
		int bank = ((offset & 2) >> 1) * 0x4000;
		//int i;

		if (bank == 0)
		{
			m_brt_r = 0x100 - ((m_brt[0] & 0xff00) >> 8);
			m_brt_g = 0x100 - ((m_brt[0] & 0x00ff) >> 0);
			m_brt_b = 0x100 - ((m_brt[1] & 0x00ff) >> 0);

		//  for (i = 0;i < 0x3000;i++)  // colors 0x3000-0x3fff are not used
		//      update_color(machine(), i);
		}
	}

//popmessage("%04x %04x %04x %04x",m_brt[0],m_brt[1],m_brt[2],m_brt[3]);
}






WRITE32_MEMBER(ms32_state::ms32_gfxctrl_w)
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 1 = flip screen */
		m_flipscreen = data & 0x02;
		m_tx_tilemap->set_flip(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
		m_bg_tilemap->set_flip(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
		m_bg_tilemap_alt->set_flip(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

		/* bit 2 used by f1superb, unknown */

		/* bit 3 used by several games, unknown */

//popmessage("%08x",data);
	}
}



/* SPRITES based on tetrisp2 for now, readd priority bits later */
void ms32_state::draw_sprites(bitmap_ind16 &bitmap, bitmap_ind8 &bitmap_pri, const rectangle &cliprect, UINT16 *sprram_top, size_t sprram_size, int gfxnum, int reverseorder)
{
	int tx, ty, sx, sy, flipx, flipy;
	int xsize, ysize;
	int code, attr, color, size;
	int pri;
	int xzoom, yzoom;
	gfx_element *gfx = m_gfxdecode->gfx(gfxnum);

	UINT16      *source =   sprram_top;
	UINT16  *finish =   sprram_top + (sprram_size - 0x10) / 2;

	if (reverseorder == 1)
	{
		source  = sprram_top + (sprram_size - 0x10) / 2;
		finish  = sprram_top;
	}

	for (;reverseorder ? (source>=finish) : (source<finish); reverseorder ? (source-=8) : (source+=8))
	{
		attr    =   source[ 0 ];
		pri = (attr & 0x00f0);

		if ((attr & 0x0004) == 0)           continue;

		flipx   =   attr & 1;
		flipy   =   attr & 2;
		code    =   source[ 1 ];
		color   =   source[ 2 ];
		tx      =   (code >> 0) & 0xff;
		ty      =   (code >> 8) & 0xff;

		code    =   (color & 0x0fff);
		color   =   (color >> 12) & 0xf;
		size    =   source[ 3 ];

		xsize   =   ((size >> 0) & 0xff) + 1;
		ysize   =   ((size >> 8) & 0xff) + 1;

		sx      =   (source[5] & 0x3ff) - (source[5] & 0x400);
		sy      =   (source[4] & 0x1ff) - (source[4] & 0x200);

		xzoom   =   (source[ 6 ]&0xffff);
		yzoom   =   (source[ 7 ]&0xffff);

		{
			if (!yzoom || !xzoom)
				continue;

			yzoom = 0x1000000/yzoom;
			xzoom = 0x1000000/xzoom;
		}


		gfx->set_source_clip(tx, xsize, ty, ysize);

		{
			// passes the priority as the upper bits of the colour
			// for post-processing in mixer instead
			gfx->prio_zoom_transpen_raw(bitmap,cliprect,
					code,
					color<<8 | pri<<8,
					flipx, flipy,
					sx,sy,
					xzoom, yzoom, bitmap_pri,0, 0);
		}
	}   /* end sprite loop */
}


void ms32_state::draw_roz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect,int priority)
{
	/* TODO: registers 0x40/4 / 0x44/4 and 0x50/4 / 0x54/4 are used, meaning unknown */

	if (m_roz_ctrl[0x5c/4] & 1)  /* "super" mode */
	{
		rectangle my_clip;
		int y,maxy;

		my_clip.min_x = cliprect.min_x;
		my_clip.max_x = cliprect.max_x;

		y = cliprect.min_y;
		maxy = cliprect.max_y;

		while (y <= maxy)
		{
			UINT16 *lineaddr = m_lineram + 8 * (y & 0xff);

			int start2x = (lineaddr[0x00/4] & 0xffff) | ((lineaddr[0x04/4] & 3) << 16);
			int start2y = (lineaddr[0x08/4] & 0xffff) | ((lineaddr[0x0c/4] & 3) << 16);
			int incxx  = (lineaddr[0x10/4] & 0xffff) | ((lineaddr[0x14/4] & 1) << 16);
			int incxy  = (lineaddr[0x18/4] & 0xffff) | ((lineaddr[0x1c/4] & 1) << 16);
			int startx = (m_roz_ctrl[0x00/4] & 0xffff) | ((m_roz_ctrl[0x04/4] & 3) << 16);
			int starty = (m_roz_ctrl[0x08/4] & 0xffff) | ((m_roz_ctrl[0x0c/4] & 3) << 16);
			int offsx  = m_roz_ctrl[0x30/4];
			int offsy  = m_roz_ctrl[0x34/4];

			my_clip.min_y = my_clip.max_y = y;

			offsx += (m_roz_ctrl[0x38/4] & 1) * 0x400;   // ??? gratia, hayaosi1...
			offsy += (m_roz_ctrl[0x3c/4] & 1) * 0x400;   // ??? gratia, hayaosi1...

			/* extend sign */
			if (start2x & 0x20000) start2x |= ~0x3ffff;
			if (start2y & 0x20000) start2y |= ~0x3ffff;
			if (startx & 0x20000) startx |= ~0x3ffff;
			if (starty & 0x20000) starty |= ~0x3ffff;
			if (incxx & 0x10000) incxx |= ~0x1ffff;
			if (incxy & 0x10000) incxy |= ~0x1ffff;

			m_roz_tilemap->draw_roz(screen, bitmap, my_clip,
					(start2x+startx+offsx)<<16, (start2y+starty+offsy)<<16,
					incxx<<8, incxy<<8, 0, 0,
					1, // Wrap
					0, priority);

			y++;
		}
	}
	else    /* "simple" mode */
	{
		int startx = (m_roz_ctrl[0x00/4] & 0xffff) | ((m_roz_ctrl[0x04/4] & 3) << 16);
		int starty = (m_roz_ctrl[0x08/4] & 0xffff) | ((m_roz_ctrl[0x0c/4] & 3) << 16);
		int incxx  = (m_roz_ctrl[0x10/4] & 0xffff) | ((m_roz_ctrl[0x14/4] & 1) << 16);
		int incxy  = (m_roz_ctrl[0x18/4] & 0xffff) | ((m_roz_ctrl[0x1c/4] & 1) << 16);
		int incyy  = (m_roz_ctrl[0x20/4] & 0xffff) | ((m_roz_ctrl[0x24/4] & 1) << 16);
		int incyx  = (m_roz_ctrl[0x28/4] & 0xffff) | ((m_roz_ctrl[0x2c/4] & 1) << 16);
		int offsx  = m_roz_ctrl[0x30/4];
		int offsy  = m_roz_ctrl[0x34/4];

		offsx += (m_roz_ctrl[0x38/4] & 1) * 0x400;   // ??? gratia, hayaosi1...
		offsy += (m_roz_ctrl[0x3c/4] & 1) * 0x400;   // ??? gratia, hayaosi1...

		/* extend sign */
		if (startx & 0x20000) startx |= ~0x3ffff;
		if (starty & 0x20000) starty |= ~0x3ffff;
		if (incxx & 0x10000) incxx |= ~0x1ffff;
		if (incxy & 0x10000) incxy |= ~0x1ffff;
		if (incyy & 0x10000) incyy |= ~0x1ffff;
		if (incyx & 0x10000) incyx |= ~0x1ffff;

		m_roz_tilemap->draw_roz(screen, bitmap, cliprect,
				(startx+offsx)<<16, (starty+offsy)<<16,
				incxx<<8, incxy<<8, incyx<<8, incyy<<8,
				1, // Wrap
				0, priority);
	}
}



UINT32 ms32_state::screen_update_ms32(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int scrollx,scrolly;
	int asc_pri;
	int scr_pri;
	int rot_pri;

	/* TODO: registers 0x04/4 and 0x10/4 are used too; the most interesting case
	   is gametngk, where they are *usually*, but not always, copies of 0x00/4
	   and 0x0c/4 (used for scrolling).
	   0x10/4 is 0xdf in most games (apart from gametngk's special case), but
	   it's 0x00 in hayaosi1 and kirarast, and 0xe2 (!) in gratia's tx layer.
	   The two registers might be somewhat related to the width and height of the
	   tilemaps, but there's something that just doesn't fit.
	 */
	int i;

	for (i = 0;i < 0x10000;i++) // colors 0x3000-0x3fff are not used
		update_color(i);

	scrollx = m_tx_scroll[0x00/4] + m_tx_scroll[0x08/4] + 0x18;
	scrolly = m_tx_scroll[0x0c/4] + m_tx_scroll[0x14/4];
	m_tx_tilemap->set_scrollx(0, scrollx);
	m_tx_tilemap->set_scrolly(0, scrolly);

	scrollx = m_bg_scroll[0x00/4] + m_bg_scroll[0x08/4] + 0x10;
	scrolly = m_bg_scroll[0x0c/4] + m_bg_scroll[0x14/4];
	m_bg_tilemap->set_scrollx(0, scrollx);
	m_bg_tilemap->set_scrolly(0, scrolly);
	m_bg_tilemap_alt->set_scrollx(0, scrollx);
	m_bg_tilemap_alt->set_scrolly(0, scrolly);


	screen.priority().fill(0, cliprect);



	/* TODO: 0 is correct for gametngk, but break f1superb scrolling grid (text at
	   top and bottom of the screen becomes black on black) */
	m_temp_bitmap_tilemaps.fill(0, cliprect);   /* bg color */

	/* clear our sprite bitmaps */
	m_temp_bitmap_sprites.fill(0, cliprect);
	m_temp_bitmap_sprites_pri.fill(0, cliprect);

	draw_sprites(m_temp_bitmap_sprites, m_temp_bitmap_sprites_pri, cliprect, m_sprram, 0x20000, 0, m_reverse_sprite_order);




	asc_pri = scr_pri = rot_pri = 0;

	if((m_priram[0x2b00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		rot_pri++;

	if((m_priram[0x2e00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		scr_pri++;

	if((m_priram[0x3a00 / 2] & 0x00ff) == 0x000c)
		scr_pri++;
	else
		rot_pri++;

	if (rot_pri == 0)
		draw_roz(screen, m_temp_bitmap_tilemaps, cliprect, 1 << 1);
	else if (scr_pri == 0)
		if (m_tilemaplayoutcontrol&1)
		{
			m_bg_tilemap_alt->draw(screen, m_temp_bitmap_tilemaps, cliprect, 0, 1 << 0);
		}
		else
		{
			m_bg_tilemap->draw(screen, m_temp_bitmap_tilemaps, cliprect, 0, 1 << 0);
		}
	else if (asc_pri == 0)
		m_tx_tilemap->draw(screen, m_temp_bitmap_tilemaps, cliprect, 0, 1 << 2);

	if (rot_pri == 1)
		draw_roz(screen, m_temp_bitmap_tilemaps, cliprect, 1 << 1);
	else if (scr_pri == 1)
		if (m_tilemaplayoutcontrol&1)
		{
			m_bg_tilemap_alt->draw(screen, m_temp_bitmap_tilemaps, cliprect, 0, 1 << 0);
		}
		else
		{
			m_bg_tilemap->draw(screen, m_temp_bitmap_tilemaps, cliprect, 0, 1 << 0);
		}
	else if (asc_pri == 1)
		m_tx_tilemap->draw(screen, m_temp_bitmap_tilemaps, cliprect, 0, 1 << 2);

	if (rot_pri == 2)
		draw_roz(screen, m_temp_bitmap_tilemaps, cliprect, 1 << 1);
	else if (scr_pri == 2)
		if (m_tilemaplayoutcontrol&1)
		{
			m_bg_tilemap_alt->draw(screen, m_temp_bitmap_tilemaps, cliprect, 0, 1 << 0);
		}
		else
		{
			m_bg_tilemap->draw(screen, m_temp_bitmap_tilemaps, cliprect, 0, 1 << 0);
		}
	else if (asc_pri == 2)
		m_tx_tilemap->draw(screen, m_temp_bitmap_tilemaps, cliprect, 0, 1 << 2);

	/* MIX it! */
	/* this mixing isn't 100% accurate, it should be using ALL the data in
	   the priority ram, probably for per-pixel / pen mixing, or more levels
	   than are supported here..  I don't know, it will need hw tests I think */
	{
		int xx, yy;
		int width = screen.width();
		int height = screen.height();
		const pen_t *paldata = m_palette->pens();

		UINT16* srcptr_tile;
		UINT8* srcptr_tilepri;
		UINT16* srcptr_spri;
		//UINT8* srcptr_spripri;

		UINT32* dstptr_bitmap;

		bitmap.fill(0, cliprect);

		for (yy=0;yy<height;yy++)
		{
			srcptr_tile =     &m_temp_bitmap_tilemaps.pix16(yy);
			srcptr_tilepri =  &screen.priority().pix8(yy);
			srcptr_spri =     &m_temp_bitmap_sprites.pix16(yy);
			//srcptr_spripri =  &m_temp_bitmap_sprites_pri.pix8(yy);
			dstptr_bitmap  =  &bitmap.pix32(yy);
			for (xx=0;xx<width;xx++)
			{
				UINT16 src_tile  = srcptr_tile[xx];
				UINT8 src_tilepri = srcptr_tilepri[xx];
				UINT16 src_spri = srcptr_spri[xx];
				//UINT8 src_spripri;// = srcptr_spripri[xx];
				UINT16 spridat = ((src_spri&0x0fff));
				UINT8  spritepri =     ((src_spri&0xf000) >> 8);
				int primask = 0;

				// get sprite priority value back out of bitmap/colour data (this is done in draw_sprite for standalone hw)
				if (m_priram[(spritepri | 0x0a00 | 0x1500) / 2] & 0x38) primask |= 1 << 0;
				if (m_priram[(spritepri | 0x0a00 | 0x1400) / 2] & 0x38) primask |= 1 << 1;
				if (m_priram[(spritepri | 0x0a00 | 0x1100) / 2] & 0x38) primask |= 1 << 2;
				if (m_priram[(spritepri | 0x0a00 | 0x1000) / 2] & 0x38) primask |= 1 << 3;
				if (m_priram[(spritepri | 0x0a00 | 0x0500) / 2] & 0x38) primask |= 1 << 4;
				if (m_priram[(spritepri | 0x0a00 | 0x0400) / 2] & 0x38) primask |= 1 << 5;
				if (m_priram[(spritepri | 0x0a00 | 0x0100) / 2] & 0x38) primask |= 1 << 6;
				if (m_priram[(spritepri | 0x0a00 | 0x0000) / 2] & 0x38) primask |= 1 << 7;


				if (primask == 0x00)
				{
					if (src_tilepri==0x00)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat]; // best bout boxing title
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}
					else if (src_tilepri==0x01)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat]; // best bout boxing title
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}
					else if (src_tilepri==0x02)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat]; // best bout boxing
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}
					else if (src_tilepri==0x03)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat]; // best bout boxing
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}
					else if (src_tilepri==0x04)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat];
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}
					else if (src_tilepri==0x05)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat];
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}
					else if (src_tilepri==0x06)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat];
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}
					else if (src_tilepri==0x07)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat]; // desert war radar?
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}


				}
				else if (primask == 0xc0)
				{
					dstptr_bitmap[xx] = paldata[machine().rand()&0xfff];
				}
				else if (primask == 0xf0)
				{
//                  dstptr_bitmap[xx] = paldata[spridat];
					if (src_tilepri==0x00)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat]; // clouds at top gametngk intro
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}
					else if (src_tilepri==0x01)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat]; // clouds gametngk intro
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}
					else if (src_tilepri==0x02)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat]; // mode select gametngk
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}
					else if (src_tilepri==0x03)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat]; // title gametngk
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}
					else if (src_tilepri==0x04)
					{
						dstptr_bitmap[xx] = paldata[src_tile]; // insert coin text on girl gametngk intro
					}
					else if (src_tilepri==0x05)
					{
						dstptr_bitmap[xx] = paldata[src_tile]; // insert coin gametngk intro
					}
					else if (src_tilepri==0x06)
					{
						dstptr_bitmap[xx] = paldata[src_tile]; // insert coin gametngk intro
					}
					else if (src_tilepri==0x07)
					{
						dstptr_bitmap[xx] = paldata[src_tile]; // insert coin gametngk intro
					}
				}
				else if (primask == 0xfc)
				{
					if (src_tilepri==0x00)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat]; // tetrisp intro text
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}
					else if (src_tilepri==0x01)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat]; // tetrisp intro text
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}
					else if (src_tilepri==0x02)
					{
						dstptr_bitmap[xx] = paldata[src_tile]; // tetrisp story
					}
					else if (src_tilepri==0x03)
					{
						dstptr_bitmap[xx] = paldata[src_tile]; // tetrisp fader to game after story
					}
					else if (src_tilepri==0x04)
					{
						dstptr_bitmap[xx] = paldata[src_tile]; // credit text tetrisp mode select
					}
					else if (src_tilepri==0x05)
					{
						dstptr_bitmap[xx] = paldata[src_tile]; // credit text tetrisp intro
					}
					else if (src_tilepri==0x06)
					{
						//dstptr_bitmap[xx] = paldata[machine().rand()&0xfff];
						dstptr_bitmap[xx] = paldata[src_tile]; // assumed
					}
					else if (src_tilepri==0x07)
					{
						//dstptr_bitmap[xx] = paldata[machine().rand()&0xfff];
						dstptr_bitmap[xx] = paldata[src_tile]; // assumed
					}
				}
				else if (primask == 0xfe)
				{
					if (src_tilepri==0x00)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat]; // screens in gametngk intro
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}
					else if (src_tilepri==0x01)
					{
						dstptr_bitmap[xx] = alpha_blend_r32( paldata[src_tile], 0x00000000, 128); // shadow, gametngk title
					}
					else if (src_tilepri==0x02)
					{
						dstptr_bitmap[xx] = alpha_blend_r32( paldata[src_tile], 0x00000000, 128); // shadow, gametngk mode select
					}
					else if (src_tilepri==0x03)
					{
						dstptr_bitmap[xx] = alpha_blend_r32( paldata[src_tile], 0x00000000, 128); // shadow, gametngk title
					}
					else if (src_tilepri==0x04)
					{
						dstptr_bitmap[xx] = paldata[src_tile]; // credit text gametngk intro
					}
					else if (src_tilepri==0x05)
					{
						dstptr_bitmap[xx] = paldata[src_tile]; // credit text near shadow, gametngk title
					}
					else if (src_tilepri==0x06)
					{
						dstptr_bitmap[xx] = paldata[src_tile]; // credit gametngk highscores
					}
					else if (src_tilepri==0x07)
					{
						dstptr_bitmap[xx] = paldata[src_tile]; // assumed
					}
				}

				else
				{
					dstptr_bitmap[xx] = 0;
					logerror("unhandled priority type %02x\n",primask);
				}



			}

		}

	}


	return 0;
}
