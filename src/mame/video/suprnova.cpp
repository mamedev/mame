// license:BSD-3-Clause
// copyright-holders:David Haywood, Sylvain Glaize, Paul Priest, Olivier Galibert
/* Super Kaneko Nova System video */

#include "emu.h"
#include "video/sknsspr.h"
#include "includes/suprnova.h"


/* draws ROZ with linescroll OR columnscroll to 16-bit indexed bitmap */
void skns_state::suprnova_draw_roz(bitmap_ind16 &bitmap, bitmap_ind8& bitmapflags, const rectangle &cliprect, tilemap_t *tmap, UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy, int wraparound, int columnscroll, UINT32* scrollram)
{
	//bitmap_ind16 *destbitmap = bitmap;
	bitmap_ind16 &srcbitmap = tmap->pixmap();
	bitmap_ind8 &srcbitmapflags = tmap->flagsmap();
	const int xmask = srcbitmap.width()-1;
	const int ymask = srcbitmap.height()-1;
	const int widthshifted = srcbitmap.width() << 16;
	const int heightshifted = srcbitmap.height() << 16;
	UINT32 cx;
	UINT32 cy;
	int x;
	int sx;
	int sy;
	int ex;
	int ey;
	UINT16 *dest;
	UINT8* destflags;
//  UINT8 *pri;
	//const UINT16 *src;
	//const UINT8 *maskptr;
	//int destadvance = destbitmap->bpp / 8;

	/* pre-advance based on the cliprect */
	startx += cliprect.min_x * incxx + cliprect.min_y * incyx;
	starty += cliprect.min_x * incxy + cliprect.min_y * incyy;

	/* extract start/end points */
	sx = cliprect.min_x;
	sy = cliprect.min_y;
	ex = cliprect.max_x;
	ey = cliprect.max_y;

	{
		/* loop over rows */
		while (sy <= ey)
		{
			/* initialize X counters */
			x = sx;
			cx = startx;
			cy = starty;

			/* get dest and priority pointers */
			dest = &bitmap.pix16(sy, sx);
			destflags = &bitmapflags.pix8(sy, sx);

			/* loop over columns */
			while (x <= ex)
			{
				if ((wraparound) || (cx < widthshifted && cy < heightshifted)) // not sure how this will cope with no wraparound, but row/col scroll..
				{
					if (columnscroll)
					{
						dest[0] = srcbitmap.pix16(((cy >> 16) - scrollram[(cx>>16)&0x3ff]) & ymask, (cx >> 16) & xmask);
						destflags[0] = srcbitmapflags.pix8(((cy >> 16) - scrollram[(cx>>16)&0x3ff]) & ymask, (cx >> 16) & xmask);
					}
					else
					{
						dest[0] = srcbitmap.pix16((cy >> 16) & ymask, ((cx >> 16) - scrollram[(cy>>16)&0x3ff]) & xmask);
						destflags[0] = srcbitmapflags.pix8((cy >> 16) & ymask, ((cx >> 16) - scrollram[(cy>>16)&0x3ff]) & xmask);
					}
				}

				/* advance in X */
				cx += incxx;
				cy += incxy;
				x++;
				dest++;
				destflags++;
//              pri++;
			}

			/* advance in Y */
			startx += incyx;
			starty += incyy;
			sy++;
		}
	}
}


