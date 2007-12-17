/*
  Sega system24 hardware

System 24      68000x2  315-5292   315-5293  315-5294  315-5242        ym2151 dac           315-5195(x3) 315-5296(IO)

  System24:
    The odd one out.  Medium resolution. Entirely ram-based, no
    graphics roms.  4-layer tilemap hardware in two pairs, selection
    on a 8-pixels basis.  Tile-based sprites(!) organised as a linked
    list.  The tilemap chip has been reused for model1 and model2,
    probably because they had it handy and it handles medium res.

*/

/*
 * System 16 color palette formats
 */


/* system24temp_ functions / variables are from shared rewrite files,
   once the rest of the rewrite is complete they can be removed, I
   just made a copy & renamed them for now to avoid any conflicts
*/

#include "driver.h"
#include "segaic24.h"


static void set_color(running_machine *machine, int color, UINT8 r, UINT8 g, UINT8 b, int highlight)
{
	palette_set_color (machine, color, MAKE_RGB(r, g, b));

	if(highlight) {
		r = 255-0.6*(255-r);
		g = 255-0.6*(255-g);
		b = 255-0.6*(255-b);
	} else {
		r = 0.6*r;
		g = 0.6*g;
		b = 0.6*b;
	}
	palette_set_color(machine,color+machine->drv->total_colors/2, MAKE_RGB(r, g, b));
}

// 315-5242
WRITE16_HANDLER (system24temp_sys16_paletteram1_w)
{
	int r, g, b;
	COMBINE_DATA (paletteram16 + offset);
	data = paletteram16[offset];

	r = (data & 0x00f) << 4;
	if(data & 0x1000)
		r |= 8;

	g = data & 0x0f0;
	if(data & 0x2000)
		g |= 8;

	b = (data & 0xf00) >> 4;
	if(data & 0x4000)
		b |= 8;

	r |= r >> 5;
	g |= g >> 5;
	b |= b >> 5;
	set_color(Machine, offset, r, g, b, data & 0x8000);
}

// - System 24

enum {SYS24_TILES = 0x4000};

static UINT16 *sys24_char_ram, *sys24_tile_ram;
static UINT16 sys24_tile_mask;
static UINT8 *sys24_char_dirtymap;
static int sys24_char_dirty, sys24_char_gfx_index;
static tilemap *sys24_tile_layer[4];

#ifdef LSB_FIRST
static const gfx_layout sys24_char_layout = {
	8, 8,
	SYS24_TILES,
	4,
	{ 0, 1, 2, 3 },
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};
#else
static const gfx_layout sys24_char_layout = {
	8, 8,
	SYS24_TILES,
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};
#endif

static TILE_GET_INFO( sys24_tile_info_0s )
{
	UINT16 val = sys24_tile_ram[tile_index];
	tileinfo->category = (val & 0x8000) != 0;
	SET_TILE_INFO(sys24_char_gfx_index, val & sys24_tile_mask, (val >> 7) & 0xff, 0);
}

static TILE_GET_INFO( sys24_tile_info_0w )
{
	UINT16 val = sys24_tile_ram[tile_index|0x1000];
	tileinfo->category = (val & 0x8000) != 0;
	SET_TILE_INFO(sys24_char_gfx_index, val & sys24_tile_mask, (val >> 7) & 0xff, 0);
}

static TILE_GET_INFO( sys24_tile_info_1s )
{
	UINT16 val = sys24_tile_ram[tile_index|0x2000];
	tileinfo->category = (val & 0x8000) != 0;
	SET_TILE_INFO(sys24_char_gfx_index, val & sys24_tile_mask, (val >> 7) & 0xff, 0);
}

static TILE_GET_INFO( sys24_tile_info_1w )
{
	UINT16 val = sys24_tile_ram[tile_index|0x3000];
	tileinfo->category = (val & 0x8000) != 0;
	SET_TILE_INFO(sys24_char_gfx_index, val & sys24_tile_mask, (val >> 7) & 0xff, 0);
}

static void sys24_tile_dirtyall(void)
{
	memset(sys24_char_dirtymap, 1, SYS24_TILES);
	sys24_char_dirty = 1;
}

