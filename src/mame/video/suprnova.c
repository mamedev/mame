/* Super Kaneko Nova System video */

/*
TODO:
Implement double buffering for sprites?
We have to tell games which buffer to draw to? irrelevant to emulation

Remove all debug keys
*/

/*
Done:
Add global sprite flip
Tilemap flip flags were reversed
*/

#include "driver.h"

#define SUPRNOVA_DECODE_BUFFER_SIZE 0x2000

extern UINT32 *skns_tilemapA_ram, *skns_tilemapB_ram, *skns_v3slc_ram;
extern UINT32 *skns_palette_ram, *skns_v3t_ram, *skns_main_ram, *skns_cache_ram;
extern UINT32 *skns_pal_regs, *skns_v3_regs, *skns_spc_regs;
extern UINT32 skns_v3t_dirty[0x4000]; // allocate this elsewhere?
extern UINT32 skns_v3t_4bppdirty[0x8000]; // allocate this elsewhere?
extern int skns_v3t_somedirty,skns_v3t_4bpp_somedirty;

static UINT8 decodebuffer[SUPRNOVA_DECODE_BUFFER_SIZE];
static int old_depthA=0, depthA=0;
static int old_depthB=0, depthB=0;

static int sprite_kludge_x=0, sprite_kludge_y=0;
static int use_spc_bright, use_v3_bright;
static UINT8 bright_spc_b=0x00, bright_spc_g=0x00, bright_spc_r=0x00;
static UINT8 bright_v3_b=0x00,  bright_v3_g=0x00,  bright_v3_r=0x00;


// This ignores the alpha values atm.
static int spc_changed=0, v3_changed=0, palette_updated=0;

WRITE32_HANDLER ( skns_pal_regs_w )
{
	COMBINE_DATA(&skns_pal_regs[offset]);
	palette_updated =1;

	switch ( offset )
	{
	case (0x00/4): // RWRA0
		if( use_spc_bright != (data&1) ) {
			use_spc_bright = data&1;
			spc_changed = 1;
		}
		break;
	case (0x04/4): // RWRA1
		if( bright_spc_g != (data&0xff) ) {
			bright_spc_g = data&0xff;
			spc_changed = 1;
		}
		break;
	case (0x08/4): // RWRA2
		if( bright_spc_r != (data&0xff) ) {
			bright_spc_r = data&0xff;
			spc_changed = 1;
		}
		break;
	case (0x0C/4): // RWRA3
		if( bright_spc_b != (data&0xff) ) {
			bright_spc_b = data&0xff;
			spc_changed = 1;
		}
		break;

	case (0x10/4): // RWRB0
		if( use_v3_bright != (data&1) ) {
			use_v3_bright = data&1;
			v3_changed = 1;
		}
		break;
	case (0x14/4): // RWRB1
		if( bright_v3_g != (data&0xff) ) {
			bright_v3_g = data&0xff;
			v3_changed = 1;
		}
		break;
	case (0x18/4): // RWRB2
		if( bright_v3_r != (data&0xff) ) {
			bright_v3_r = data&0xff;
			v3_changed = 1;
		}
		break;
	case (0x1C/4): // RWRB3
		if( bright_v3_b != (data&0xff) ) {
			bright_v3_b = data&0xff;
			v3_changed = 1;
		}
		break;
	}
}


WRITE32_HANDLER ( skns_palette_ram_w )
{
	int r,g,b;
	int brightness_r, brightness_g, brightness_b, alpha;
	int use_bright;

	COMBINE_DATA(&skns_palette_ram[offset]);

	b = ((skns_palette_ram[offset] >> 0  ) & 0x1f);
	g = ((skns_palette_ram[offset] >> 5  ) & 0x1f);
	r = ((skns_palette_ram[offset] >> 10  ) & 0x1f);

	alpha = ((skns_palette_ram[offset] >> 15  ) & 0x1);

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

	palette_set_color(Machine,offset,MAKE_RGB(r,g,b));
}


