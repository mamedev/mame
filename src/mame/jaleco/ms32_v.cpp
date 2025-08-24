// license:BSD-3-Clause
// copyright-holders:David Haywood,Paul Priest, Luca Elia
/* Jaleco MegaSystem 32 Video Hardware */

/* The Video Hardware is Similar to the Non-MS32 Version of Tetris Plus 2 */

/* Plenty to do, see list in drivers/ms32.c */

/*

priority should be given to
(a) dekluding the priorities, the kludge for kirarast made it easier to emulate the rest of it until then
(b) working out the background registers correctly ...
*/


#include "emu.h"
#include "ms32.h"


/********** Tilemaps **********/

TILE_GET_INFO_MEMBER(ms32_state::get_ms32_tx_tile_info)
{
	const int tileno = m_txram[tile_index *2]   & 0xffff;
	const int colour = m_txram[tile_index *2+1] & 0x000f;

	tileinfo.set(2,tileno,colour,0);
}

TILE_GET_INFO_MEMBER(ms32_state::get_ms32_roz_tile_info)
{
	const int tileno = m_rozram[tile_index *2]   & 0xffff;
	const int colour = m_rozram[tile_index *2+1] & 0x000f;

	tileinfo.set(0,tileno,colour,0);
}

TILE_GET_INFO_MEMBER(ms32_state::get_ms32_bg_tile_info)
{
	const int tileno = m_bgram[tile_index *2]   & 0xffff;
	const int colour = m_bgram[tile_index *2+1] & 0x000f;

	tileinfo.set(1,tileno,colour,0);
}

TILE_GET_INFO_MEMBER(ms32_f1superbattle_state::get_ms32_extra_tile_info)
{
	const int tileno = m_road_vram[tile_index *2]   & 0xffff;
	const int colour = m_road_vram[tile_index *2+1] & 0x000f;

	tileinfo.set(3,tileno,colour+0x50,0);
}



void ms32_state::video_start()
{
	m_tx_tilemap     = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ms32_state::get_ms32_tx_tile_info)),  TILEMAP_SCAN_ROWS,  8, 8,  64, 64);
	m_bg_tilemap     = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ms32_state::get_ms32_bg_tile_info)),  TILEMAP_SCAN_ROWS, 16,16,  64, 64);
	// alt layout, controller by register
	m_bg_tilemap_alt = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ms32_state::get_ms32_bg_tile_info)),  TILEMAP_SCAN_ROWS, 16,16, 256, 16);
	m_roz_tilemap    = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ms32_state::get_ms32_roz_tile_info)), TILEMAP_SCAN_ROWS, 16,16, 128,128);

	m_objectram_size = m_sprram.length();
	m_sprram_buffer = make_unique_clear<u16[]>(m_objectram_size);

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

	// tp2m32 doesn't set the brightness registers so we need sensible defaults
	m_brt[0] = m_brt[1] = 0xffff;
	m_sprite_ctrl[0x10/4] = 0x8000;

	save_pointer(NAME(m_sprram_buffer), m_objectram_size);
	save_item(NAME(m_temp_bitmap_tilemaps));
	save_item(NAME(m_temp_bitmap_sprites));
	save_item(NAME(m_temp_bitmap_sprites_pri));
	save_item(NAME(m_tilemaplayoutcontrol));
	save_item(NAME(m_brt));
	save_item(NAME(m_brt_r));
	save_item(NAME(m_brt_g));
	save_item(NAME(m_brt_b));
}

void ms32_f1superbattle_state::video_start()
{
	ms32_state::video_start();

	m_extra_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ms32_f1superbattle_state::get_ms32_extra_tile_info)), TILEMAP_SCAN_ROWS, 2048, 1, 1, 0x400);
}

/********** PALETTE WRITES **********/