void sys24_tile_vh_start(running_machine *machine, UINT16 tile_mask)
{
	sys24_tile_mask = tile_mask;

	for(sys24_char_gfx_index = 0; sys24_char_gfx_index < MAX_GFX_ELEMENTS; sys24_char_gfx_index++)
		if (machine->gfx[sys24_char_gfx_index] == 0)
			break;
	assert(sys24_char_gfx_index != MAX_GFX_ELEMENTS);

	sys24_char_ram = auto_malloc(0x80000);

	sys24_tile_ram = auto_malloc(0x10000);

	sys24_char_dirtymap = auto_malloc(SYS24_TILES);

	sys24_tile_layer[0] = tilemap_create(sys24_tile_info_0s, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 64, 64);
	sys24_tile_layer[1] = tilemap_create(sys24_tile_info_0w, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 64, 64);
	sys24_tile_layer[2] = tilemap_create(sys24_tile_info_1s, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 64, 64);
	sys24_tile_layer[3] = tilemap_create(sys24_tile_info_1w, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 64, 64);

	tilemap_set_transparent_pen(sys24_tile_layer[0], 0);
	tilemap_set_transparent_pen(sys24_tile_layer[1], 0);
	tilemap_set_transparent_pen(sys24_tile_layer[2], 0);
	tilemap_set_transparent_pen(sys24_tile_layer[3], 0);

	memset(sys24_char_ram, 0, 0x80000);
	memset(sys24_tile_ram, 0, 0x10000);
	memset(sys24_char_dirtymap, 1, SYS24_TILES);
	sys24_char_dirty = 1;

	machine->gfx[sys24_char_gfx_index] = allocgfx(&sys24_char_layout);

	if (machine->drv->color_table_len)
		machine->gfx[sys24_char_gfx_index]->total_colors = machine->drv->color_table_len / 16;
	else
		machine->gfx[sys24_char_gfx_index]->total_colors = machine->drv->total_colors / 16;

	state_save_register_global_pointer(sys24_tile_ram, 0x10000/2);
	state_save_register_global_pointer(sys24_char_ram, 0x80000/2);
	state_save_register_func_postload(sys24_tile_dirtyall);
}

void sys24_tile_update(running_machine *machine)
{
	if(sys24_char_dirty) {
		int i;
		for(i=0; i<SYS24_TILES; i++) {
			if(sys24_char_dirtymap[i]) {
				sys24_char_dirtymap[i] = 0;
				decodechar(machine->gfx[sys24_char_gfx_index], i, (UINT8 *)sys24_char_ram, &sys24_char_layout);
			}
		}
		tilemap_mark_all_tiles_dirty(sys24_tile_layer[0]);
		tilemap_mark_all_tiles_dirty(sys24_tile_layer[1]);
		tilemap_mark_all_tiles_dirty(sys24_tile_layer[2]);
		tilemap_mark_all_tiles_dirty(sys24_tile_layer[3]);
		sys24_char_dirty = 0;
	}
}