WRITE32_MEMBER(skns_state::skns_pal_regs_w)
{
	COMBINE_DATA(&m_pal_regs[offset]);
	m_palette_updated =1;

	switch ( offset )
	{
	/* RWRA regs are for SPRITES */

	case (0x00/4): // RWRA0
		if( m_use_spc_bright != (data&1) ) {
			m_use_spc_bright = data&1;
			m_spc_changed = 1;
		}
		m_alt_enable_sprites = (data>>8)&1;


		break;
	case (0x04/4): // RWRA1
		if( m_bright_spc_g != (data&0xff) ) {
			m_bright_spc_g = data&0xff;
			m_spc_changed = 1;
		}
		m_bright_spc_g_trans = (data>>8) &0xff;


		break;
	case (0x08/4): // RWRA2
		if( m_bright_spc_r != (data&0xff) ) {
			m_bright_spc_r = data&0xff;
			m_spc_changed = 1;
		}
		m_bright_spc_r_trans = (data>>8) &0xff;

		break;
	case (0x0C/4): // RWRA3
		if( m_bright_spc_b != (data&0xff) ) {
			m_bright_spc_b = data&0xff;
			m_spc_changed = 1;
		}
		m_bright_spc_b_trans = (data>>8)&0xff;


		break;

	/* RWRB regs are for BACKGROUND */

	case (0x10/4): // RWRB0
		if( m_use_v3_bright != (data&1) ) {
			m_use_v3_bright = data&1;
			m_v3_changed = 1;
		}

		m_alt_enable_background = (data>>8)&1;

		break;
	case (0x14/4): // RWRB1
		if( m_bright_v3_g != (data&0xff) ) {
			m_bright_v3_g = data&0xff;
			m_v3_changed = 1;
		}

		m_bright_v3_g_trans = (data>>8)&0xff;

		break;
	case (0x18/4): // RWRB2
		if( m_bright_v3_r != (data&0xff) ) {
			m_bright_v3_r = data&0xff;
			m_v3_changed = 1;
		}

		m_bright_v3_r_trans = (data>>8)&0xff;

		break;
	case (0x1C/4): // RWRB3
		if( m_bright_v3_b != (data&0xff) ) {
			m_bright_v3_b = data&0xff;
			m_v3_changed = 1;
		}

		m_bright_v3_b_trans = (data>>8)&0xff;

		break;
	}
}


WRITE32_MEMBER(skns_state::skns_palette_ram_w)
{
	int r,g,b;
	int brightness_r, brightness_g, brightness_b/*, alpha*/;
	int use_bright;

	COMBINE_DATA(&m_palette_ram[offset]);

	b = ((m_palette_ram[offset] >> 0  ) & 0x1f);
	g = ((m_palette_ram[offset] >> 5  ) & 0x1f);
	r = ((m_palette_ram[offset] >> 10  ) & 0x1f);

	//alpha = ((m_palette_ram[offset] >> 15  ) & 0x1);

	if(offset<(0x40*256)) { // 1st half is for Sprites
		use_bright = m_use_spc_bright;
		brightness_b = m_bright_spc_b;
		brightness_g = m_bright_spc_g;
		brightness_r = m_bright_spc_r;
	} else { // V3 bg's
		use_bright = m_use_v3_bright;
		brightness_b = m_bright_v3_b;
		brightness_g = m_bright_v3_g;
		brightness_r = m_bright_v3_r;
	}

	if(use_bright) {
		if(brightness_b) b = ((b<<3) * (brightness_b+1))>>8;
		else b = 0;
		if(brightness_g) g = ((g<<3) * (brightness_g+1))>>8;
		else g = 0;
		if(brightness_r) r = ((r<<3) * (brightness_r+1))>>8;
		else r = 0;
	} else {
		b <<= 3;
		g <<= 3;
		r <<= 3;
	}

	m_palette->set_pen_color(offset,rgb_t(r,g,b));
}


void skns_state::palette_set_rgb_brightness (int offset, UINT8 brightness_r, UINT8 brightness_g, UINT8 brightness_b)
{
	int use_bright, r, g, b/*, alpha*/;

	b = ((m_palette_ram[offset] >> 0  ) & 0x1f);
	g = ((m_palette_ram[offset] >> 5  ) & 0x1f);
	r = ((m_palette_ram[offset] >> 10  ) & 0x1f);

	//alpha = ((m_palette_ram[offset] >> 15  ) & 0x1);

	if(offset<(0x40*256)) { // 1st half is for Sprites
		use_bright = m_use_spc_bright;
	} else { // V3 bg's
		use_bright = m_use_v3_bright;
	}

	if(use_bright) {
		if(brightness_b) b = ((b<<3) * (brightness_b+1))>>8;
		else b = 0;
		if(brightness_g) g = ((g<<3) * (brightness_g+1))>>8;
		else g = 0;
		if(brightness_r) r = ((r<<3) * (brightness_r+1))>>8;
		else r = 0;
	} else {
		b <<= 3;
		g <<= 3;
		r <<= 3;
	}

	m_palette->set_pen_color(offset,rgb_t(r,g,b));
}


void skns_state::palette_update()
{
	int i;

	if (m_palette_updated)
	{
		if(m_spc_changed)
			for(i=0; i<=((0x40*256)-1); i++)
				palette_set_rgb_brightness (i, m_bright_spc_r, m_bright_spc_g, m_bright_spc_b);

		if(m_v3_changed)
			for(i=(0x40*256); i<=((0x80*256)-1); i++)
				palette_set_rgb_brightness (i, m_bright_v3_r, m_bright_v3_g, m_bright_v3_b);
		m_palette_updated =0;
	}
}



