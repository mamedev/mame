// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Super Kaneko Nova System Sprites

   "CG24173 6186" & "CG24143 4181" (always used as a pair?)

   used by suprnova.c
           galpani3.c
           jchan.c

   TODO:
   - Get rid of sprite position kludges
   - Fix zooming precision/rounding (most noticeable on jchan backgrounds)
   - RLE uses fifo7200 chips

*/

#include "emu.h"
#include "sknsspr.h"

const device_type SKNS_SPRITE = &device_creator<sknsspr_device>;


sknsspr_device::sknsspr_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SKNS_SPRITE, "SKNS Sprite", tag, owner, clock, "sknsspr", __FILE__),
		device_video_interface(mconfig, *this)
{
}

void sknsspr_device::device_start()
{
	//printf("sknsspr_device::device_start()\n");
}

void sknsspr_device::device_reset()
{
	//printf("sknsspr_device::device_reset()\n");
}

int sknsspr_device::skns_rle_decode ( int romoffset, int size, UINT8*gfx_source, size_t gfx_length )
{
	UINT8 *src = gfx_source;
	size_t srcsize = gfx_length;
	UINT8 *dst = decodebuffer;
	int decodeoffset = 0;

	while(size>0) {
		UINT8 code = src[(romoffset++)%srcsize];
		size -= (code & 0x7f) + 1;
		if(code & 0x80) { /* (code & 0x7f) normal values will follow */
			code &= 0x7f;
			do {
				dst[(decodeoffset++)%SUPRNOVA_DECODE_BUFFER_SIZE] = src[(romoffset++)%srcsize];
				code--;
			} while(code != 0xff);
		} else {  /* repeat next value (code & 0x7f) times */
			UINT8 val = src[(romoffset++)%srcsize];
			do {
				dst[(decodeoffset++)%SUPRNOVA_DECODE_BUFFER_SIZE] = val;
				code--;
			} while(code != 0xff);
		}
	}
	return &src[romoffset%srcsize]-gfx_source;
}

void sknsspr_device::skns_sprite_kludge(int x, int y)
{
	sprite_kludge_x = x;
	sprite_kludge_y = y;
}

/* Zooming blitter, zoom is by way of both source and destination offsets */
/* We are working in .6 fixed point if you hadn't guessed */

#define z_decls(step)               \
	UINT16 zxs = 0x40-(zx_m>>2);            \
	UINT16 zxd = 0x40-(zx_s>>2);        \
	UINT16 zys = 0x40-(zy_m>>2);            \
	UINT16 zyd = 0x40-(zy_s>>2);        \
	int xs, ys, xd, yd, old, old2;      \
	int step_spr = step;                \
	int bxs = 0, bys = 0;               \
	rectangle clip;                 \
	clip.min_x = cliprect.min_x<<6;                 \
	clip.max_x = (cliprect.max_x+1)<<6;                 \
	clip.min_y = cliprect.min_y<<6;                 \
	clip.max_y = (cliprect.max_y+1)<<6;                 \
	sx <<= 6;                   \
	sy <<= 6;                   \
	x <<= 6;                    \
	y <<= 6;

#define z_clamp_x_min()         \
	if(x < clip.min_x) {                    \
		do {                    \
			bxs += zxs;             \
			x += zxd;                   \
		} while(x < clip.min_x);                \
	}

#define z_clamp_x_max()         \
	if(x > clip.max_x) {                \
		do {                    \
			bxs += zxs;             \
			x -= zxd;                   \
		} while(x > clip.max_x);                \
	}

#define z_clamp_y_min()         \
	if(y < clip.min_y) {                    \
		do {                    \
			bys += zys;             \
			y += zyd;                   \
		} while(y < clip.min_y);                \
		src += (bys>>6)*step_spr;           \
	}

#define z_clamp_y_max()         \
	if(y > clip.max_y) {                \
		do {                    \
			bys += zys;             \
			y -= zyd;                   \
		} while(y > clip.max_y);                \
		src += (bys>>6)*step_spr;           \
	}

#define z_loop_x()          \
	xs = bxs;                   \
	xd = x;                 \
	while(xs < sx && xd <= clip.max_x)

#define z_loop_x_flip()         \
	xs = bxs;                   \
	xd = x;                 \
	while(xs < sx && xd >= clip.min_x)

#define z_loop_y()          \
	ys = bys;                   \
	yd = y;                 \
	while(ys < sy && yd <= clip.max_y)

#define z_loop_y_flip()         \
	ys = bys;                   \
	yd = y;                 \
	while(ys < sy && yd >= clip.min_y)

#define z_draw_pixel()              \
	UINT8 val = src[xs >> 6];           \
	if(val)                 \
		bitmap.pix16(yd>>6, xd>>6) = val + colour;

#define z_x_dst(op)         \
	old = xd;                   \
	do {                        \
		xs += zxs;                  \
		xd op zxd;                  \
	} while(!((xd^old) & ~0x3f));

