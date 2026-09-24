// license:BSD-3-Clause
// copyright-holders:David Haywood,Paul Priest, Luca Elia, Andrea Bogazzi
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

TILE_GET_INFO_MEMBER(ms32_f1superbattle_state::get_latched_tx_tile_info)
{
	const int tileno = m_txram_latch[tile_index *2]   & 0xffff;
	const int colour = m_txram_latch[tile_index *2+1] & 0x000f;

	tileinfo.set(2,tileno,colour,0);
}

TILE_GET_INFO_MEMBER(ms32_f1superbattle_state::get_ms32_extra_tile_info)
{
	const int tileno = m_road_vram[tile_index *2]   & 0xffff;
	const int colour = m_road_vram[tile_index *2+1] & 0x000f;

	tileinfo.set(3,tileno,colour+0x50,0);
}



void ms32_state::video_start()
{
	m_tx_tilemap     = &create_tx_tilemap();
	m_bg_tilemap     = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ms32_state::get_ms32_bg_tile_info)),  TILEMAP_SCAN_ROWS, 16,16,  64, 64);
	// alt layout, controller by register
	m_bg_tilemap_alt = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ms32_state::get_ms32_bg_tile_info)),  TILEMAP_SCAN_ROWS, 16,16, 256, 16);
	m_roz_tilemap    = &create_roz_tilemap();

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
	m_brt1_r = m_brt1_g = m_brt1_b = 0x100;
	m_sprite_ctrl[0x10/4] = 0x8000;

	m_screen->register_screen_bitmap(m_layer_tx);
	m_screen->register_screen_bitmap(m_layer_bg);
	m_screen->register_screen_bitmap(m_layer_roz);

	save_pointer(NAME(m_sprram_buffer), m_objectram_size);
	save_item(NAME(m_temp_bitmap_tilemaps));
	save_item(NAME(m_temp_bitmap_sprites));
	save_item(NAME(m_temp_bitmap_sprites_pri));
	save_item(NAME(m_tilemaplayoutcontrol));
	save_item(NAME(m_brt));
	save_item(NAME(m_brt_r));
	save_item(NAME(m_brt_g));
	save_item(NAME(m_brt_b));
	save_item(NAME(m_brt1_r));
	save_item(NAME(m_brt1_g));
	save_item(NAME(m_brt1_b));
	save_item(NAME(m_layer_tx));
	save_item(NAME(m_layer_bg));
	save_item(NAME(m_layer_roz));
}

tilemap_t &ms32_state::create_tx_tilemap()
{
	return machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ms32_state::get_ms32_tx_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
}

tilemap_t &ms32_state::create_roz_tilemap()
{
	return machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ms32_state::get_ms32_roz_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 128, 128);
}

tilemap_t &ms32_f1superbattle_state::create_tx_tilemap()
{
	return machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ms32_f1superbattle_state::get_latched_tx_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
}

tilemap_t &ms32_f1superbattle_state::create_roz_tilemap()
{
	return machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ms32_f1superbattle_state::get_ms32_roz_tile_info)), TILEMAP_SCAN_ROWS, 2048, 1, 1, 0x400);
}

void ms32_f1superbattle_state::video_start()
{
	ms32_state::video_start();

	m_extra_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ms32_f1superbattle_state::get_ms32_extra_tile_info)), TILEMAP_SCAN_ROWS, 2048, 1, 1, 0x400);
	m_extra_tilemap->set_transparent_pen(0);

	m_screen->register_screen_bitmap(m_layer_road);

	m_txram_latch.assign(m_txram.length(), 0);
	save_item(NAME(m_txram_latch));
}

void ms32_f1superbattle_state::draw_line_plane(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, tilemap_t *tilemap, u16 const *vram, u16 const *lineram, u32 const *ctrl, bool wrap, u16 *line_colour)
{
	int const startx = util::sext((ctrl[0x00/4] & 0xffff) | ((ctrl[0x04/4] & 3) << 16), 18);
	int const starty = util::sext((ctrl[0x08/4] & 0xffff) | ((ctrl[0x0c/4] & 3) << 16), 18);
	int const offsx = ctrl[0x30/4] + (ctrl[0x38/4] & 1) * 0x400;
	int const offsy = ctrl[0x34/4] + (ctrl[0x3c/4] & 1) * 0x400;

	rectangle clip = cliprect;
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		u16 const *const line = &lineram[8 * (y & 0xff)];
		int const start2x = util::sext((line[0] & 0xffff) | ((line[1] & 3) << 16), 18);
		int const start2y = util::sext((line[2] & 0xffff) | ((line[3] & 3) << 16), 18);
		int const incxx = util::sext((line[4] & 0xffff) | ((line[5] & 1) << 16), 17);
		int const incxy = util::sext((line[6] & 0xffff) | ((line[7] & 1) << 16), 17);

		int const row = (start2y + starty + offsy) & 0x3ff;
		line_colour[y & 0xff] = 0;
		if (!vram[row * 2])
			continue;
		line_colour[y & 0xff] = vram[row * 2 + 1];

		clip.min_y = clip.max_y = y;
		tilemap->draw_roz(screen, bitmap, clip,
				u32(start2x + startx + offsx) << 16, u32(start2y + starty + offsy) << 16,
				incxx * 0x100, incxy * 0x100, 0, 0,
				wrap,
				0, 0);
	}
}