TILE_GET_INFO_MEMBER(skns_state::get_tilemap_A_tile_info)
{
	int code = ((m_tilemapA_ram[tile_index] & 0x001fffff) >> 0 );
	int colr = ((m_tilemapA_ram[tile_index] & 0x3f000000) >> 24 );
	int pri  = ((m_tilemapA_ram[tile_index] & 0x00e00000) >> 21 );
	int depth = (m_v3_regs[0x0c/4] & 0x0001) << 1;
	int flags = 0;

	if(m_tilemapA_ram[tile_index] & 0x80000000) flags |= TILE_FLIPX;
	if(m_tilemapA_ram[tile_index] & 0x40000000) flags |= TILE_FLIPY;

	SET_TILE_INFO_MEMBER(0+depth,
			code,
			0x40+colr,
			flags);
	tileinfo.category = pri;

	//if (pri) popmessage("pri A!! %02x\n", pri);
}

WRITE32_MEMBER(skns_state::skns_tilemapA_w)
{
	COMBINE_DATA(&m_tilemapA_ram[offset]);
	m_tilemap_A->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(skns_state::get_tilemap_B_tile_info)
{
	int code = ((m_tilemapB_ram[tile_index] & 0x001fffff) >> 0 );
	int colr = ((m_tilemapB_ram[tile_index] & 0x3f000000) >> 24 );
	int pri  = ((m_tilemapB_ram[tile_index] & 0x00e00000) >> 21 );
	int depth = (m_v3_regs[0x0c/4] & 0x0100) >> 7;
	int flags = 0;

	if(m_tilemapB_ram[tile_index] & 0x80000000) flags |= TILE_FLIPX;
	if(m_tilemapB_ram[tile_index] & 0x40000000) flags |= TILE_FLIPY;

	SET_TILE_INFO_MEMBER(1+depth,
			code,
			0x40+colr,
			flags);
	tileinfo.category = pri;

	//if (pri) popmessage("pri B!! %02x\n", pri); // 02 on cyvern
}

WRITE32_MEMBER(skns_state::skns_tilemapB_w)
{
	COMBINE_DATA(&m_tilemapB_ram[offset]);
	m_tilemap_B->mark_tile_dirty(offset);
}

WRITE32_MEMBER(skns_state::skns_v3_regs_w)
{
	COMBINE_DATA(&m_v3_regs[offset]);

	/* if the depth changes we need to dirty the tilemap */
	if (offset == 0x0c/4)
	{
		int old_depthA = m_depthA;
		int old_depthB = m_depthB;

		m_depthA = (m_v3_regs[0x0c/4] & 0x0001) << 1;
		m_depthB = (m_v3_regs[0x0c/4] & 0x0100) >> 7;

		if (old_depthA != m_depthA) m_tilemap_A->mark_all_dirty();
		if (old_depthB != m_depthB) m_tilemap_B->mark_all_dirty();

	}
}


void skns_state::video_start()
{
	m_tilemap_A = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(skns_state::get_tilemap_A_tile_info),this),TILEMAP_SCAN_ROWS,16,16,64, 64);
		m_tilemap_A->set_transparent_pen(0);

	m_tilemap_B = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(skns_state::get_tilemap_B_tile_info),this),TILEMAP_SCAN_ROWS,16,16,64, 64);
		m_tilemap_B->set_transparent_pen(0);

	m_sprite_bitmap.allocate(1024,1024);

	m_tilemap_bitmap_lower.allocate(320,240);
	m_tilemap_bitmapflags_lower.allocate(320,240);

	m_tilemap_bitmap_higher.allocate(320,240);
	m_tilemap_bitmapflags_higher.allocate(320,240);

	m_gfxdecode->gfx(2)->set_granularity(256);
	m_gfxdecode->gfx(3)->set_granularity(256);
}

void skns_state::video_reset()
{
	m_depthA = m_depthB = 0;
	m_use_spc_bright = m_use_v3_bright = 1;
	m_bright_spc_b= m_bright_spc_g = m_bright_spc_r = 0x00;
	m_bright_spc_b_trans = m_bright_spc_g_trans = m_bright_spc_r_trans = 0x00;
	m_bright_v3_b = m_bright_v3_g = m_bright_v3_r = 0x00;
	m_bright_v3_b_trans = m_bright_v3_g_trans = m_bright_v3_r_trans = 0x00;

	m_spc_changed = m_v3_changed = m_palette_updated = 0;
	m_alt_enable_background = m_alt_enable_sprites = 1;
}