static void sys24_tile_draw_rect(running_machine *machine, mame_bitmap *bm, mame_bitmap *tm, mame_bitmap *dm, const UINT16 *mask,
								 UINT16 tpri, UINT8 lpri, int win, int sx, int sy, int xx1, int yy1, int xx2, int yy2)
{
	int y;
	const UINT16 *source  = ((UINT16 *)bm->base) + sx + sy*bm->rowpixels;
	const UINT8  *trans = ((UINT8 *) tm->base) + sx + sy*tm->rowpixels;
	UINT8        *prib = priority_bitmap->base;
	UINT16       *dest = dm->base;

	tpri |= TILEMAP_PIXEL_LAYER0;

	dest += yy1*dm->rowpixels + xx1;
	prib += yy1*priority_bitmap->rowpixels + xx1;
	mask += yy1*4;
	yy2 -= yy1;

	while(xx1 >= 128) {
		xx1 -= 128;
		xx2 -= 128;
		mask++;
	}

	for(y=0; y<yy2; y++) {
		const UINT16 *src   = source;
		const UINT8  *srct  = trans;
		UINT16 *dst         = dest;
		UINT8 *pr           = prib;
		const UINT16 *mask1 = mask;
		int llx = xx2;
		int cur_x = xx1;

		while(llx > 0) {
			UINT16 m = *mask1++;

			if(win)
				m = ~m;

			if(!cur_x && llx>=128) {
				// Fast paths for the 128-pixels without side clipping case

				if(!m) {
					// 1- 128 pixels from this layer
					int x;
					for(x=0; x<128; x++) {
						if(*srct++ == tpri) {
							*dst = *src;
							*pr |= lpri;
						}
						src++;
						dst++;
						pr++;
					}

				} else if(m == 0xffff) {
					// 2- 128 pixels from the other layer
					src += 128;
					srct += 128;
					dst += 128;
					pr += 128;

				} else {
					// 3- 128 pixels from both layers
					int x;
					for(x=0; x<128; x+=8) {
						if(!(m & 0x8000)) {
							int xx;
							for(xx=0; xx<8; xx++)
								if(srct[xx] == tpri) {
								   dst[xx] = src[xx];
								   pr[xx] |= lpri;
								}
						}
						src += 8;
						srct += 8;
						dst += 8;
						pr += 8;
						m <<= 1;
					}
				}
			} else {
				// Clipped path
				int llx1 = llx >= 128 ? 128 : llx;

				if(!m) {
					// 1- 128 pixels from this layer
					int x;
					for(x = cur_x; x<llx1; x++) {
						if(*srct++ == tpri) {
						   *dst = *src;
						   *pr |= lpri;
						}
						src++;
						dst++;
						pr++;
					}

				} else if(m == 0xffff) {
					// 2- 128 pixels from the other layer
					src += 128 - cur_x;
					srct += 128 - cur_x;
					dst += 128 - cur_x;
					pr += 128 - cur_x;

				} else {
					// 3- 128 pixels from both layers
					int x;
					for(x=cur_x; x<llx1; x++) {
						if(*srct++ == tpri && !(m & (0x8000 >> (x >> 3)))) {
						   *dst = *src;
						   *pr |= lpri;
						}

						src++;
						dst++;
						pr++;
					}
				}
			}
			llx -= 128;
			cur_x = 0;
		}
		source += bm->rowpixels;
		trans  += tm->rowpixels;
		dest   += dm->rowpixels;
		prib   += priority_bitmap->rowpixels;
		mask   += 4;
	}
}


// The rgb version is used by model 1 & 2 which do not need to care
// about sprite priority hence the lack of support for the
// priority_bitmap

static void sys24_tile_draw_rect_rgb(running_machine *machine, mame_bitmap *bm, mame_bitmap *tm, mame_bitmap *dm, const UINT16 *mask,
									 UINT16 tpri, UINT8 lpri, int win, int sx, int sy, int xx1, int yy1, int xx2, int yy2)
{
	int y;
	const UINT16 *source  = ((UINT16 *)bm->base) + sx + sy*bm->rowpixels;
	const UINT8  *trans = ((UINT8 *) tm->base) + sx + sy*tm->rowpixels;
	UINT16       *dest = dm->base;
	const pen_t  *pens   = machine->pens;

	tpri |= TILEMAP_PIXEL_LAYER0;

	dest += yy1*dm->rowpixels + xx1;
	mask += yy1*4;
	yy2 -= yy1;

	while(xx1 >= 128) {
		xx1 -= 128;
		xx2 -= 128;
		mask++;
	}

	for(y=0; y<yy2; y++) {
		const UINT16 *src   = source;
		const UINT8  *srct  = trans;
		UINT16 *dst         = dest;
		const UINT16 *mask1 = mask;
		int llx = xx2;
		int cur_x = xx1;

		while(llx > 0) {
			UINT16 m = *mask1++;

			if(win)
				m = ~m;

			if(!cur_x && llx>=128) {
				// Fast paths for the 128-pixels without side clipping case

				if(!m) {
					// 1- 128 pixels from this layer
					int x;
					for(x=0; x<128; x++) {
						if(*srct++ == tpri)
							*dst = pens[*src];
						src++;
						dst++;
					}

				} else if(m == 0xffff) {
					// 2- 128 pixels from the other layer
					src += 128;
					srct += 128;
					dst += 128;

				} else {
					// 3- 128 pixels from both layers
					int x;
					for(x=0; x<128; x+=8) {
						if(!(m & 0x8000)) {
							int xx;
							for(xx=0; xx<8; xx++)
								if(srct[xx] == tpri)
								   dst[xx] = pens[src[xx]];
						}
						src += 8;
						srct += 8;
						dst += 8;
						m <<= 1;
					}
				}
			} else {
				// Clipped path
				int llx1 = llx >= 128 ? 128 : llx;

				if(!m) {
					// 1- 128 pixels from this layer
					int x;
					for(x = cur_x; x<llx1; x++) {
						if(*srct++ == tpri)
						   *dst = pens[*src];
						src++;
						dst++;
					}

				} else if(m == 0xffff) {
					// 2- 128 pixels from the other layer
					src += 128 - cur_x;
					srct += 128 - cur_x;
					dst += 128 - cur_x;

				} else {
					// 3- 128 pixels from both layers
					int x;
					for(x=cur_x; x<llx1; x++) {
						if(*srct++ == tpri && !(m & (0x8000 >> (x >> 3))))
						   *dst = pens[*src];

						src++;
						dst++;
					}
				}
			}
			llx -= 128;
			cur_x = 0;
		}
		source += bm->rowpixels;
		trans  += tm->rowpixels;
		dest   += dm->rowpixels;
		mask   += 4;
	}
}