static void palette_set_rgb_brightness (running_machine *machine, int offset, UINT8 brightness_r, UINT8 brightness_g, UINT8 brightness_b)
{
	int use_bright, r, g, b, alpha;

	b = ((skns_palette_ram[offset] >> 0  ) & 0x1f);
	g = ((skns_palette_ram[offset] >> 5  ) & 0x1f);
	r = ((skns_palette_ram[offset] >> 10  ) & 0x1f);

	alpha = ((skns_palette_ram[offset] >> 15  ) & 0x1);

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


static void palette_update(running_machine *machine)
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


static int skns_rle_decode ( int romoffset, int size )
{
	UINT8 *src = memory_region (REGION_GFX1);
	size_t srcsize = memory_region_length (REGION_GFX1);
	UINT8 *dst = decodebuffer;
	int decodeoffset = 0;

	while(size>0) {
		UINT8 code = src[(romoffset++)%srcsize];
		size -= (code & 0x7f) + 1;
		if(code & 0x80) {
			code &= 0x7f;
			do {
				dst[(decodeoffset++)%SUPRNOVA_DECODE_BUFFER_SIZE] = src[(romoffset++)%srcsize];
				code--;
			} while(code != 0xff);
		} else {
			UINT8 val = src[(romoffset++)%srcsize];
			do {
				dst[(decodeoffset++)%SUPRNOVA_DECODE_BUFFER_SIZE] = val;
				code--;
			} while(code != 0xff);
		}
	}
	return &src[romoffset%srcsize]-memory_region (REGION_GFX1);
}

void skns_sprite_kludge(int x, int y)
{
	sprite_kludge_x = x;
	sprite_kludge_y = y;
}

/* Zooming blitter, zoom is by way of both source and destination offsets */
/* We are working in .6 fixed point if you hadn't guessed */

#define z_decls(step)				\
	UINT16 zxs = 0x40-(zx>>10);			\
	UINT16 zxd = 0x40-((zx>>2) & 0x3f);		\
	UINT16 zys = 0x40-(zy>>10);			\
	UINT16 zyd = 0x40-((zy>>2) & 0x3f);		\
	int xs, ys, xd, yd, old, old2;		\
	int step_spr = step;				\
	int bxs = 0, bys = 0;				\
	rectangle clip;					\
	clip.min_x = cliprect->min_x<<6;					\
	clip.max_x = (cliprect->max_x+1)<<6;					\
	clip.min_y = cliprect->min_y<<6;					\
	clip.max_y = (cliprect->max_y+1)<<6;					\
	sx <<= 6;					\
	sy <<= 6;					\
	x <<= 6;					\
	y <<= 6;

#define z_clamp_x_min()			\
	if(x < clip.min_x) {					\
		do {					\
			bxs += zxs;				\
			x += zxd;					\
		} while(x < clip.min_x);				\
	}

#define z_clamp_x_max()			\
	if(x > clip.max_x) {				\
		do {					\
			bxs += zxs;				\
			x -= zxd;					\
		} while(x > clip.max_x);				\
	}

#define z_clamp_y_min()			\
	if(y < clip.min_y) {					\
		do {					\
			bys += zys;				\
			y += zyd;					\
		} while(y < clip.min_y);				\
		src += (bys>>6)*step_spr;			\
	}

#define z_clamp_y_max()			\
	if(y > clip.max_y) {				\
		do {					\
			bys += zys;				\
			y -= zyd;					\
		} while(y > clip.max_y);				\
		src += (bys>>6)*step_spr;			\
	}

#define z_loop_x()			\
	xs = bxs;					\
	xd = x;					\
	while(xs < sx && xd <= clip.max_x)

#define z_loop_x_flip()			\
	xs = bxs;					\
	xd = x;					\
	while(xs < sx && xd >= clip.min_x)

#define z_loop_y()			\
	ys = bys;					\
	yd = y;					\
	while(ys < sy && yd <= clip.max_y)

#define z_loop_y_flip()			\
	ys = bys;					\
	yd = y;					\
	while(ys < sy && yd >= clip.min_y)

#define z_draw_pixel()				\
	UINT8 val = src[xs >> 6];			\
	if(val)					\
		*BITMAP_ADDR16(bitmap, yd>>6, xd>>6) = val + colour;

#define z_x_dst(op)			\
	old = xd;					\
	do {						\
		xs += zxs;					\
		xd op zxd;					\
	} while(!((xd^old) & ~0x3f));

#define z_y_dst(op)			\
	old = yd;					\
	old2 = ys;					\
	do {						\
		ys += zys;					\
		yd op zyd;					\
	} while(!((yd^old) & ~0x3f));			\
	while((ys^old2) & ~0x3f) {			\
		src += step_spr;				\
		old2 += 0x40;				\
	}

static void blit_nf_z(mame_bitmap *bitmap, const rectangle *cliprect, const UINT8 *src, int x, int y, int sx, int sy, UINT16 zx, UINT16 zy, int colour)
{
	z_decls(sx);
	z_clamp_x_min();
	z_clamp_y_min();
	z_loop_y() {
		z_loop_x() {
			z_draw_pixel();
			z_x_dst(+=);
		}
		z_y_dst(+=);
	}
}

static void blit_fy_z(mame_bitmap *bitmap, const rectangle *cliprect, const UINT8 *src, int x, int y, int sx, int sy, UINT16 zx, UINT16 zy, int colour)
{
	z_decls(sx);
	z_clamp_x_min();
	z_clamp_y_max();
	z_loop_y_flip() {
		z_loop_x() {
			z_draw_pixel();
			z_x_dst(+=);
		}
		z_y_dst(-=);
	}
}

static void blit_fx_z(mame_bitmap *bitmap, const rectangle *cliprect, const UINT8 *src, int x, int y, int sx, int sy, UINT16 zx, UINT16 zy, int colour)
{
	z_decls(sx);
	z_clamp_x_max();
	z_clamp_y_min();
	z_loop_y() {
		z_loop_x_flip() {
			z_draw_pixel();
			z_x_dst(-=);
		}
		z_y_dst(+=);
	}
}

static void blit_fxy_z(mame_bitmap *bitmap, const rectangle *cliprect, const UINT8 *src, int x, int y, int sx, int sy, UINT16 zx, UINT16 zy, int colour)
{
	z_decls(sx);
	z_clamp_x_max();
	z_clamp_y_max();
	z_loop_y_flip() {
		z_loop_x_flip() {
			z_draw_pixel();
			z_x_dst(-=);
		}
		z_y_dst(-=);
	}
}

static void (*const blit_z[4])(mame_bitmap *bitmap, const rectangle *cliprect, const UINT8 *src, int x, int y, int sx, int sy, UINT16 zx, UINT16 zy, int colour) = {
	blit_nf_z,
	blit_fy_z,
	blit_fx_z,
	blit_fxy_z,
};

void skns_draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	/*- SPR RAM Format -**

      16 bytes per sprite

0x00  --ss --SS  z--- ----  jjjg g-ff  ppcc cccc

      s = y size
      S = x size
      j = joint
      g = group sprite is part of (if groups are enabled)
      f = flip
      p = priority
      c = palette

0x04  ---- -aaa  aaaa aaaa  aaaa aaaa  aaaa aaaa

      a = ROM address of sprite data

0x08  ZZZZ ZZ--  zzzz zz--  xxxx xxxx  xx-- ----

      Z = horizontal zoom table
      z = horizontal zoom subtable
      x = x position

0x0C  ZZZZ ZZ--  zzzz zz--  yyyy yyyy  yy-- ----

      Z = vertical zoom table
      z = vertical zoom subtable
      x = y position

    **- End of Comments -*/

	/* sprite ram start / end is not really fixed registers change it */

	UINT32 *source = buffered_spriteram32;
	UINT32 *finish = source + spriteram_size/4;

	int group_x_offset[4];
	int group_y_offset[4];
	int group_enable;
	int group_number;
	int sprite_flip;
	int sprite_x_scroll;
	int sprite_y_scroll;
	int disabled = skns_spc_regs[0x04/4] & 0x08; // RWR1
	int xsize,ysize, size, xpos=0,ypos=0, pri=0, romoffset, colour=0, xflip,yflip, joint;
	int sx,sy;
	int endromoffs=0;
	UINT16 zoomx, zoomy;


	if (!disabled){

		group_enable    = (skns_spc_regs[0x00/4] & 0x0040) >> 6; // RWR0

		/* Sengekis uses global flip */
		sprite_flip = (skns_spc_regs[0x04/4] & 0x03); // RWR1

		sprite_y_scroll = ((skns_spc_regs[0x08/4] & 0x7fc0) >> 6); // RWR2
		sprite_x_scroll = ((skns_spc_regs[0x10/4] & 0x7fc0) >> 6); // RWR4
		if (sprite_y_scroll&0x100) sprite_y_scroll -= 0x200; // Signed
		if (sprite_x_scroll&0x100) sprite_x_scroll -= 0x200; // Signed

		group_x_offset[0] = (skns_spc_regs[0x18/4] & 0xffc0) >> 6; // RWR6
		group_y_offset[0] = (skns_spc_regs[0x1c/4] & 0xffc0) >> 6; // RWR7
		if (group_x_offset[0]&0x200) group_x_offset[0] -= 0x400; // Signed
		if (group_y_offset[0]&0x200) group_y_offset[0] -= 0x400; // Signed

		group_x_offset[1] = (skns_spc_regs[0x20/4] & 0xffc0) >> 6; // RWR8
		group_y_offset[1] = (skns_spc_regs[0x24/4] & 0xffc0) >> 6; // RWR9
		if (group_x_offset[1]&0x200) group_x_offset[1] -= 0x400; // Signed
		if (group_y_offset[1]&0x200) group_y_offset[1] -= 0x400; // Signed

		group_x_offset[2] = (skns_spc_regs[0x28/4] & 0xffc0) >> 6; // RWR10
		group_y_offset[2] = (skns_spc_regs[0x2c/4] & 0xffc0) >> 6; // RWR11
		if (group_x_offset[2]&0x200) group_x_offset[2] -= 0x400; // Signed
		if (group_y_offset[2]&0x200) group_y_offset[2] -= 0x400; // Signed

		group_x_offset[3] = (skns_spc_regs[0x30/4] & 0xffc0) >> 6; // RWR12
		group_y_offset[3] = (skns_spc_regs[0x34/4] & 0xffc0) >> 6; // RWR13
		if (group_x_offset[3]&0x200) group_x_offset[3] -= 0x400; // Signed
		if (group_y_offset[3]&0x200) group_y_offset[3] -= 0x400; // Signed

	//  popmessage ("x %08x y %08x x2 %08x y2 %08x",sprite_x_scroll, sprite_y_scroll,group_x_offset[1], group_y_offset[1]);
	//  popmessage("%d %d %d %d A:%d B:%d", sprite_kludge_x, sprite_kludge_y, sprite_x_scroll, sprite_y_scroll, (skns_pal_regs[0x00/4] & 0x7000) >> 12, (skns_pal_regs[0x10/4] & 0x7000) >> 12);
	//  if (keyboard_pressed(KEYCODE_Q)) sprite_kludge_x++;
	//  if (keyboard_pressed(KEYCODE_W)) sprite_kludge_x--;
	//  if (keyboard_pressed(KEYCODE_E)) sprite_kludge_y++;
	//  if (keyboard_pressed(KEYCODE_D)) sprite_kludge_y--;

		// Tilemap Pri/enables
	//  popmessage("A: %x %x B: %x %x", skns_v3_regs[0x10/4]>>3, skns_v3_regs[0x10/4]&7, skns_v3_regs[0x34/4]>>3, skns_v3_regs[0x34/4]&7);

		/* Seems that sprites are consistently off by a fixed no. of pixels in different games
           (Patterns emerge through Manufacturer/Date/Orientation) */
		sprite_x_scroll += sprite_kludge_x;
		sprite_y_scroll += sprite_kludge_y;


		while( source<finish )
		{
			xflip = (source[0] & 0x00000200) >> 9;
			yflip = (source[0] & 0x00000100) >> 8;

			ysize = (source[0] & 0x30000000) >> 28;
			xsize = (source[0] & 0x03000000) >> 24;
			xsize ++;
			ysize ++;

			xsize *= 16;
			ysize *= 16;

			size = xsize * ysize;

			joint = (source[0] & 0x0000e000) >> 13;

			if (!(joint & 1))
			{
				xpos =  (source[2] & 0x0000ffc0) >> 6;
				ypos =  (source[3] & 0x0000ffc0) >> 6;

				xpos += sprite_x_scroll; // Global offset
				ypos += sprite_y_scroll;

				if (group_enable)
				{
					group_number = (source[0] & 0x00001800) >> 11;

					/* the group positioning doesn't seem to be working as i'd expect,
                       if I apply the x position the cursor on galpani4 ends up moving
                       from the correct position to too far right, also the y offset
                       seems to cause the position to be off by one in galpans2 even if
                       it fixes the position in galpani4?

                       even if I take into account the global sprite scroll registers
                       it isn't right

                       global offset kludged using game specific offset -pjp */

					xpos += group_x_offset[group_number];
					ypos += group_y_offset[group_number];
				}
			}
			else
			{
				xpos +=  (source[2] & 0x0000ffc0) >> 6;
				ypos +=  (source[3] & 0x0000ffc0) >> 6;
			}

			if (xpos > 0x1ff) xpos -= 0x400;
			if (ypos > 0x1ff) ypos -= 0x400;

			/* Local sprite offset (for taking flip into account and drawing offset) */
			sx = xpos;
			sy = ypos;

			/* Global Sprite Flip (sengekis) */
			if (sprite_flip&2)
			{
				xflip ^= 1;
				sx = machine->screen[0].visarea.max_x+1 - sx;
			}
			if (sprite_flip&1)
			{
				yflip ^= 1;
				sy = machine->screen[0].visarea.max_y+1 - sy;
			}

			/* Palette linking */
			if (!(joint & 2))
			{
				colour = (source[0] & 0x0000003f) >> 0;
			}

			/* Priority and Tile linking */
			if (!(joint & 4))
			{
				romoffset = (source[1] & 0x07ffffff) >> 0;
				pri = (source[0] & 0x000000c0) >> 6;
			} else {
				romoffset = endromoffs;
			}

			zoomx = source[2] >> 16;
			zoomy = source[3] >> 16;

			romoffset &= memory_region_length (REGION_GFX1)-1;

			endromoffs = skns_rle_decode ( romoffset, size );

			// PriTest
//          if(!( (keyboard_pressed(KEYCODE_Q)&&(pri==0)) || (keyboard_pressed(KEYCODE_W)&&(pri==1)) || (keyboard_pressed(KEYCODE_E)&&(pri==2)) || (keyboard_pressed(KEYCODE_D)&&(pri==3)) ))
//          if( !(keyboard_pressed(KEYCODE_Q) && ((source[0] & 0x00800000)>>24)) )
			{
				int NewColour = colour*256;

				if(zoomx || zoomy)
				{
					blit_z[ (xflip<<1) | yflip ](bitmap, cliprect, decodebuffer, sx, sy, xsize, ysize, zoomx, zoomy, NewColour);
				}
				else
				{
					if (!xflip && !yflip) {
						int xx,yy;

						for (xx = 0; xx<xsize; xx++)
						{
							if ((sx+xx < (cliprect->max_x+1)) && (sx+xx >= cliprect->min_x))
							{
								for (yy = 0; yy<ysize; yy++)
								{
									if ((sy+yy < (cliprect->max_y+1)) && (sy+yy >= cliprect->min_y))
									{
										int pix;
										pix = decodebuffer[xsize*yy+xx];
										if (pix)
											*BITMAP_ADDR16(bitmap, sy+yy, sx+xx) = pix+ NewColour; // change later
									}
								}
							}
						}
					} else if (!xflip && yflip) {
						int xx,yy;
						sy -= ysize;

						for (xx = 0; xx<xsize; xx++)
						{
							if ((sx+xx < (cliprect->max_x+1)) && (sx+xx >= cliprect->min_x))
							{
								for (yy = 0; yy<ysize; yy++)
								{
									if ((sy+(ysize-1-yy) < (cliprect->max_y+1)) && (sy+(ysize-1-yy) >= cliprect->min_y))
									{
										int pix;
										pix = decodebuffer[xsize*yy+xx];
										if (pix)
											*BITMAP_ADDR16(bitmap, sy+(ysize-1-yy), sx+xx) = pix+ NewColour; // change later
									}
								}
							}
						}
					} else if (xflip && !yflip) {
						int xx,yy;
						sx -= xsize;

						for (xx = 0; xx<xsize; xx++)
						{
							if ( (sx+(xsize-1-xx) < (cliprect->max_x+1)) && (sx+(xsize-1-xx) >= cliprect->min_x))
							{
								for (yy = 0; yy<ysize; yy++)
								{
									if ((sy+yy < (cliprect->max_y+1)) && (sy+yy >= cliprect->min_y))
									{
										int pix;
										pix = decodebuffer[xsize*yy+xx];
										if (pix)
											*BITMAP_ADDR16(bitmap, sy+yy, sx+(xsize-1-xx)) = pix+ NewColour; // change later
									}
								}
							}
						}
					} else if (xflip && yflip) {
						int xx,yy;
						sx -= xsize;
						sy -= ysize;

						for (xx = 0; xx<xsize; xx++)
						{
							if ((sx+(xsize-1-xx) < (cliprect->max_x+1)) && (sx+(xsize-1-xx) >= cliprect->min_x))
							{
								for (yy = 0; yy<ysize; yy++)
								{
									if ((sy+(ysize-1-yy) < (cliprect->max_y+1)) && (sy+(ysize-1-yy) >= cliprect->min_y))
									{
										int pix;
										pix = decodebuffer[xsize*yy+xx];
										if (pix)
											*BITMAP_ADDR16(bitmap, sy+(ysize-1-yy), sx+(xsize-1-xx)) = pix+ NewColour; // change later
									}
								}
							}
						}
					}
				}
		}//End PriTest

		source+=4;
		}
	}
}