void skns_state::supernova_draw_a( bitmap_ind16 &bitmap, bitmap_ind8 &bitmap_flags, const rectangle &cliprect, int tran )
{
	int enable_a  = (m_v3_regs[0x10/4] >> 0) & 0x0001;
	int nowrap_a = (m_v3_regs[0x10/4] >> 0) & 0x0004;


	UINT32 startx,starty;
	int incxx,incxy,incyx,incyy;
	int columnscroll;

	//if(nowrap_a) printf("a\n");

	if (enable_a && m_alt_enable_background)
	{
		startx = m_v3_regs[0x1c/4];
		incyy  = m_v3_regs[0x30/4]&0x7ffff;
		if (incyy&0x40000) incyy = incyy-0x80000; // level 3 boss in sengekis
		incyx  = m_v3_regs[0x2c/4];
		starty = m_v3_regs[0x20/4];
		incxy  = m_v3_regs[0x28/4];
		incxx  = m_v3_regs[0x24/4]&0x7ffff;
		if (incxx&0x40000) incxx = incxx-0x80000;

		columnscroll = (m_v3_regs[0x0c/4] >> 1) & 0x0001;

		suprnova_draw_roz(bitmap,bitmap_flags,cliprect, m_tilemap_A, startx << 8,starty << 8,    incxx << 8,incxy << 8,incyx << 8,incyy << 8, !nowrap_a, columnscroll, &m_v3slc_ram[0]);
		//tilemap_copy_bitmap(bitmap, m_tilemap_bitmap_lower, m_tilemap_bitmapflags_lower);
	}
}

void skns_state::supernova_draw_b( bitmap_ind16 &bitmap, bitmap_ind8 &bitmap_flags, const rectangle &cliprect, int tran )
{
	int enable_b  = (m_v3_regs[0x34/4] >> 0) & 0x0001;
	int nowrap_b = (m_v3_regs[0x34/4] >> 0) & 0x0004;


	UINT32 startx,starty;
	int incxx,incxy,incyx,incyy;
	int columnscroll;

	//if(nowrap_b) printf("b\n");

	if (enable_b && m_alt_enable_background)
	{
		startx = m_v3_regs[0x40/4];
		incyy  = m_v3_regs[0x54/4]&0x7ffff;
		if (incyy&0x40000) incyy = incyy-0x80000;
		incyx  = m_v3_regs[0x50/4];
		starty = m_v3_regs[0x44/4];
		incxy  = m_v3_regs[0x4c/4];
		incxx  = m_v3_regs[0x48/4]&0x7ffff;
		if (incxx&0x40000) incxx = incxx-0x80000;
		columnscroll = (m_v3_regs[0x0c/4] >> 9) & 0x0001; // selects column scroll or rowscroll
		suprnova_draw_roz(bitmap,bitmap_flags, cliprect, m_tilemap_B, startx << 8,starty << 8,   incxx << 8,incxy << 8,incyx << 8,incyy << 8, !nowrap_b, columnscroll, &m_v3slc_ram[0x1000/4]);

		//popmessage("%08x %08x %08x %08x %08x %08x", startx, starty, incxx, incyy, incxy, incyx);

	}
}