// TODO: fix p47aces brightness
// intro text should actually appear one line at a time instead of fading-in altogether,
// see https://youtu.be/PQsefFtqAwA
void ms32_state::update_color(int color)
{
	// anything above text layer is unaffected (maybe a priority setting?)
	// that means this must happen at mixing time rather than here ...
	// bnstars gameplay: 0x0000 0x0000 0x8080 0x0080
	// desertwr ranking: 0x8080 0xff80 0x0000 0x0000
	// gametngk: sets upper words of first two regs as 0x0100xxxx (discarded?)
	//          gameplay:0x0000 0x0000 0x2020 0x0020
	//          continue:0x5050 0x0050 0x2020 0x0020
	// hayaosi3 title:   0x7070 0x0070 0x0000 0x0000
	// p47aces: bomb on stage clear fade out (untested, tbd)

	int r,g,b;

	/* I'm not sure how the brightness should be applied, currently I'm only
	   affecting bg & sprites, not fg.
	   The second brightness control might apply to shadows, see gametngk.
	 */
	if (~color & 0x4000)
	{
		r = ((m_palram[color*2] & 0xff00) >> 8) * m_brt_r / 0x100;
		g = ((m_palram[color*2] & 0x00ff) >> 0) * m_brt_g / 0x100;
		b = ((m_palram[color*2+1] & 0x00ff) >> 0) * m_brt_b / 0x100;
	}
	else
	{
		r = ((m_palram[color*2] & 0xff00) >> 8);
		g = ((m_palram[color*2] & 0x00ff) >> 0);
		b = ((m_palram[color*2+1] & 0x00ff) >> 0);
	}

	m_palette->set_pen_color(color,rgb_t(r,g,b));
}