void sys24_tile_draw(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int layer, int lpri, int flags)
{
	UINT16 hscr = sys24_tile_ram[0x5000+(layer >> 1)];
	UINT16 vscr = sys24_tile_ram[0x5004+(layer >> 1)];
	UINT16 ctrl = sys24_tile_ram[0x5004+((layer >> 1) & 2)];
	UINT16 *mask = sys24_tile_ram + (layer & 4 ? 0x6800 : 0x6000);
	UINT16 tpri = layer & 1;

	lpri = 1 << lpri;
	layer >>= 1;

	// Layer disable
	if(vscr & 0x8000)
		return;

	if(ctrl & 0x6000) {
		// Special window/scroll modes
		if(layer & 1)
			return;

		tilemap_set_scrolly(sys24_tile_layer[layer],   0, +(vscr & 0x1ff));
		tilemap_set_scrolly(sys24_tile_layer[layer|1], 0, +(vscr & 0x1ff));

		if(hscr & 0x8000) {
			//#ifdef MAME_DEBUG
			popmessage("Linescroll with special mode %04x", ctrl);
			//          return;
			//#endif
		} else {
			tilemap_set_scrollx(sys24_tile_layer[layer],   0, -(hscr & 0x1ff));
			tilemap_set_scrollx(sys24_tile_layer[layer|1], 0, -(hscr & 0x1ff));
		}

		switch((ctrl & 0x6000) >> 13) {
		case 1: {
			rectangle c1 = *cliprect;
			rectangle c2 = *cliprect;
			UINT16 v;
			v = (-vscr) & 0x1ff;
			if(c1.max_y >= v)
				c1.max_y = v-1;
			if(c2.min_y < v)
				c2.min_y = v;
			if(!((-vscr) & 0x200))
				layer ^= 1;

			tilemap_draw(bitmap, &c1, sys24_tile_layer[layer],   tpri, lpri);
			tilemap_draw(bitmap, &c2, sys24_tile_layer[layer^1], tpri, lpri);
			break;
		}
		case 2: {
			rectangle c1 = *cliprect;
			rectangle c2 = *cliprect;
			UINT16 h;
			h = (+hscr) & 0x1ff;
			if(c1.max_x >= h)
				c1.max_x = h-1;
			if(c2.min_x < h)
				c2.min_x = h;
			if(!((+hscr) & 0x200))
				layer ^= 1;

			tilemap_draw(bitmap, &c1, sys24_tile_layer[layer],   tpri, lpri);
			tilemap_draw(bitmap, &c2, sys24_tile_layer[layer^1], tpri, lpri);
			break;
		}
		case 3:
			//#ifdef MAME_DEBUG
			popmessage("Mode 3, please scream");
			//#endif
			break;
		};

	} else {
		mame_bitmap *bm, *tm;
		void (*draw)(running_machine *machine, mame_bitmap *, mame_bitmap *, mame_bitmap *, const UINT16 *,
					 UINT16, UINT8, int, int, int, int, int, int, int);
		int win = layer & 1;

		if(bitmap->format != BITMAP_FORMAT_INDEXED16)
			draw = sys24_tile_draw_rect_rgb;
		else
			draw = sys24_tile_draw_rect;

		bm = tilemap_get_pixmap(sys24_tile_layer[layer]);
		tm = tilemap_get_flagsmap(sys24_tile_layer[layer]);

		if(hscr & 0x8000) {
			int y;
			UINT16 *hscrtb = sys24_tile_ram + 0x4000 + 0x200*layer;
			vscr &= 0x1ff;

			for(y=0; y<384; y++) {
				// Whether it's tilemap-relative or screen-relative is unknown
				hscr = (-hscrtb[vscr]) & 0x1ff;
				if(hscr + 496 <= 512) {
					// Horizontal split unnecessary
					draw(machine, bm, tm, bitmap, mask, tpri, lpri, win, hscr, vscr,        0,        y,      496,      y+1);
				} else {
					// Horizontal split necessary
					draw(machine, bm, tm, bitmap, mask, tpri, lpri, win, hscr, vscr,        0,        y, 512-hscr,      y+1);
					draw(machine, bm, tm, bitmap, mask, tpri, lpri, win,    0, vscr, 512-hscr,        y,      496,      y+1);
				}
				vscr = (vscr + 1) & 0x1ff;
			}
		} else {
			hscr = (-hscr) & 0x1ff;
			vscr = (+vscr) & 0x1ff;

			if(hscr + 496 <= 512) {
				// Horizontal split unnecessary
				if(vscr + 384 <= 512) {
					// Vertical split unnecessary
					draw(machine, bm, tm, bitmap, mask, tpri, lpri, win, hscr, vscr,        0,        0,      496,      384);
				} else {
					// Vertical split necessary
					draw(machine, bm, tm, bitmap, mask, tpri, lpri, win, hscr, vscr,        0,        0,      496, 512-vscr);
					draw(machine, bm, tm, bitmap, mask, tpri, lpri, win, hscr,    0,        0, 512-vscr,      496,      384);

				}
			} else {
				// Horizontal split necessary
				if(vscr + 384 <= 512) {
					// Vertical split unnecessary
					draw(machine, bm, tm, bitmap, mask, tpri, lpri, win, hscr, vscr,        0,        0, 512-hscr,      384);
					draw(machine, bm, tm, bitmap, mask, tpri, lpri, win,    0, vscr, 512-hscr,        0,      496,      384);
				} else {
					// Vertical split necessary
					draw(machine, bm, tm, bitmap, mask, tpri, lpri, win, hscr, vscr,        0,        0, 512-hscr, 512-vscr);
					draw(machine, bm, tm, bitmap, mask, tpri, lpri, win,    0, vscr, 512-hscr,        0,      496, 512-vscr);
					draw(machine, bm, tm, bitmap, mask, tpri, lpri, win, hscr,    0,        0, 512-vscr, 512-hscr,      384);
					draw(machine, bm, tm, bitmap, mask, tpri, lpri, win,    0,    0, 512-hscr, 512-vscr,      496,      384);
				}
			}
		}
	}
}