static tilemap *skns_tilemap_A;
static tilemap *skns_tilemap_B;

static TILE_GET_INFO( get_tilemap_A_tile_info )
{
	int code = ((skns_tilemapA_ram[tile_index] & 0x001fffff) >> 0 );
	int colr = ((skns_tilemapA_ram[tile_index] & 0x3f000000) >> 24 );
//  int pri  = ((skns_tilemapA_ram[tile_index] & 0x00e00000) >> 21 );
	int depth = (skns_v3_regs[0x0c/4] & 0x0001) << 1;
	int flags = 0;

	if(skns_tilemapA_ram[tile_index] & 0x80000000) flags |= TILE_FLIPX;
	if(skns_tilemapA_ram[tile_index] & 0x40000000) flags |= TILE_FLIPY;

	SET_TILE_INFO(
			0+depth,
			code,
			0x40+colr,
			flags);
//  tileinfo->category = pri;
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
//  int pri  = ((skns_tilemapA_ram[tile_index] & 0x00e00000) >> 21 );
	int depth = (skns_v3_regs[0x0c/4] & 0x0100) >> 7;
	int flags = 0;

	if(skns_tilemapB_ram[tile_index] & 0x80000000) flags |= TILE_FLIPX;
	if(skns_tilemapB_ram[tile_index] & 0x40000000) flags |= TILE_FLIPY;

	SET_TILE_INFO(
			1+depth,
			code,
			0x40+colr,
			flags);
//  tileinfo->category = pri;
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
		old_depthA = depthA;
		old_depthB = depthB;

		depthA = (skns_v3_regs[0x0c/4] & 0x0001) << 1;
		depthB = (skns_v3_regs[0x0c/4] & 0x0100) >> 7;

		if (old_depthA != depthA) 	tilemap_mark_all_tiles_dirty (skns_tilemap_A);
		if (old_depthB != depthB) 	tilemap_mark_all_tiles_dirty (skns_tilemap_B);

	}
}