#define z_y_dst(op)         \
	old = yd;                   \
	old2 = ys;                  \
	do {                        \
		ys += zys;                  \
		yd op zyd;                  \
	} while(!((yd^old) & ~0x3f));           \
	while((ys^old2) & ~0x3f) {          \
		src += step_spr;                \
		old2 += 0x40;               \
	}

static void blit_nf_z(bitmap_ind16 &bitmap, const rectangle &cliprect, const UINT8 *src, int x, int y, int sx, int sy, UINT16 zx_m, UINT16 zx_s, UINT16 zy_m, UINT16 zy_s, int colour)
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

static void blit_fy_z(bitmap_ind16 &bitmap, const rectangle &cliprect, const UINT8 *src, int x, int y, int sx, int sy, UINT16 zx_m, UINT16 zx_s, UINT16 zy_m, UINT16 zy_s, int colour)
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

static void blit_fx_z(bitmap_ind16 &bitmap, const rectangle &cliprect, const UINT8 *src, int x, int y, int sx, int sy, UINT16 zx_m, UINT16 zx_s, UINT16 zy_m, UINT16 zy_s, int colour)
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

static void blit_fxy_z(bitmap_ind16 &bitmap, const rectangle &cliprect, const UINT8 *src, int x, int y, int sx, int sy, UINT16 zx_m, UINT16 zx_s, UINT16 zy_m, UINT16 zy_s, int colour)
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

static void (*const blit_z[4])(bitmap_ind16 &bitmap, const rectangle &cliprect, const UINT8 *src, int x, int y, int sx, int sy, UINT16 zx_m, UINT16 zx_s, UINT16 zy_m, UINT16 zy_s, int colour) = {
	blit_nf_z,
	blit_fy_z,
	blit_fx_z,
	blit_fxy_z,
};

