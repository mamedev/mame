/* Super Kaneko Nova System video */

#include "emu.h"
#include "video/sknsspr.h"
#include "includes/suprnova.h"

static bitmap_t *sprite_bitmap;

static bitmap_t *tilemap_bitmap_lower;
static bitmap_t *tilemap_bitmapflags_lower;

static bitmap_t *tilemap_bitmap_higher;
static bitmap_t *tilemap_bitmapflags_higher;


/* draws ROZ with linescroll OR columnscroll to 16-bit indexed bitmap */
static void suprnova_draw_roz(bitmap_t* bitmap, bitmap_t* bitmapflags, const rectangle *cliprect, tilemap_t *tmap, UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy, int wraparound, int columnscroll, UINT32* scrollram)
{
	//bitmap_t *destbitmap = bitmap;
	bitmap_t *srcbitmap = tilemap_get_pixmap(tmap);
	bitmap_t *srcbitmapflags = tilemap_get_flagsmap(tmap);
	const int xmask = srcbitmap->width-1;
	const int ymask = srcbitmap->height-1;
	const int widthshifted = srcbitmap->width << 16;
	const int heightshifted = srcbitmap->height << 16;
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
	startx += cliprect->min_x * incxx + cliprect->min_y * incyx;
	starty += cliprect->min_x * incxy + cliprect->min_y * incyy;

	/* extract start/end points */
	sx = cliprect->min_x;
	sy = cliprect->min_y;
	ex = cliprect->max_x;
	ey = cliprect->max_y;

	{
		/* loop over rows */
		while (sy <= ey)
		{

			/* initialize X counters */
			x = sx;
			cx = startx;
			cy = starty;

			/* get dest and priority pointers */
			dest = BITMAP_ADDR16( bitmap, sy, sx);
			destflags = BITMAP_ADDR8( bitmapflags, sy, sx);

			/* loop over columns */
			while (x <= ex)
			{
				if ((wraparound) || (cx < widthshifted && cy < heightshifted)) // not sure how this will cope with no wraparound, but row/col scroll..
				{
					if (columnscroll)
					{
						dest[0] = BITMAP_ADDR16(srcbitmap, ((cy >> 16) - scrollram[(cx>>16)&0x3ff]) & ymask, (cx >> 16) & xmask)[0];
						destflags[0] = BITMAP_ADDR8(srcbitmapflags, ((cy >> 16) - scrollram[(cx>>16)&0x3ff]) & ymask, (cx >> 16) & xmask)[0];
					}
					else
					{
						dest[0] = BITMAP_ADDR16(srcbitmap, (cy >> 16) & ymask, ((cx >> 16) - scrollram[(cy>>16)&0x3ff]) & xmask)[0];
						destflags[0] = BITMAP_ADDR8(srcbitmapflags, (cy >> 16) & ymask, ((cx >> 16) - scrollram[(cy>>16)&0x3ff]) & xmask)[0];
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


static int depthA=0;
static int depthB=0;


static int use_spc_bright, use_v3_bright; // makes sarukani rather dark, but should be default..
static UINT8 bright_spc_b=0x00, bright_spc_g=0x00, bright_spc_r=0x00;

static UINT8 bright_spc_b_trans=0x00, bright_spc_g_trans=0x00, bright_spc_r_trans=0x00;


static UINT8 bright_v3_b=0x00,  bright_v3_g=0x00,  bright_v3_r=0x00;
static UINT8 bright_v3_b_trans = 0x00, bright_v3_g_trans = 0x00, bright_v3_r_trans = 0x00;

// This ignores the alpha values atm.
static int spc_changed=0, v3_changed=0, palette_updated=0;
static int suprnova_alt_enable_background;
int suprnova_alt_enable_sprites;

WRITE32_HANDLER ( skns_pal_regs_w )
{
	COMBINE_DATA(&skns_pal_regs[offset]);
	palette_updated =1;

	switch ( offset )
	{
	/* RWRA regs are for SPRITES */

	case (0x00/4): // RWRA0
		if( use_spc_bright != (data&1) ) {
			use_spc_bright = data&1;
			spc_changed = 1;
		}
		suprnova_alt_enable_sprites = (data>>8)&1;


		break;
	case (0x04/4): // RWRA1
		if( bright_spc_g != (data&0xff) ) {
			bright_spc_g = data&0xff;
			spc_changed = 1;
		}
		bright_spc_g_trans = (data>>8) &0xff;


		break;
	case (0x08/4): // RWRA2
		if( bright_spc_r != (data&0xff) ) {
			bright_spc_r = data&0xff;
			spc_changed = 1;
		}
		bright_spc_r_trans = (data>>8) &0xff;

		break;
	case (0x0C/4): // RWRA3
		if( bright_spc_b != (data&0xff) ) {
			bright_spc_b = data&0xff;
			spc_changed = 1;
		}
		bright_spc_b_trans = (data>>8)&0xff;


		break;

	/* RWRB regs are for BACKGROUND */

	case (0x10/4): // RWRB0
		if( use_v3_bright != (data&1) ) {
			use_v3_bright = data&1;
			v3_changed = 1;
		}

		suprnova_alt_enable_background = (data>>8)&1;

		break;
	case (0x14/4): // RWRB1
		if( bright_v3_g != (data&0xff) ) {
			bright_v3_g = data&0xff;
			v3_changed = 1;
		}

		bright_v3_g_trans = (data>>8)&0xff;

		break;
	case (0x18/4): // RWRB2
		if( bright_v3_r != (data&0xff) ) {
			bright_v3_r = data&0xff;
			v3_changed = 1;
		}

		bright_v3_r_trans = (data>>8)&0xff;

		break;
	case (0x1C/4): // RWRB3
		if( bright_v3_b != (data&0xff) ) {
			bright_v3_b = data&0xff;
			v3_changed = 1;
		}

		bright_v3_b_trans = (data>>8)&0xff;

		break;
	}
}


WRITE32_HANDLER ( skns_palette_ram_w )
{
	int r,g,b;
	int brightness_r, brightness_g, brightness_b/*, alpha*/;
	int use_bright;

	COMBINE_DATA(&skns_palette_ram[offset]);

	b = ((skns_palette_ram[offset] >> 0  ) & 0x1f);
	g = ((skns_palette_ram[offset] >> 5  ) & 0x1f);
	r = ((skns_palette_ram[offset] >> 10  ) & 0x1f);

	//alpha = ((skns_palette_ram[offset] >> 15  ) & 0x1);

	if(offset<(0x40*256)) { // 1st half is for Sprites
		use_bright = use_spc_bright;
		brightness_b = bright_spc_b;
		brightness_g = bright_spc_g;
		brightness_r = bright_spc_r;
	} else { // V3 bg's
		use_bright = use_v3_bright;
		brightness_b = bright_v3_b;
		brightness_g = bright_v3_g;
		brightness_r = bright_v3_r;
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

	palette_set_color(space->machine(),offset,MAKE_RGB(r,g,b));
}


static void palette_set_rgb_brightness (running_machine &machine, int offset, UINT8 brightness_r, UINT8 brightness_g, UINT8 brightness_b)
{
	int use_bright, r, g, b/*, alpha*/;

	b = ((skns_palette_ram[offset] >> 0  ) & 0x1f);
	g = ((skns_palette_ram[offset] >> 5  ) & 0x1f);
	r = ((skns_palette_ram[offset] >> 10  ) & 0x1f);

	//alpha = ((skns_palette_ram[offset] >> 15  ) & 0x1);

	if(offset<(0x40*256)) { // 1st half is for Sprites
		use_bright = use_spc_bright;
	} else { // V3 bg's
		use_bright = use_v3_bright;
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

	palette_set_color(machine,offset,MAKE_RGB(r,g,b));
}


static void palette_update(running_machine &machine)
{
	int i;

	if (palette_updated)
	{
		if(spc_changed)
			for(i=0; i<=((0x40*256)-1); i++)
				palette_set_rgb_brightness (machine, i, bright_spc_r, bright_spc_g, bright_spc_b);

		if(v3_changed)
			for(i=(0x40*256); i<=((0x80*256)-1); i++)
				palette_set_rgb_brightness (machine, i, bright_v3_r, bright_v3_g, bright_v3_b);
		palette_updated =0;
	}
}


static tilemap_t *skns_tilemap_A;
static tilemap_t *skns_tilemap_B;

static TILE_GET_INFO( get_tilemap_A_tile_info )
{
	int code = ((skns_tilemapA_ram[tile_index] & 0x001fffff) >> 0 );
	int colr = ((skns_tilemapA_ram[tile_index] & 0x3f000000) >> 24 );
	int pri  = ((skns_tilemapA_ram[tile_index] & 0x00e00000) >> 21 );
	int depth = (skns_v3_regs[0x0c/4] & 0x0001) << 1;
	int flags = 0;

	if(skns_tilemapA_ram[tile_index] & 0x80000000) flags |= TILE_FLIPX;
	if(skns_tilemapA_ram[tile_index] & 0x40000000) flags |= TILE_FLIPY;

	SET_TILE_INFO(
			0+depth,
			code,
			0x40+colr,
			flags);
	tileinfo->category = pri;

	//if (pri) popmessage("pri A!! %02x\n", pri);
}

WRITE32_HANDLER ( skns_tilemapA_w )
{
	COMBINE_DATA(&skns_tilemapA_ram[offset]);
	tilemap_mark_tile_dirty(skns_tilemap_A,offset);
}

static TILE_GET_INFO( get_tilemap_B_tile_info )
{
	int code = ((skns_tilemapB_ram[tile_index] & 0x001fffff) >> 0 );
	int colr = ((skns_tilemapB_ram[tile_index] & 0x3f000000) >> 24 );
	int pri  = ((skns_tilemapB_ram[tile_index] & 0x00e00000) >> 21 );
	int depth = (skns_v3_regs[0x0c/4] & 0x0100) >> 7;
	int flags = 0;

	if(skns_tilemapB_ram[tile_index] & 0x80000000) flags |= TILE_FLIPX;
	if(skns_tilemapB_ram[tile_index] & 0x40000000) flags |= TILE_FLIPY;

	SET_TILE_INFO(
			1+depth,
			code,
			0x40+colr,
			flags);
	tileinfo->category = pri;

	//if (pri) popmessage("pri B!! %02x\n", pri); // 02 on cyvern
}

WRITE32_HANDLER ( skns_tilemapB_w )
{
	COMBINE_DATA(&skns_tilemapB_ram[offset]);
	tilemap_mark_tile_dirty(skns_tilemap_B,offset);
}

WRITE32_HANDLER ( skns_v3_regs_w )
{
	COMBINE_DATA(&skns_v3_regs[offset]);

	/* if the depth changes we need to dirty the tilemap */
	if (offset == 0x0c/4)
	{
		int old_depthA = depthA;
		int old_depthB = depthB;

		depthA = (skns_v3_regs[0x0c/4] & 0x0001) << 1;
		depthB = (skns_v3_regs[0x0c/4] & 0x0100) >> 7;

		if (old_depthA != depthA)	tilemap_mark_all_tiles_dirty (skns_tilemap_A);
		if (old_depthB != depthB)	tilemap_mark_all_tiles_dirty (skns_tilemap_B);

	}
}


VIDEO_START(skns)
{
	skns_state *state = machine.driver_data<skns_state>();

	state->m_spritegen = machine.device<sknsspr_device>("spritegen");

	skns_tilemap_A = tilemap_create(machine, get_tilemap_A_tile_info,tilemap_scan_rows,16,16,64, 64);
		tilemap_set_transparent_pen(skns_tilemap_A,0);

	skns_tilemap_B = tilemap_create(machine, get_tilemap_B_tile_info,tilemap_scan_rows,16,16,64, 64);
		tilemap_set_transparent_pen(skns_tilemap_B,0);

	sprite_bitmap = auto_bitmap_alloc(machine,1024,1024,BITMAP_FORMAT_INDEXED16);

	tilemap_bitmap_lower = auto_bitmap_alloc(machine,320,240,BITMAP_FORMAT_INDEXED16);
	tilemap_bitmapflags_lower = auto_bitmap_alloc(machine,320,240,BITMAP_FORMAT_INDEXED8);

	tilemap_bitmap_higher = auto_bitmap_alloc(machine,320,240,BITMAP_FORMAT_INDEXED16);
	tilemap_bitmapflags_higher = auto_bitmap_alloc(machine,320,240,BITMAP_FORMAT_INDEXED8);

	machine.gfx[2]->color_granularity=256;
	machine.gfx[3]->color_granularity=256;
}

VIDEO_RESET( skns )
{
	depthA = depthB = 0;
	use_spc_bright = use_v3_bright = 1;
	bright_spc_b= bright_spc_g = bright_spc_r = 0x00;
	bright_spc_b_trans = bright_spc_g_trans = bright_spc_r_trans = 0x00;
	bright_v3_b = bright_v3_g = bright_v3_r = 0x00;
	bright_v3_b_trans = bright_v3_g_trans = bright_v3_r_trans = 0x00;

	spc_changed = v3_changed = palette_updated = 0;
	suprnova_alt_enable_background = suprnova_alt_enable_sprites = 1;
}

static void supernova_draw_a( bitmap_t *bitmap, bitmap_t* bitmap_flags, const rectangle *cliprect, int tran )
{
	int enable_a  = (skns_v3_regs[0x10/4] >> 0) & 0x0001;
	int nowrap_a = (skns_v3_regs[0x10/4] >> 0) & 0x0004;


	UINT32 startx,starty;
	int incxx,incxy,incyx,incyy;
	int columnscroll;

	//if(nowrap_a) printf("a\n");

	if (enable_a && suprnova_alt_enable_background)
	{
		startx = skns_v3_regs[0x1c/4];
		incyy  = skns_v3_regs[0x30/4]&0x7ffff;
		if (incyy&0x40000) incyy = incyy-0x80000; // level 3 boss in sengekis
		incyx  = skns_v3_regs[0x2c/4];
		starty = skns_v3_regs[0x20/4];
		incxy  = skns_v3_regs[0x28/4];
		incxx  = skns_v3_regs[0x24/4]&0x7ffff;
		if (incxx&0x40000) incxx = incxx-0x80000;

		columnscroll = (skns_v3_regs[0x0c/4] >> 1) & 0x0001;

		suprnova_draw_roz(bitmap,bitmap_flags,cliprect, skns_tilemap_A, startx << 8,starty << 8,	incxx << 8,incxy << 8,incyx << 8,incyy << 8, !nowrap_a, columnscroll, &skns_v3slc_ram[0]);
		//tilemap_copy_bitmap(bitmap, tilemap_bitmap_lower, tilemap_bitmapflags_lower);
	}
}

static void supernova_draw_b( bitmap_t *bitmap, bitmap_t* bitmap_flags, const rectangle *cliprect, int tran )
{
	int enable_b  = (skns_v3_regs[0x34/4] >> 0) & 0x0001;
	int nowrap_b = (skns_v3_regs[0x34/4] >> 0) & 0x0004;


	UINT32 startx,starty;
	int incxx,incxy,incyx,incyy;
	int columnscroll;

	//if(nowrap_b) printf("b\n");

	if (enable_b && suprnova_alt_enable_background)
	{
		startx = skns_v3_regs[0x40/4];
		incyy  = skns_v3_regs[0x54/4]&0x7ffff;
		if (incyy&0x40000) incyy = incyy-0x80000;
		incyx  = skns_v3_regs[0x50/4];
		starty = skns_v3_regs[0x44/4];
		incxy  = skns_v3_regs[0x4c/4];
		incxx  = skns_v3_regs[0x48/4]&0x7ffff;
		if (incxx&0x40000) incxx = incxx-0x80000;
		columnscroll = (skns_v3_regs[0x0c/4] >> 9) & 0x0001; // selects column scroll or rowscroll
		suprnova_draw_roz(bitmap,bitmap_flags, cliprect, skns_tilemap_B, startx << 8,starty << 8,	incxx << 8,incxy << 8,incyx << 8,incyy << 8, !nowrap_b, columnscroll, &skns_v3slc_ram[0x1000/4]);

		//popmessage("%08x %08x %08x %08x %08x %08x", startx, starty, incxx, incyy, incxy, incyx);

	}
}

SCREEN_UPDATE(skns)
{
	skns_state *state = screen->machine().driver_data<skns_state>();

	palette_update(screen->machine());

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine()));
	bitmap_fill(tilemap_bitmap_lower, NULL, 0);
	bitmap_fill(tilemap_bitmapflags_lower, NULL, 0);
	bitmap_fill(tilemap_bitmap_higher, NULL, 0);
	bitmap_fill(tilemap_bitmapflags_higher, NULL, 0);

	{
		int supernova_pri_a;
		int supernova_pri_b;
		int tran = 0;

		supernova_pri_a = (skns_v3_regs[0x10/4] & 0x0002)>>1;
		supernova_pri_b = (skns_v3_regs[0x34/4] & 0x0002)>>1;

		//popmessage("pri %d %d\n", supernova_pri_a, supernova_pri_b);

		/*if (!supernova_pri_b) { */
		supernova_draw_b(tilemap_bitmap_lower, tilemap_bitmapflags_lower, cliprect,tran);// tran = 1;
		supernova_draw_a(tilemap_bitmap_higher,tilemap_bitmapflags_higher,cliprect,tran);// tran = 1;

		{
			int x,y;
			UINT8* srcflags, *src2flags;
			UINT16* src, *src2, *src3;
			UINT32* dst;
			UINT16 pri, pri2, pri3;
			UINT16 bgpri;
			const pen_t *clut = &screen->machine().pens[0];
//          int drawpri;


			for (y=0;y<240;y++)
			{
				src = BITMAP_ADDR16(tilemap_bitmap_lower, y, 0);
				srcflags = BITMAP_ADDR8(tilemap_bitmapflags_lower, y, 0);

				src2 = BITMAP_ADDR16(tilemap_bitmap_higher, y, 0);
				src2flags = BITMAP_ADDR8(tilemap_bitmapflags_higher, y, 0);

				src3 = BITMAP_ADDR16(sprite_bitmap, y, 0);

				dst = BITMAP_ADDR32(bitmap, y, 0);


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

							UINT16 palvalue = skns_palette_ram[pendata3];

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

								r2 = (r2 * bright_spc_r_trans) >> 8;
								g2 = (g2 * bright_spc_g_trans) >> 8;
								b2 = (b2 * bright_spc_b_trans) >> 8;

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

	bitmap_fill(sprite_bitmap, cliprect, 0x0000);

	if (suprnova_alt_enable_sprites)
		state->m_spritegen->skns_draw_sprites(screen->machine(), sprite_bitmap, cliprect, screen->machine().generic.spriteram.u32, screen->machine().generic.spriteram_size, screen->machine().region("gfx1")->base(), screen->machine().region ("gfx1")->bytes(), skns_spc_regs );


	return 0;
}

SCREEN_EOF(skns)
{

}