VIDEO_START(skns)
{
	skns_tilemap_A = tilemap_create(get_tilemap_A_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,64, 64);
		tilemap_set_transparent_pen(skns_tilemap_A,0);

	skns_tilemap_B = tilemap_create(get_tilemap_B_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,64, 64);
		tilemap_set_transparent_pen(skns_tilemap_B,0);

	machine->gfx[2]->color_granularity=256;
	machine->gfx[3]->color_granularity=256;
}

static void supernova_draw_a( mame_bitmap *bitmap, const rectangle *cliprect, int tran )
{
		int enable_a  = (skns_v3_regs[0x10/4] >> 0) & 0x0001;
	UINT32 startx,starty;
	int incxx,incxy,incyx,incyy;

	if (enable_a)
	{
		startx = skns_v3_regs[0x1c/4];
			incyy  = skns_v3_regs[0x30/4]; // was xx, changed for sarukani
		incyx  = skns_v3_regs[0x2c/4];
		starty = skns_v3_regs[0x20/4];
		incxy  = skns_v3_regs[0x28/4];
			incxx  = skns_v3_regs[0x24/4]; // was yy, changed for sarukani

		if( (incxx == 1<<8) && !incxy & !incyx && (incyy == 1<<8) ) // No Roz, only scroll.
		{
			int columnscroll_a = (skns_v3_regs[0x0c/4] >> 1) & 0x0001;
			int offs;

			startx >>= 8; // Lose Floating point
			starty >>= 8;

			if(columnscroll_a) {
				tilemap_set_scroll_rows(skns_tilemap_A,1);
				tilemap_set_scroll_cols(skns_tilemap_A,0x400);

				tilemap_set_scrollx( skns_tilemap_A, 0, startx );
				for(offs=0; offs<(0x1000/4); offs++)
					tilemap_set_scrolly( skns_tilemap_A, offs, starty - (skns_v3slc_ram[offs]&0x3ff) );
			}
			else
			{
				tilemap_set_scroll_rows(skns_tilemap_A,0x400);
				tilemap_set_scroll_cols(skns_tilemap_A,1);

				tilemap_set_scrolly( skns_tilemap_A, 0, starty );
				for(offs=0; offs<(0x1000/4); offs++)
					tilemap_set_scrollx( skns_tilemap_A, offs, startx - (skns_v3slc_ram[offs]&0x3ff) );
			}
				tilemap_draw(bitmap,cliprect,skns_tilemap_A,tran ? 0 : TILEMAP_DRAW_OPAQUE,0);
		}
		else
		{
			tilemap_draw_roz(bitmap,cliprect,skns_tilemap_A,startx << 8,starty << 8,
					incxx << 8,incxy << 8,incyx << 8,incyy << 8,
					1,	/* wraparound */
						tran ? 0 : TILEMAP_DRAW_OPAQUE,0);
		}
	}
}