READ16_HANDLER(sys24_tile_r)
{
	return sys24_tile_ram[offset];
}

READ16_HANDLER(sys24_char_r)
{
	return sys24_char_ram[offset];
}

WRITE16_HANDLER(sys24_tile_w)
{
	COMBINE_DATA(sys24_tile_ram + offset);
	if(offset < 0x4000)
		tilemap_mark_tile_dirty(sys24_tile_layer[offset >> 12], offset & 0xfff);
}

WRITE16_HANDLER(sys24_char_w)
{
	UINT16 old = sys24_char_ram[offset];
	COMBINE_DATA(sys24_char_ram + offset);
	if(old != sys24_char_ram[offset]) {
		sys24_char_dirtymap[offset / 16] = 1;
		sys24_char_dirty = 1;
	}
}

READ32_HANDLER(sys24_tile32_r)
{
	return sys24_tile_r(offset*2, mem_mask&0xffff) | sys24_tile_r((offset*2)+1, mem_mask>>16)<<16;
}

READ32_HANDLER(sys24_char32_r)
{
	return sys24_char_r(offset*2, mem_mask&0xffff) | sys24_char_r((offset*2)+1, mem_mask>>16)<<16;
}

WRITE32_HANDLER(sys24_tile32_w)
{
	sys24_tile_w(offset*2, data&0xffff, mem_mask&0xffff);
	sys24_tile_w((offset*2)+1, data>>16, mem_mask>>16);
}