void sknsspr_device::skns_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, UINT32* spriteram_source, size_t spriteram_size, UINT8* gfx_source, size_t gfx_length, UINT32* sprite_regs)
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

	//printf ("addr %08x\n", (sprite_regs[0x14/4]));


	UINT32 *source = spriteram_source;
	UINT32 *finish = source + spriteram_size/4;

	int group_x_offset[4];
	int group_y_offset[4];
	int group_enable;
	int group_number;
	int sprite_flip;
	int sprite_x_scroll;
	int sprite_y_scroll;
	int disabled = sprite_regs[0x04/4] & 0x08; // RWR1
	int xsize,ysize, size, xpos=0,ypos=0, pri=0, romoffset, colour=0, xflip,yflip, joint;
	int sx,sy;
	int endromoffs=0, gfxlen;
	int grow;
	UINT16 zoomx_m, zoomx_s, zoomy_m, zoomy_s;


	if ((!disabled)){
		group_enable    = (sprite_regs[0x00/4] & 0x0040) >> 6; // RWR0

		/* Sengekis uses global flip */
		sprite_flip = (sprite_regs[0x04/4] & 0x03); // RWR1

		sprite_y_scroll = ((sprite_regs[0x08/4] & 0x7fc0) >> 6); // RWR2
		sprite_x_scroll = ((sprite_regs[0x10/4] & 0x7fc0) >> 6); // RWR4
		if (sprite_y_scroll&0x100) sprite_y_scroll -= 0x200; // Signed
		if (sprite_x_scroll&0x100) sprite_x_scroll -= 0x200; // Signed

		group_x_offset[0] = (sprite_regs[0x18/4] & 0xffc0) >> 6; // RWR6
		group_y_offset[0] = (sprite_regs[0x1c/4] & 0xffc0) >> 6; // RWR7
		if (group_x_offset[0]&0x200) group_x_offset[0] -= 0x400; // Signed
		if (group_y_offset[0]&0x200) group_y_offset[0] -= 0x400; // Signed

		group_x_offset[1] = (sprite_regs[0x20/4] & 0xffc0) >> 6; // RWR8
		group_y_offset[1] = (sprite_regs[0x24/4] & 0xffc0) >> 6; // RWR9
		if (group_x_offset[1]&0x200) group_x_offset[1] -= 0x400; // Signed
		if (group_y_offset[1]&0x200) group_y_offset[1] -= 0x400; // Signed

		group_x_offset[2] = (sprite_regs[0x28/4] & 0xffc0) >> 6; // RWR10
		group_y_offset[2] = (sprite_regs[0x2c/4] & 0xffc0) >> 6; // RWR11
		if (group_x_offset[2]&0x200) group_x_offset[2] -= 0x400; // Signed
		if (group_y_offset[2]&0x200) group_y_offset[2] -= 0x400; // Signed

		group_x_offset[3] = (sprite_regs[0x30/4] & 0xffc0) >> 6; // RWR12
		group_y_offset[3] = (sprite_regs[0x34/4] & 0xffc0) >> 6; // RWR13
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


		gfxlen = gfx_length;
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
				sx = m_screen->visible_area().max_x+1 - sx;
			}
			if (sprite_flip&1)
			{
				yflip ^= 1;
				sy = m_screen->visible_area().max_y+1 - sy;
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

			grow = (source[0]>>23) & 1;

			if (!grow)
			{
				zoomx_m = (source[2] >> 24)&0x00fc;
				zoomx_s = (source[2] >> 16)&0x00fc;
				zoomy_m = (source[3] >> 24)&0x00fc;
				zoomy_s = (source[3] >> 16)&0x00fc;
			}
			else
			{
				// sengekis uses this on sprites which are shrinking as they head towards the ground
				// it's also used on the input test of Gals Panic S2
				//
				// it appears to offer a higher precision 'shrink' mode (although I'm not entirely
				//  convinced this implementation is correct because we simply end up ignoring
				//  part of the data)
				zoomx_m = 0;
				zoomx_s = (source[2] >> 24)&0x00fc;
				zoomy_m = 0;
				zoomy_s = (source[3] >> 24)&0x00fc;


			}


			romoffset &= gfxlen-1;

			endromoffs = skns_rle_decode ( romoffset, size, gfx_source, gfx_length );

			// in Cyvern

			//  train in tunnel pri = 0x00
			//  nothing?         = 0x01
			//  players etc. pri = 0x02
			//  pickups etc. pri = 0x03

			// PriTest
//          if(!( (keyboard_pressed(KEYCODE_Q)&&(pri==0)) || (keyboard_pressed(KEYCODE_W)&&(pri==1)) || (keyboard_pressed(KEYCODE_E)&&(pri==2)) || (keyboard_pressed(KEYCODE_D)&&(pri==3)) ))
//          if( !(keyboard_pressed(KEYCODE_Q) && ((source[0] & 0x00800000)>>24)) )



			{
				int NewColour = (colour<<8) | (pri << 14);

				if(zoomx_m || zoomx_s || zoomy_m || zoomy_s)
				{
					blit_z[ (xflip<<1) | yflip ](bitmap, cliprect, decodebuffer, sx, sy, xsize, ysize, zoomx_m, zoomx_s, zoomy_m, zoomy_s, NewColour);
				}
				else
				{
					if (!xflip && !yflip) {
						int xx,yy;

						for (xx = 0; xx<xsize; xx++)
						{
							if ((sx+xx < (cliprect.max_x+1)) && (sx+xx >= cliprect.min_x))
							{
								for (yy = 0; yy<ysize; yy++)
								{
									if ((sy+yy < (cliprect.max_y+1)) && (sy+yy >= cliprect.min_y))
									{
										int pix;
										pix = decodebuffer[xsize*yy+xx];
										if (pix)
											bitmap.pix16(sy+yy, sx+xx) = pix+ NewColour; // change later
									}
								}
							}
						}
					} else if (!xflip && yflip) {
						int xx,yy;
						sy -= ysize;

						for (xx = 0; xx<xsize; xx++)
						{
							if ((sx+xx < (cliprect.max_x+1)) && (sx+xx >= cliprect.min_x))
							{
								for (yy = 0; yy<ysize; yy++)
								{
									if ((sy+(ysize-1-yy) < (cliprect.max_y+1)) && (sy+(ysize-1-yy) >= cliprect.min_y))
									{
										int pix;
										pix = decodebuffer[xsize*yy+xx];
										if (pix)
											bitmap.pix16(sy+(ysize-1-yy), sx+xx) = pix+ NewColour; // change later
									}
								}
							}
						}
					} else if (xflip && !yflip) {
						int xx,yy;
						sx -= xsize;

						for (xx = 0; xx<xsize; xx++)
						{
							if ( (sx+(xsize-1-xx) < (cliprect.max_x+1)) && (sx+(xsize-1-xx) >= cliprect.min_x))
							{
								for (yy = 0; yy<ysize; yy++)
								{
									if ((sy+yy < (cliprect.max_y+1)) && (sy+yy >= cliprect.min_y))
									{
										int pix;
										pix = decodebuffer[xsize*yy+xx];
										if (pix)
											bitmap.pix16(sy+yy, sx+(xsize-1-xx)) = pix+ NewColour; // change later
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
							if ((sx+(xsize-1-xx) < (cliprect.max_x+1)) && (sx+(xsize-1-xx) >= cliprect.min_x))
							{
								for (yy = 0; yy<ysize; yy++)
								{
									if ((sy+(ysize-1-yy) < (cliprect.max_y+1)) && (sy+(ysize-1-yy) >= cliprect.min_y))
									{
										int pix;
										pix = decodebuffer[xsize*yy+xx];
										if (pix)
											bitmap.pix16(sy+(ysize-1-yy), sx+(xsize-1-xx)) = pix+ NewColour; // change later
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