static void supernova_draw_b( mame_bitmap *bitmap, const rectangle *cliprect, int tran )
{
		int enable_b  = (skns_v3_regs[0x34/4] >> 0) & 0x0001;
	UINT32 startx,starty;
	int incxx,incxy,incyx,incyy;

	if (enable_b)
	{
		startx = skns_v3_regs[0x40/4];
			incyy  = skns_v3_regs[0x54/4];
		incyx  = skns_v3_regs[0x50/4];
		starty = skns_v3_regs[0x44/4];
		incxy  = skns_v3_regs[0x4c/4];
			incxx  = skns_v3_regs[0x48/4];

		if( (incxx == 1<<8) && !incxy & !incyx && (incyy == 1<<8) ) // No Roz, only scroll.
		{
			int columnscroll_b = (skns_v3_regs[0x0c/4] >> 9) & 0x0001;
			int offs;

			startx >>= 8;
			starty >>= 8;

			if(columnscroll_b) {
				tilemap_set_scroll_rows(skns_tilemap_B,1);
				tilemap_set_scroll_cols(skns_tilemap_B,0x400);

				tilemap_set_scrollx( skns_tilemap_B, 0, startx );
				for(offs=0; offs<(0x1000/4); offs++)
					tilemap_set_scrolly( skns_tilemap_B, offs, starty - (skns_v3slc_ram[offs+(0x1000/4)]&0x3ff) );
			}
			else
			{
				tilemap_set_scroll_rows(skns_tilemap_B,0x400);
				tilemap_set_scroll_cols(skns_tilemap_B,1);

				tilemap_set_scrolly( skns_tilemap_B, 0, starty );
				for(offs=0; offs<(0x1000/4); offs++)
					tilemap_set_scrollx( skns_tilemap_B, offs, startx - (skns_v3slc_ram[offs+(0x1000/4)]&0x3ff) );
			}
				tilemap_draw(bitmap,cliprect,skns_tilemap_B,tran ? 0 : TILEMAP_DRAW_OPAQUE,0);
		}
		else
		{
			tilemap_draw_roz(bitmap,cliprect,skns_tilemap_B,startx << 8,starty << 8,
					incxx << 8,incxy << 8,incyx << 8,incyy << 8,
					1,	/* wraparound */
						tran ? 0 : TILEMAP_DRAW_OPAQUE,0);
		}
	}
}