WRITE32_HANDLER(sys24_char32_w)
{
	sys24_char_w(offset*2, data&0xffff, mem_mask&0xffff);
	sys24_char_w((offset*2)+1, data>>16, mem_mask>>16);
}

// - System 24

static UINT16 *sys24_sprite_ram;

void sys24_sprite_vh_start(void)
{
	sys24_sprite_ram = auto_malloc(0x40000);

	state_save_register_global_pointer(sys24_sprite_ram, 0x40000/2);
	//  kc = 0;
}

/* System24 sprites
      Normal sprite:
    0   00Znnnnn    nnnnnnnn    zoom mode (1 = separate x and y), next sprite
    1   xxxxxxxx    yyyyyyyy    zoom x, zoom y (zoom y is used for both when mode = 0)
    2   --TTTTTT    TTTTTTTT    sprite number
    3   -CCCCCCC    CCCCCCCC    indirect palette base
    4   FSSSYYYY    YYYYYYYY    flipy, y size, top
    5   FSSSXXXX    XXXXXXXX    flipx, x size, left

      Clip?
    0   01-nnnnn    nnnnnnnn    next sprite
    1   ????????    ????????

      Skipped entry
    0   10-nnnnn    nnnnnnnn    next sprite

      End of sprite list
    0   11------    --------
*/