void ms32_state::ms32_brightness_w(offs_t offset, u32 data, u32 mem_mask)
{
	const u32 oldword = m_brt[offset];
	COMBINE_DATA(&m_brt[offset]);


	if (m_brt[offset] != oldword)
	{
		// TODO: bank "1" is for sprite colors
		const u32 bank = ((offset & 2) >> 1) * 0x4000;
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
}


/* SPRITES based on tetrisp2 for now, readd priority bits later */
void ms32_state::draw_sprites(bitmap_ind16 &bitmap, bitmap_ind8 &bitmap_pri, const rectangle &cliprect, u16 *sprram_top)
{
	const size_t sprite_tail = m_objectram_size - 8; //(0x20000 - 0x10) / 2;
	u16 *source = sprram_top;
	u16 *finish = sprram_top + sprite_tail;
	// TODO: sprite control 0x10 also uses bits 0-11 for sprite start address?
	// akiss uses it for double buffer animations, flips between 0 and 0x800 (and is ugly for latter)
	const bool reverseorder = (m_sprite_ctrl[0x10/4] & 0x8000) == 0x0000;

	if (reverseorder == true)
	{
		source = sprram_top + sprite_tail;
		finish = sprram_top;
	}

	for (;reverseorder ? (source>=finish) : (source<finish); reverseorder ? (source-=8) : (source+=8))
	{
		bool disable;
		u8 pri;
		bool flipx, flipy;
		u32 code, color;
		u8 tx, ty;
		u16 xsize, ysize;
		s32 sx, sy;
		u16 xzoom, yzoom;

		m_sprite->extract_parameters(source, disable, pri, flipx, flipy, code, color, tx, ty, xsize, ysize, sx, sy, xzoom, yzoom);

		if (disable || !xzoom || !yzoom)
			continue;

		// passes the priority as the upper bits of the colour
		// for post-processing in mixer instead
		m_sprite->prio_zoom_transpen_raw(bitmap,cliprect,
				code,
				color<<8 | pri<<8,
				flipx, flipy,
				sx, sy,
				tx, ty, xsize, ysize,
				xzoom, yzoom, bitmap_pri, 0, 0);
	}   /* end sprite loop */
}


void ms32_state::draw_roz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect,int priority)
{
	// TODO: registers 0x40 / 0x44 and 0x50 / 0x54 are used, unknown meaning
	// Given how this works out it is most likely that 0x*0 controls X axis while 0x*4 Y,
	// nothing is known to diverge between settings so far (i.e. bbbxing sets 0xffff to 0x4* and 0x0000 to 0x5*).
	//             0x4*   0x5*  ROZ should wrap?
	// bbbxing:  0xffff 0x0000  0 (match presentation)
	// gratia:   0x0000 0x0000  1 (sky in stage 2)
	// p47aces:  0xffff 0x0651  0 (title screen)
	// desertwr: 0xffff 0x0651  1 (any stage)
	// f1superb: 0xffff 0x0000  ?
	// suchie2:  0x0000 0x0000  0?
	// bnstars:  0x0000 0x0000  ?
	// hayaosi3: 0x0000 0x0000  ?
	// akiss:    0xffff 0x0000  0 (gal riichi, cfr. attract mode)
	// Maybe wrapping is done by limit boundaries rather than individual bits, so that bbbxing and p47aces abuses of this behaviour?
	// Are we missing a ROZ plane size as well?

	if (m_roz_ctrl[0x5c/4] & 1)  /* "super" mode */
	{
		rectangle my_clip;

		my_clip.min_x = cliprect.min_x;
		my_clip.max_x = cliprect.max_x;

		int y = cliprect.min_y;
		const int maxy = cliprect.max_y;

		while (y <= maxy)
		{
			u16 *lineaddr = &m_lineram[8 * (y & 0xff)];

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



u32 ms32_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int scrollx,scrolly;
	int asc_pri;
	int scr_pri;
	int rot_pri;

	/*
	    sprite control regs
	            0x1c 0x20 0x40 <prihack>
	    akiss      0    0    0     0
	    bbbxing    0    0    0     0
	    bnstars    1    0    0     1
	    bnstars1   ?    ?    ?     1
	    desertwr   1    0    0     0
	    f1superb   0    0    0     0
	    gametngk   0 0140    0     0
	    gratia     0    0    0     0
	    hayaosi2   0    0    0     0
	    hayaosi3   0 0140    0     1
	    kirarast   0    0    0     1
	    p47aces    1    0    0     0
	    suchie2    0    0    0     1
	    tetrisp    0    0    0     0
	    tp2m32     0 0140    0     1
	    wpksocv2   0 0140    0     1
	*/
//  popmessage("%04x %04x %04x",m_sprite_ctrl[0x1c/4], m_sprite_ctrl[0x20/4], m_sprite_ctrl[0x40/4]);
//  popmessage("%04x %04x %04x %04x|%04x %04x %04x",m_sprite_ctrl[0x00/4],m_sprite_ctrl[0x04/4],m_sprite_ctrl[0x08/4],m_sprite_ctrl[0x0c/4]
//                   ,m_sprite_ctrl[0x10/4],m_sprite_ctrl[0x14/4],m_sprite_ctrl[0x18/4]);
	/* TODO: registers 0x04/4 and 0x10/4 are used too; the most interesting case
	   is gametngk, where they are *usually*, but not always, copies of 0x00/4
	   and 0x0c/4 (used for scrolling).
	   0x10/4 is 0xdf in most games (apart from gametngk's special case), but
	   it's 0x00 in hayaosi1 and kirarast, and 0xe2 (!) in gratia's tx layer.
	   The two registers might be somewhat related to the width and height of the
	   tilemaps, but there's something that just doesn't fit.
	 */

	// TODO: move to a cache system
	for (int i = 0; i < m_palette->entries(); i++) // colors 0x3000-0x3fff are not used
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

	draw_sprites(m_temp_bitmap_sprites, m_temp_bitmap_sprites_pri, cliprect, m_sprram_buffer.get());

	// TODO: actually understand this (per-scanline priority and alpha-blend over every layer?)
	asc_pri = scr_pri = rot_pri = 0;

	if((m_priram[0x2b00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		rot_pri++;

	if((m_priram[0x2e00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		scr_pri++;

	// Suchiepai 2 title & Gratia gameplay intermissions uses 0x0f
	// hayaosi3 uses 0x09 during flames screen on attract (text should go above the rest)
	// this is otherwise 0x17 most of the time except for 0x15 in hayaosi3, tetris plus 2 & world pk soccer 2
	// kirarast flips between 0x16 in gameplay and 0x17 otherwise
	if(m_priram[0x3a00 / 2] == 0x09)
		asc_pri = 3;
	if((m_priram[0x3a00 / 2] & 0x0030) == 0x00)
		scr_pri++;
	else
		rot_pri++;

	//popmessage("%02x %02x %02x %d %d %d",m_priram[0x2b00 / 2],m_priram[0x2e00 / 2],m_priram[0x3a00 / 2], asc_pri, scr_pri, rot_pri);

	// tile-tile mixing
	for(int prin=0;prin<4;prin++)
	{
		if(rot_pri == prin)
			draw_roz(screen, m_temp_bitmap_tilemaps, cliprect, 1 << 1);
		else if (scr_pri == prin)
		{
			if (m_tilemaplayoutcontrol&1)
			{
				m_bg_tilemap_alt->draw(screen, m_temp_bitmap_tilemaps, cliprect, 0, 1 << 0);
			}
			else
			{
				m_bg_tilemap->draw(screen, m_temp_bitmap_tilemaps, cliprect, 0, 1 << 0);
			}
		}
		else if(asc_pri == prin)
			m_tx_tilemap->draw(screen, m_temp_bitmap_tilemaps, cliprect, 0, 1 << 2);
	}

	// tile-sprite mixing
	// TODO: spaghetti code
	// TODO: complete guesswork and missing many spots
	// TODO: move to a reusable function
	/* it should be using ALL the data in the priority ram, probably for
	   per-pixel / pen mixing, or more levels than are supported here..
	   I don't know, it will need hw tests I think */
	{
		pen_t const *const paldata = m_palette->pens();
		bitmap.fill(0, cliprect);

		for (int yy = cliprect.min_y; yy <= cliprect.max_y; yy++)
		{
			u16 const *const srcptr_tile =     &m_temp_bitmap_tilemaps.pix(yy);
			u8 const *const  srcptr_tilepri =  &screen.priority().pix(yy);
			u16 const *const srcptr_spri =     &m_temp_bitmap_sprites.pix(yy);
			//u8 const *const  srcptr_spripri =  &m_temp_bitmap_sprites_pri.pix(yy);
			u32 *const       dstptr_bitmap  =  &bitmap.pix(yy);

			for (int xx = cliprect.min_x; xx <= cliprect.max_x; xx++)
			{
				u16 src_tile  = srcptr_tile[xx];
				u8 src_tilepri = srcptr_tilepri[xx];
				u16 src_spri = srcptr_spri[xx];
				//u8 src_spripri;// = srcptr_spripri[xx];
				u16 spridat = (src_spri & 0x0fff);
				u8  spritepri = ((src_spri & 0xf000) >> 8);
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
					popmessage("unhandled priority type %02x, contact MAMEdev",primask);
				}
				else if (primask == 0xcc)
				{
					// hayaosi3 final round ($00 normal, $02 mesh, $03/$05/$07 zoomed in)
					// TODO: may have some blending, hard to say without ref video
					if (src_tilepri & 0x02)
						dstptr_bitmap[xx] = paldata[src_tile];
					else
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat];
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}
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
				else if(primask == 0xf8) // gratia ending
				{
					if (spridat & 0xff && src_tilepri == 0x02)
						dstptr_bitmap[xx] = paldata[spridat];
					else
						dstptr_bitmap[xx] = paldata[src_tile];
				}
				else
				{
					// $fa actually used on hayaosi3 second champ transition, unknown purpose
					dstptr_bitmap[xx] = 0;
					popmessage("unhandled priority type %02x, contact MAMEdev",primask);
				}
			}
		}
	}

	return 0;
}

void ms32_state::screen_vblank(int state)
{
	if (state)
	{
		std::copy_n(&m_sprram[0], m_objectram_size, &m_sprram_buffer[0]);
	}
}

void ms32_state::flipscreen_w(int state)
{
	m_tx_tilemap->set_flip(state ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	m_bg_tilemap->set_flip(state ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	m_bg_tilemap_alt->set_flip(state ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	m_roz_tilemap->set_flip(state ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	// TODO: sprite device
}