/*
    Per-pixel lookup in priority RAM, index layout matching the other MS32 games' tables:
    bit 12     sprite transparent
    bit 11     text transparent
    bit 10     unknown, always 1 on the games checked
    bit 9      ROZ transparent
    bit 8      road plane transparent (always 1 on games without it)
    bit 7      BG transparent
    bits 6-3   sprite priority (attribute bits 7-4)
    bits 2-0   line depth, colour bits 6-4 of the ROZ line, or of the road plane line where ROZ is transparent

    Output: bits 5-3 select the layer (0 sprite, 1 BG, 2 ROZ, 4 road plane, 6 text), bit 6 selects the backdrop.
    TODO: bit 2 clear is approximated as half brightness, bits 1-0 are ignored
*/
void ms32_f1superbattle_state::mix_layers(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_layer_tx.fill(0xffff, cliprect);
	m_layer_bg.fill(0xffff, cliprect);
	m_layer_road.fill(0xffff, cliprect);
	m_layer_roz.fill(0xffff, cliprect);

	tx_tilemap()->draw(screen, m_layer_tx, cliprect, 0, 0);
	bg_layer_tilemap()->draw(screen, m_layer_bg, cliprect, 0, 0);
	u16 road_line_colour[256] = { };
	u16 roz_line_colour[256] = { };
	draw_line_plane(screen, m_layer_road, cliprect, m_extra_tilemap, &m_road_vram[0], &m_road_lineram[0], &m_road_ctrl[0], true, road_line_colour);
	draw_line_plane(screen, m_layer_roz, cliprect, roz_tilemap(), &m_rozram[0], &m_lineram[0], &m_roz_ctrl[0], false, roz_line_colour);

	pen_t const *const paldata = m_palette->pens();
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		u16 const *const spr = &m_temp_bitmap_sprites.pix(y);
		u16 const *const tx = &m_layer_tx.pix(y);
		u16 const *const bg = &m_layer_bg.pix(y);
		u16 const *const road = &m_layer_road.pix(y);
		u16 const *const roz = &m_layer_roz.pix(y);
		u32 *const dst = &bitmap.pix(y);
		u16 const road_depth = (road_line_colour[y & 0xff] >> 4) & 7;
		u16 const roz_depth = (roz_line_colour[y & 0xff] >> 4) & 7;

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			bool const s_op = (spr[x] & 0xff) != 0;
			u16 const pri = s_op ? (spr[x] >> 12) : 0;
			u16 const depth = (roz[x] != 0xffff) ? roz_depth : road_depth;

			u16 const idx = (!s_op << 12) | ((tx[x] == 0xffff) << 11) | (1 << 10) | ((roz[x] == 0xffff) << 9) | ((road[x] == 0xffff) << 8) | ((bg[x] == 0xffff) << 7) | (pri << 3) | depth;
			u8 const code = m_priram[idx];

			u16 pen = 0;
			if (!BIT(code, 6))
			{
				switch ((code >> 3) & 7)
				{
				case 0: pen = spr[x] & 0x0fff; break;
				case 1: pen = bg[x]; break;
				case 2: pen = roz[x]; break;
				case 4: pen = road[x]; break;
				case 6: pen = tx[x]; break;
				default: pen = 0; break;
				}
				if (pen == 0xffff)
					pen = 0;
			}

			rgb_t c = paldata[pen & 0x7fff];
			if (!BIT(code, 2))
				c = rgb_t(c.r() >> 1, c.g() >> 1, c.b() >> 1);
			dst[x] = c;
		}
	}
}

/********** PALETTE WRITES **********/