VIDEO_UPDATE(skns)
{
	int i, offset;

	UINT8 *btiles;


	palette_update(machine);

	btiles = memory_region (REGION_GFX3);

//  if (!(skns_v3_regs[0x0c/4] & 0x0100)); // if tilemap b is in 8bpp mode
	{
		if (skns_v3t_somedirty)
		{
			skns_v3t_somedirty = 0;

			/* check if & where that tile is used in the tilemap */
			for (offset=0;offset<0x4000/4;offset++)
			{
				int code = ((skns_tilemapB_ram[offset] & 0x001fffff) >> 0 );
				if (skns_v3t_dirty[code&0x3ff])
					tilemap_mark_tile_dirty(skns_tilemap_B,offset);
			}

			for (i = 0; i < 0x0400; i++)
			{
				if (skns_v3t_dirty[i] == 1)
				{
					decodechar(machine->gfx[1], i, (UINT8*)btiles);

					skns_v3t_dirty[i] = 0;
				}
			}
		}
	}

//  if (skns_v3_regs[0x0c/4] & 0x0100); // if tilemap b is in 4bpp mode
	{
		if (skns_v3t_4bpp_somedirty)
		{
			skns_v3t_4bpp_somedirty = 0;

			/* check if & where that tile is used in the tilemap */
			for (offset=0;offset<0x4000/4;offset++)
			{
				int code = ((skns_tilemapB_ram[offset] & 0x001fffff) >> 0 );
				if (skns_v3t_4bppdirty[code&0x7ff])
					tilemap_mark_tile_dirty(skns_tilemap_B,offset);
			}

			for (i = 0; i < 0x0800; i++)
			{
				if (skns_v3t_4bppdirty[i] == 1)
				{
					decodechar(machine->gfx[3], i, (UINT8*)btiles);

					skns_v3t_4bppdirty[i] = 0;
				}
			}
		}
	}

	fillbitmap(bitmap, get_black_pen(machine), cliprect);

	{
		int supernova_pri_a;
		int supernova_pri_b;
		int tran = 0;

		supernova_pri_a = skns_v3_regs[0x10/4] & 0x0002;
		supernova_pri_b = skns_v3_regs[0x34/4] & 0x0002;


		/* needed until we have the per tile priorities sorted out */
		if (!strcmp(machine->gamedrv->name,"vblokbrk") ||
			!strcmp(machine->gamedrv->name,"sarukani") ||
			!strcmp(machine->gamedrv->name,"sengekis") ||
			!strcmp(machine->gamedrv->name,"sengekij"))
		{
			supernova_pri_b = 0;
			supernova_pri_a = 1;
		}


		if (!supernova_pri_a) { supernova_draw_a(bitmap,cliprect,tran); tran = 1;}
		if (!supernova_pri_b) { supernova_draw_b(bitmap,cliprect,tran); tran = 1;}
		if (supernova_pri_a) { supernova_draw_a(bitmap,cliprect,tran); tran = 1;}
		if (supernova_pri_b) { supernova_draw_b(bitmap,cliprect,tran); tran = 1;}


	}


	skns_draw_sprites(machine, bitmap, cliprect);
	return 0;
}

VIDEO_EOF(skns)
{
	buffer_spriteram32_w(0,0,0);
}