void sys24_sprite_draw(mame_bitmap *bitmap, const rectangle *cliprect, const int *spri)
{
	UINT16 curspr = 0;
	int countspr = 0;
	int seen;
	UINT8 pmt[4];
	int i;
	UINT16 *sprd[0x2000], *clip[0x2000];
	UINT16 *cclip = 0;

	for(i=0; i<4; i++)
		pmt[i] = 0xff << (1+spri[3-i]);

	for(seen = 0; seen < 0x2000; seen++) {
		UINT16 *source;
		UINT16 type;

		source = sys24_sprite_ram + (curspr << 3);

		if(curspr == 0 && source[0] == 0)
			break;

		curspr = source[0];
		type = curspr & 0xc000;
		curspr &= 0x1fff;

		if(type == 0xc000)
			break;

		if(type == 0x8000)
			continue;

		if(type == 0x4000) {
			cclip = source;
			continue;
		}

		sprd[countspr] = source;
		clip[countspr] = cclip;

		countspr++;
		if(!curspr)
			break;
	}

	for(countspr--; countspr >= 0; countspr--) {
		UINT16 *source, *pix;
		int x, y, sx, sy;
		int px, py;
		UINT16 colors[16];
		int flipx, flipy;
		int zoomx, zoomy;
		UINT8 pm[16];
		//      int dump;
		int xmod, ymod;
		int min_x, min_y, max_x, max_y;

		UINT32 addoffset;
		UINT32 newoffset;
		UINT32 offset;

		source = sprd[countspr];
		cclip = clip[countspr];

		if(cclip) {
			min_y = (cclip[2] & 511);
			min_x = (cclip[3] & 511) - 8;
			max_y = (cclip[4] & 511);
			max_x = (cclip[5] & 511) - 8;
		} else {
			min_x = 0;
			max_x = 495;
			min_y = 0;
			max_y = 383;
		}


		if(min_x < cliprect->min_x)
			min_x = cliprect->min_x;
		if(min_y < cliprect->min_y)
			min_y = cliprect->min_y;
		if(max_x > cliprect->max_x)
			max_x = cliprect->max_x;
		if(max_y > cliprect->max_y)
			max_y = cliprect->max_y;

		if(!(source[0] & 0x2000))
			zoomx = zoomy = source[1] & 0xff;
		else {
			zoomx = source[1] >> 8;
			zoomy = source[1] & 0xff;
		}
		if(!zoomx)
			zoomx = 0x3f;
		if(!zoomy)
			zoomy = 0x3f;

		zoomx++;
		zoomy++;

		x = source[5] & 0xfff;
		flipx = source[5] & 0x8000;
		if(x & 0x800)
			x -= 0x1000;
		sx = 1 << ((source[5] & 0x7000) >> 12);

		x -= 8;

		y = source[4] & 0xfff;
		if(y & 0x800)
			y -= 0x1000;
		flipy = source[4] & 0x8000;
		sy = 1 << ((source[4] & 0x7000) >> 12);

		pix = &sys24_sprite_ram[(source[3] & 0x3fff)* 0x8];
		for(px=0; px<8; px++) {
			int c;
			c              = pix[px] >> 8;
			pm[px*2]       = pmt[c>>6];
			if(c>1)
				c |= 0x1000;
			colors[px*2]   = c;

			c              = pix[px] & 0xff;
			pm[px*2+1]     = pmt[c>>6];
			if(c>1)
				c |= 0x1000;
			colors[px*2+1] = c;
		}

		offset = (source[2] & 0x7fff) * 0x10;

		xmod = 0x20;
		ymod = 0x20;
		for(py=0; py<sy; py++) {
			int xmod1 = xmod;
			int xpos1 = x;
			int ypos1 = y, ymod1 = ymod;
			for(px=0; px<sx; px++) {
				int xmod2 = xmod1, xpos2 = xpos1;
				int zy;
				addoffset = 0x10*(flipx ? sx-px-1 : px) + 0x10*sx*(flipy ? sy-py-1 : py) + (flipy ? 7*2 : 0);
				newoffset = offset + addoffset;

				ymod1 = ymod;
				ypos1 = y;
				for(zy=0; zy<8; zy++) {

					ymod1 += zoomy;
					while(ymod1 >= 0x40) {
						if(ypos1 >= min_y && ypos1 <= max_y) {
							int zx;
							xmod2 = xmod1;
							xpos2 = xpos1;

							for(zx=0; zx<8; zx++) {
								xmod2 += zoomx;
								while(xmod2 >= 0x40) {
									if(xpos2 >= min_x && xpos2 <= max_x) {
										int zx1 = flipx ? 7-zx : zx;
										UINT32 neweroffset = (newoffset+(zx1>>2))&0x1ffff; // crackdown sometimes attempts to use data past the end of spriteram
										int c = (sys24_sprite_ram[neweroffset] >> (((~zx1) & 3) << 2)) & 0xf;
										UINT8 *pri = BITMAP_ADDR8(priority_bitmap, ypos1, xpos2);
										if(!(*pri & pm[c])) {
											c = colors[c];
											if(c) {
												UINT16 *dst = BITMAP_ADDR16(bitmap, ypos1, xpos2);
												if(c==1)
													*dst = (*dst) | 0x2000;
												else
													*dst = c;
												*pri = 0xff;
											}
										}
									}
									xmod2 -= 0x40;
									xpos2++;
								}
							}
						}
						ymod1 -= 0x40;
						ypos1++;
					}
					if(flipy)
						newoffset -= 2;
					else
						newoffset += 2;
				}

				xpos1 = xpos2;
				xmod1 = xmod2;
			}
			y    = ypos1;
			ymod = ymod1;
		}
	}
}


WRITE16_HANDLER(sys24_sprite_w)
{
	COMBINE_DATA(sys24_sprite_ram + offset);
}

READ16_HANDLER(sys24_sprite_r)
{
	return sys24_sprite_ram[offset];
}

// Programmable mixers
//   System 24

static UINT16 sys24_mixer_reg[0x10];

void sys24_mixer_vh_start(void)
{
	memset(sys24_mixer_reg, 0, sizeof(sys24_mixer_reg));
	state_save_register_global_array(sys24_mixer_reg);
}

WRITE16_HANDLER (sys24_mixer_w)
{
	COMBINE_DATA(sys24_mixer_reg + offset);
}

READ16_HANDLER (sys24_mixer_r)
{
	return sys24_mixer_reg[offset];
}

int sys24_mixer_get_reg(int reg)
{
	return sys24_mixer_reg[reg];
}