// Brightness notes (applied at mix time, not here):
// bnstars gameplay: 0x0000 0x0000 0x8080 0x0080
// desertwr ranking: 0x8080 0xff80 0x0000 0x0000
// gametngk: sets upper words of first two regs as 0x0100xxxx (discarded?)
//          gameplay:0x0000 0x0000 0x2020 0x0020
//          continue:0x5050 0x0050 0x2020 0x0020
// hayaosi3 title:   0x7070 0x0070 0x0000 0x0000
// p47aces: bomb on stage clear fade out (untested, tbd)
void ms32_state::update_color(int color)
{
	const int r = ((m_palram[color*2] & 0xff00) >> 8);
	const int g = ((m_palram[color*2] & 0x00ff) >> 0);
	const int b = ((m_palram[color*2+1] & 0x00ff) >> 0);

	m_palette->set_pen_color(color,rgb_t(r,g,b));
}

void ms32_state::ms32_brightness_w(offs_t offset, u32 data, u32 mem_mask)
{
	const u32 oldword = m_brt[offset];
	COMBINE_DATA(&m_brt[offset]);

	if (m_brt[offset] != oldword)
	{
		// two brightness banks, selected per-pixel by priram output bits 1-0:
		//   bits 1:0 = 11 -> bank 0 (brt[0]/brt[1])
		//   bits 1:0 = 00 -> bank 1 (brt[2]/brt[3])
		m_brt_r = 0x100 - ((m_brt[0] & 0xff00) >> 8);
		m_brt_g = 0x100 - ((m_brt[0] & 0x00ff) >> 0);
		m_brt_b = 0x100 - ((m_brt[1] & 0x00ff) >> 0);
		m_brt1_r = 0x100 - ((m_brt[2] & 0xff00) >> 8);
		m_brt1_g = 0x100 - ((m_brt[2] & 0x00ff) >> 0);
		m_brt1_b = 0x100 - ((m_brt[3] & 0x00ff) >> 0);
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

	/* TODO: 0 is correct for gametngk */
	m_temp_bitmap_tilemaps.fill(0, cliprect);   /* bg color */

	/* clear our sprite bitmaps */
	m_temp_bitmap_sprites.fill(0, cliprect);
	m_temp_bitmap_sprites_pri.fill(0, cliprect);

	draw_sprites(m_temp_bitmap_sprites, screen.priority(), cliprect, m_sprram_buffer.get());

	draw_tile_layers(screen, cliprect);

	mix_layers(screen, bitmap, cliprect);

	apply_sprite_effects(screen, bitmap, cliprect);

	return 0;
}

void ms32_state::draw_tile_layers(screen_device &screen, const rectangle &cliprect)
{
	m_layer_tx.fill(0xffff, cliprect);
	m_layer_bg.fill(0xffff, cliprect);
	m_layer_roz.fill(0xffff, cliprect);

	m_tx_tilemap->draw(screen, m_layer_tx, cliprect, 0, 0);
	bg_layer_tilemap()->draw(screen, m_layer_bg, cliprect, 0, 0);
	draw_roz(screen, m_layer_roz, cliprect, 0);
}

/*
    Per-pixel lookup in priority RAM, index layout:
    bit 12     sprite transparent
    bit 11     text transparent
    bit 10     unknown, always 1 on the games checked
    bit 9      ROZ transparent
    bit 8      road plane transparent (always 1 on games without it)
    bit 7      BG transparent
    bits 6-3   sprite priority (attribute bits 7-4)
    bits 2-0   line depth, colour bits 6-4 of the ROZ line

    Output: bits 5-3 select the layer (0 sprite, 1 BG, 2 ROZ, 4 road plane, 6 text), bit 6 selects the backdrop.
    TODO: bit 2 clear is approximated as half brightness, bits 1-0 are ignored
*/
void ms32_state::mix_layers(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t const *const paldata = m_palette->pens();

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		u16 const *const spr = &m_temp_bitmap_sprites.pix(y);
		u16 const *const tx  = &m_layer_tx.pix(y);
		u16 const *const bg  = &m_layer_bg.pix(y);
		u16 const *const roz = &m_layer_roz.pix(y);
		u32 *const dst = &bitmap.pix(y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			bool const s_op = (spr[x] & 0xff) != 0;
			u16 const pri = s_op ? (spr[x] >> 12) : 0;
			// TODO: derive depth from ROZ line colour bits 6-4 in super mode
			u16 const depth = 0;

			u16 const idx = (!s_op << 12)
					| ((tx[x] == 0xffff) << 11)
					| (1 << 10)
					| ((roz[x] == 0xffff) << 9)
					| (1 << 8) // road always transparent for non-f1superb
					| ((bg[x] == 0xffff) << 7)
					| (pri << 3)
					| depth;
			u8 const code = m_priram[idx];
			u8 const layer = (code >> 3) & 7;

			u16 pen = 0;
			if (!BIT(code, 6))
			{
				switch (layer)
				{
				case 0: pen = spr[x] & 0x0fff; break;
				case 1: pen = bg[x]; break;
				case 2: pen = roz[x]; break;
				case 6: pen = tx[x]; break;
				default: pen = 0; break;
				}
				if (pen == 0xffff)
					pen = 0;
			}

			rgb_t c = paldata[pen & 0x7fff];
			// TX text is unaffected by the second brightness bank. In gametngk,
			// the 30 Hz pulse belongs to sprite priority output 0x02 (cabinet glow).
			if ((code & 3) == 3)
				c = rgb_t(c.r() * m_brt_r / 0x100, c.g() * m_brt_g / 0x100, c.b() * m_brt_b / 0x100);
			else if (((code & 3) == 0 && layer != 6) || ((code & 3) == 2 && layer == 0))
				c = rgb_t(c.r() * m_brt1_r / 0x100, c.g() * m_brt1_g / 0x100, c.b() * m_brt1_b / 0x100);
			if (!BIT(code, 2))
			{
				if (layer == 0)  // sprite → glow
					c = alpha_blend_r32(c, 0x00ffffff, 128);
				else  // BG, ROZ, TX → shadow
					c = rgb_t(c.r() >> 1, c.g() >> 1, c.b() >> 1);
			}
			dst[x] = c;
		}
	}
}