UINT32 skns_state::screen_update_skns(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	palette_update();

	bitmap.fill(m_palette->black_pen(), cliprect);
	m_tilemap_bitmap_lower.fill(0);
	m_tilemap_bitmapflags_lower.fill(0);
	m_tilemap_bitmap_higher.fill(0);
	m_tilemap_bitmapflags_higher.fill(0);

	{
		int supernova_pri_a;
		int supernova_pri_b;
		int tran = 0;

		supernova_pri_a = (m_v3_regs[0x10/4] & 0x0002)>>1;
		supernova_pri_b = (m_v3_regs[0x34/4] & 0x0002)>>1;

		//popmessage("pri %d %d\n", supernova_pri_a, supernova_pri_b);

		/*if (!supernova_pri_b) { */
		supernova_draw_b(m_tilemap_bitmap_lower, m_tilemap_bitmapflags_lower, cliprect,tran);// tran = 1;
		supernova_draw_a(m_tilemap_bitmap_higher,m_tilemap_bitmapflags_higher,cliprect,tran);// tran = 1;

		{
			int x,y;
			UINT8* srcflags, *src2flags;
			UINT16* src, *src2, *src3;
			UINT32* dst;
			UINT16 pri, pri2, pri3;
			UINT16 bgpri;
			const pen_t *clut = &m_palette->pen(0);
//          int drawpri;


			for (y=0;y<240;y++)
			{
				src = &m_tilemap_bitmap_lower.pix16(y);
				srcflags = &m_tilemap_bitmapflags_lower.pix8(y);

				src2 = &m_tilemap_bitmap_higher.pix16(y);
				src2flags = &m_tilemap_bitmapflags_higher.pix8(y);

				src3 = &m_sprite_bitmap.pix16(y);

				dst = &bitmap.pix32(y);


				for (x=0;x<320;x++)
				{
					UINT16 pendata  = src[x]&0x7fff;
					UINT16 pendata2 = src2[x]&0x7fff;
					UINT16 bgpendata;
					UINT16 pendata3 = src3[x]&0x3fff;

					UINT32 coldat;

					pri = ((srcflags[x] & 0x07)<<1) | (supernova_pri_b);
					pri2= ((src2flags[x] & 0x07)<<1) | (supernova_pri_a);
					pri3 = ((src3[x]&0xc000)>>12)+3;

					// work out which layers bg pixel has the higher priority
					//  note, can the bg layers be blended?? sarukani uses an alpha pen for
					//        some of them.. and registers indicate it might be possible..

					// this priority mixing is almost certainly still incorrect
					// bg colour / prioirty handling is now wrong

					if (pri<=pri2) // <= is good for last level of cyvern.. < seem better for galpanis kaneko logo
					{
						if (pendata2&0xff)
						{
							bgpendata = pendata2&0x7fff;
							bgpri = pri2;
						}
						else if (pendata&0xff)
						{
							bgpendata = pendata&0x7fff;
							bgpri = pri;
						}
						else
						{
							bgpendata = pendata2&0x7fff;
							bgpri = 0;;
						}
					}
					else
					{
						if (pendata&0xff)
						{
							bgpendata = pendata&0x7fff;
							bgpri = pri;
						}
						else if (pendata2&0xff)
						{
							bgpendata = pendata2&0x7fff;
							bgpri = pri2;
						}
						else
						{
							bgpendata = 0;
							bgpri = 0;;
						}
					}

					// if the sprites are higher than the bg pixel
					if (pri3 > bgpri)
					{
						if (pendata3&0xff)
						{
							UINT16 palvalue = m_palette_ram[pendata3];

							coldat = clut[pendata3];

							if (palvalue&0x8000)
							{
								UINT32 srccolour = clut[bgpendata&0x7fff];
								UINT32 dstcolour = clut[pendata3&0x3fff];

								int r,g,b;
								int r2,g2,b2;

								r = (srccolour & 0x000000ff)>> 0;
								g = (srccolour & 0x0000ff00)>> 8;
								b = (srccolour & 0x00ff0000)>> 16;

								r2 = (dstcolour & 0x000000ff)>> 0;
								g2 = (dstcolour & 0x0000ff00)>> 8;
								b2 = (dstcolour & 0x00ff0000)>> 16;

								r2 = (r2 * m_bright_spc_r_trans) >> 8;
								g2 = (g2 * m_bright_spc_g_trans) >> 8;
								b2 = (b2 * m_bright_spc_b_trans) >> 8;

								r = (r+r2);
								if (r>255) r = 255;

								g = (g+g2);
								if (g>255) g = 255;

								b = (b+b2);
								if (b>255) b = 255;

								dst[x] = (r << 0) | (g << 8) | (b << 16);


							}

							else
							{
								coldat = clut[pendata3];
								dst[x] = coldat;
							}
						}
						else
						{
							coldat = clut[bgpendata];
							dst[x] = coldat;
						}
					}
					else
					{
						coldat = clut[bgpendata];
						dst[x] = coldat;
					}

				}
			}
		}
	}

	m_sprite_bitmap.fill(0x0000, cliprect);

	if (m_alt_enable_sprites)
		m_spritegen->skns_draw_sprites(m_sprite_bitmap, cliprect, m_spriteram, m_spriteram.bytes(), memregion("gfx1")->base(), memregion ("gfx1")->bytes(), m_spc_regs );


	return 0;
}

void skns_state::screen_eof_skns(screen_device &screen, bool state)
{
}