void ms32_state::apply_sprite_effects(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_temp_bitmap_sprites_pri.fill(0, cliprect);
	screen.priority().fill(0, cliprect);

	const size_t sprite_tail = m_objectram_size - 8;
	u16 *source = m_sprram_buffer.get();
	u16 *finish = m_sprram_buffer.get() + sprite_tail;
	const bool reverseorder = (m_sprite_ctrl[0x10/4] & 0x8000) == 0x0000;
	if (reverseorder) { source = m_sprram_buffer.get() + sprite_tail; finish = m_sprram_buffer.get(); }
	for (;reverseorder ? (source>=finish) : (source<finish); reverseorder ? (source-=8) : (source+=8))
	{
		bool disable; u8 pri; bool flipx, flipy; u32 code, color;
		u8 tx, ty; u16 xsize, ysize; s32 sx, sy; u16 xzoom, yzoom;
		m_sprite->extract_parameters(source, disable, pri, flipx, flipy, code, color, tx, ty, xsize, ysize, sx, sy, xzoom, yzoom);
		if (disable || !xzoom || !yzoom) continue;
		m_sprite->prio_zoom_transpen_raw(m_temp_bitmap_sprites_pri,cliprect,
				code, (1 + pri) << 8, flipx, flipy, sx, sy,
				tx, ty, xsize, ysize, xzoom, yzoom,
				screen.priority(), 0, 0xffff);
	}

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		u16 const *const spr = &m_temp_bitmap_sprites.pix(y);
		u16 const *const cov = &m_temp_bitmap_sprites_pri.pix(y);
		u16 const *const tx_row = &m_layer_tx.pix(y);
		u16 const *const bg_row = &m_layer_bg.pix(y);
		u16 const *const roz_row = &m_layer_roz.pix(y);
		u32 *const dst = &bitmap.pix(y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			if (cov[x] == 0) continue;
			if (spr[x] & 0xff) continue;

			u16 const cov_pri = ((cov[x] >> 8) - 1) & 0xf;
			u16 const idx_with = (0 << 12)
					| ((tx_row[x] == 0xffff) << 11)
					| (1 << 10)
					| ((roz_row[x] == 0xffff) << 9)
					| (1 << 8)
					| ((bg_row[x] == 0xffff) << 7)
					| (cov_pri << 3);
			u8 const code_with = m_priram[idx_with];

			if (!BIT(code_with, 2))
			{
				u8 const layer = (code_with >> 3) & 7;
				rgb_t c(dst[x]);
				if (layer == 0)
				{
					if ((code_with & 3) == 2)
						c = rgb_t(c.r() * m_brt1_r / 0x100, c.g() * m_brt1_g / 0x100, c.b() * m_brt1_b / 0x100);
					c = alpha_blend_r32(c, 0x00ffffff, 128);
				}
				else
					c = rgb_t(c.r() >> 1, c.g() >> 1, c.b() >> 1);
				dst[x] = c;
			}
		}
	}
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
