// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Kaneko RLE Compressed Sprite Hardware

   "CG24173 6186" & "CG24143 4181" (always used as a pair?)
   "SPCII-A" & "SPCII-B" in suprnova.cpp

   used by suprnova.cpp
           galpani3.cpp
           jchan.cpp

   TODO:
   - Get rid of sprite position kludges
   - Fix zooming precision/rounding (most noticeable on jchan backgrounds)
   - RLE uses fifo7200 chips

   reference
   - jchan : https://youtu.be/PJijmhRwMUk

*/

#include "emu.h"
#include "kaneko_rlespr.h"

#include "screen.h"


DEFINE_DEVICE_TYPE(KANEKO_RLE_SPRITES, kaneko_rle_sprites_device, "kaneko_rlespr", "Kaneko RLE Compressed Sprite Hardware")


kaneko_rle_sprites_device::kaneko_rle_sprites_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, KANEKO_RLE_SPRITES, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this)
	, m_sprite_kludge_x(0)
	, m_sprite_kludge_y(0)
	, m_buffer(0)
{
}

void kaneko_rle_sprites_device::device_start()
{
	m_decodebuffer = make_unique_clear<u8[]>(DECODE_BUFFER_SIZE);
	for (int i = 0; i < 2; i++)
	{
		m_sprite_bitmap[i].allocate(512, 512); // 512x512 16 bit double buffered
		save_item(NAME(m_sprite_bitmap[i]), i);
	}

	save_pointer(NAME(m_decodebuffer), DECODE_BUFFER_SIZE);
	save_item(NAME(m_buffer));
	//logerror("kaneko_rle_sprites_device::device_start()\n");
}

void kaneko_rle_sprites_device::device_reset()
{
	//logerror("kaneko_rle_sprites_device::device_reset()\n");
}

int kaneko_rle_sprites_device::rle_decode(int romoffset, int size)
{
	int decodeoffset = 0;

	while (size > 0)
	{
		u8 code = read_byte((romoffset++) & ROM_ADDRESS_MASK);
		size -= (code & 0x7f) + 1;
		if (code & 0x80) /* (code & 0x7f) normal values will follow */
		{
			code &= 0x7f;
			do
			{
				m_decodebuffer[(decodeoffset++) & DECODE_BUFFER_MASK] = read_byte((romoffset++) & ROM_ADDRESS_MASK);
				code--;
			} while (code != 0xff);
		}
		else  /* repeat next value (code & 0x7f) times */
		{
			u8 const val = read_byte((romoffset++) & ROM_ADDRESS_MASK);
			do {
				m_decodebuffer[(decodeoffset++) & DECODE_BUFFER_MASK] = val;
				code--;
			} while (code != 0xff);
		}
	}
	return romoffset & ROM_ADDRESS_MASK;
}

void kaneko_rle_sprites_device::set_sprite_kludge(int x, int y)
{
	m_sprite_kludge_x = x << 6;
	m_sprite_kludge_y = y << 6;
}

/* Zooming blitter, zoom is by way of both source and destination offsets */
/* We are working in .16 fixed point if you hadn't guessed */

#define z_decls(step)                        \
	u32 zxs = 0x10000 - (zx_m);              \
	u32 zxd = 0x10000 - (zx_s);              \
	u32 zys = 0x10000 - (zy_m);              \
	u32 zyd = 0x10000 - (zy_s);              \
	u32 bxs = 0, bys = 0, xs, ys;            \
	int xd, yd, old, old2;                   \
	int step_spr = step;                     \
	rectangle clip;                          \
	clip.min_x = cliprect.min_x << 16;       \
	clip.max_x = (cliprect.max_x + 1) << 16; \
	clip.min_y = cliprect.min_y << 16;       \
	clip.max_y = (cliprect.max_y + 1) << 16; \
	sx <<= 16;                               \
	sy <<= 16;                               \
	x <<= 10;                                \
	y <<= 10;

#define z_clamp_x_min()           \
	if (x < clip.min_x)           \
	{                             \
		do                        \
		{                         \
			bxs += zxs;           \
			x += zxd;             \
		} while (x < clip.min_x); \
	}

#define z_clamp_x_max()            \
	if (x >= clip.max_x)           \
	{                              \
		do                         \
		{                          \
			bxs += zxs;            \
			x -= zxd;              \
		} while (x >= clip.max_x); \
	}

#define z_clamp_y_min()                \
	if (y < clip.min_y)                \
	{                                  \
		do                             \
		{                              \
			bys += zys;                \
			y += zyd;                  \
		} while (y < clip.min_y);      \
		src += (bys >> 16) * step_spr; \
	}

#define z_clamp_y_max()                \
	if (y >= clip.max_y)               \
	{                                  \
		do                             \
		{                              \
			bys += zys;                \
			y -= zyd;                  \
		} while (y >= clip.max_y);     \
		src += (bys >> 16) * step_spr; \
	}

#define z_loop_x()                     \
	xs = bxs;                          \
	xd = x;                            \
	while (xs < sx && xd < clip.max_x)

#define z_loop_x_flip()                 \
	xs = bxs;                           \
	xd = x;                             \
	while (xs < sx && xd >= clip.min_x)

#define z_loop_y()                     \
	ys = bys;                          \
	yd = y;                            \
	while (ys < sy && yd < clip.max_y)

#define z_loop_y_flip()                 \
	ys = bys;                           \
	yd = y;                             \
	while (ys < sy && yd >= clip.min_y)

#define z_draw_pixel()                                 \
	u8 const val = src[(xs >> 16) & 0x3f];             \
	if (val)                                           \
		bitmap.pix(yd >> 16, xd >> 16) = val + colour;

#define z_x_dst(op)                    \
	old = xd;                          \
	do                                 \
	{                                  \
		xs += zxs;                     \
		xd op zxd;                     \
	} while (!((xd ^ old) & ~0xffff));

#define z_y_dst(op)                    \
	old = yd;                          \
	old2 = ys;                         \
	do                                 \
	{                                  \
		ys += zys;                     \
		yd op zyd;                     \
	} while (!((yd ^ old) & ~0xffff)); \
	while ((ys ^ old2) & ~0xffff)      \
	{                                  \
		src += step_spr;               \
		old2 += 0x10000;               \
	}

static void blit_nf_z(bitmap_ind16 &bitmap, const rectangle &cliprect, const u8 *src, int x, int y, int sx, int sy, u16 zx_m, u16 zx_s, u16 zy_m, u16 zy_s, int colour)
{
	z_decls(sx);
	z_clamp_x_min();
	z_clamp_y_min();
	z_loop_y()
	{
		z_loop_x()
		{
			z_draw_pixel();
			z_x_dst(+=);
		}
		z_y_dst(+=);
	}
}

static void blit_fy_z(bitmap_ind16 &bitmap, const rectangle &cliprect, const u8 *src, int x, int y, int sx, int sy, u16 zx_m, u16 zx_s, u16 zy_m, u16 zy_s, int colour)
{
	z_decls(sx);
	z_clamp_x_min();
	z_clamp_y_max();
	z_loop_y_flip()
	{
		z_loop_x()
		{
			z_draw_pixel();
			z_x_dst(+=);
		}
		z_y_dst(-=);
	}
}

static void blit_fx_z(bitmap_ind16 &bitmap, const rectangle &cliprect, const u8 *src, int x, int y, int sx, int sy, u16 zx_m, u16 zx_s, u16 zy_m, u16 zy_s, int colour)
{
	z_decls(sx);
	z_clamp_x_max();
	z_clamp_y_min();
	z_loop_y()
	{
		z_loop_x_flip()
		{
			z_draw_pixel();
			z_x_dst(-=);
		}
		z_y_dst(+=);
	}
}

static void blit_fxy_z(bitmap_ind16 &bitmap, const rectangle &cliprect, const u8 *src, int x, int y, int sx, int sy, u16 zx_m, u16 zx_s, u16 zy_m, u16 zy_s, int colour)
{
	z_decls(sx);
	z_clamp_x_max();
	z_clamp_y_max();
	z_loop_y_flip()
	{
		z_loop_x_flip()
		{
			z_draw_pixel();
			z_x_dst(-=);
		}
		z_y_dst(-=);
	}
}

static void (*const blit_z[4])(bitmap_ind16 &bitmap, const rectangle &cliprect, const u8 *src, int x, int y, int sx, int sy, u16 zx_m, u16 zx_s, u16 zy_m, u16 zy_s, int colour) =
{
	blit_nf_z,
	blit_fy_z,
	blit_fx_z,
	blit_fxy_z,
};

void kaneko_rle_sprites_device::draw_sprites(const rectangle &cliprect, u32* spriteram_source, size_t spriteram_size, u32* sprite_regs)
{
	m_buffer ^= 1;
	rectangle clip = cliprect;
	clip &= m_sprite_bitmap[m_buffer].cliprect();
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

0x08  ZZZZ ZZ--  zzzz zz--  xxxx xxxx  xxxx xxxx

	  Z = horizontal zoom table
	  z = horizontal zoom subtable
	  x = x position (10.6 fixed point)

0x0C  ZZZZ ZZ--  zzzz zz--  yyyy yyyy  yyyy yyyy

	  Z = vertical zoom table
	  z = vertical zoom subtable
	  x = y position (10.6 fixed point)

	**- End of Comments -*/

	/* sprite ram start / end is not really fixed registers change it */

	//logerror("addr %08x\n", sprite_regs[0x14/4]);


	u32 const *source = spriteram_source;
	u32 const *finish = source + spriteram_size / 4;

	/* galpani3 uses sprite trail effect (disable clearing sprite bitmap) */
	bool const clear_bitmap = BIT(~sprite_regs[0x04/4], 2); // RWR1
	bool const disabled = BIT(sprite_regs[0x04/4], 3); // RWR1

	if (clear_bitmap)
	{
		m_sprite_bitmap[m_buffer].fill(0, clip);
	}

	if ((!disabled))
	{
		bool const group_enable = BIT(sprite_regs[0x00/4], 6); // RWR0

		/* Sengekis uses global flip */
		u8 const sprite_flip = (sprite_regs[0x04/4] & 0x03); // RWR1

		s16 sprite_y_scroll = s16((sprite_regs[0x08/4] & 0x7fff) << 1) >> 1; // RWR2
		s16 sprite_x_scroll = s16((sprite_regs[0x10/4] & 0x7fff) << 1) >> 1; // RWR4

		s16 group_x_offset[4] = {0};
		int group_y_offset[4] = {0};
		group_x_offset[0] = (sprite_regs[0x18/4] & 0xffff); // RWR6
		group_y_offset[0] = (sprite_regs[0x1c/4] & 0xffff); // RWR7

		group_x_offset[1] = (sprite_regs[0x20/4] & 0xffff); // RWR8
		group_y_offset[1] = (sprite_regs[0x24/4] & 0xffff); // RWR9

		group_x_offset[2] = (sprite_regs[0x28/4] & 0xffff); // RWR10
		group_y_offset[2] = (sprite_regs[0x2c/4] & 0xffff); // RWR11

		group_x_offset[3] = (sprite_regs[0x30/4] & 0xffff); // RWR12
		group_y_offset[3] = (sprite_regs[0x34/4] & 0xffff); // RWR13

	//  popmessage ("x %08x y %08x x2 %08x y2 %08x",sprite_x_scroll, sprite_y_scroll,group_x_offset[1], group_y_offset[1]);
	//  popmessage("%d %d %d %d A:%d B:%d", m_sprite_kludge_x, m_sprite_kludge_y, sprite_x_scroll, sprite_y_scroll, (skns_pal_regs[0x00/4] & 0x7000) >> 12, (skns_pal_regs[0x10/4] & 0x7000) >> 12);
	//  if (keyboard_pressed(KEYCODE_Q)) m_sprite_kludge_x++;
	//  if (keyboard_pressed(KEYCODE_W)) m_sprite_kludge_x--;
	//  if (keyboard_pressed(KEYCODE_E)) m_sprite_kludge_y++;
	//  if (keyboard_pressed(KEYCODE_D)) m_sprite_kludge_y--;

		// Tilemap Pri/enables
	//  popmessage("A: %x %x B: %x %x", skns_v3_regs[0x10/4]>>3, skns_v3_regs[0x10/4]&7, skns_v3_regs[0x34/4]>>3, skns_v3_regs[0x34/4]&7);

		/* Seems that sprites are consistently off by a fixed no. of pixels in different games
		   (Patterns emerge through Manufacturer/Date/Orientation) */
		sprite_x_scroll += m_sprite_kludge_x;
		sprite_y_scroll += m_sprite_kludge_y;

		int pri = 0, colour = 0;
		int endromoffs = 0;
		s16 xpos = 0, ypos = 0;

		while (source < finish)
		{
			u8 xflip = BIT(source[0], 9);
			u8 yflip = BIT(source[0], 8);

			int ysize = (source[0] & 0x30000000) >> 28;
			int xsize = (source[0] & 0x03000000) >> 24;
			xsize++;
			ysize++;

			xsize *= 16;
			ysize *= 16;

			int const size = xsize * ysize;

			u8 const joint = (source[0] & 0x0000e000) >> 13;

			if (!BIT(joint, 0))
			{
				xpos = (source[2] & 0x0000ffff);
				ypos = (source[3] & 0x0000ffff);

				xpos += sprite_x_scroll; // Global offset
				ypos += sprite_y_scroll;

				if (group_enable)
				{
					u8 const group_number = (source[0] & 0x00001800) >> 11;

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
				xpos += (source[2] & 0x0000ffff);
				ypos += (source[3] & 0x0000ffff);
			}

			/* Local sprite offset (for taking flip into account and drawing offset) */
			int sx = xpos;
			int sy = ypos;

			/* Global Sprite Flip (sengekis) */
			if (BIT(sprite_flip, 1))
			{
				xflip ^= 1;
				sx = ((screen().visible_area().max_x + 1) << 6) - sx;
			}
			if (BIT(sprite_flip, 0))
			{
				yflip ^= 1;
				sy = ((screen().visible_area().max_y + 1) << 6) - sy;
			}

			/* Palette linking */
			if (!BIT(joint, 1))
			{
				colour = (source[0] & 0x0000003f) >> 0;
			}

			int romoffset;
			/* Priority and Tile linking */
			if (!BIT(joint, 2))
			{
				romoffset = (source[1] & 0x07ffffff) >> 0;
				pri = (source[0] & 0x000000c0) >> 6;
			}
			else
			{
				romoffset = endromoffs;
			}

			bool const grow = BIT(source[0], 23);

			u16 zoomx_m, zoomx_s, zoomy_m, zoomy_s;
			if (!grow)
			{
				zoomx_m = (source[2] & 0xff000000) >> 16;
				zoomx_s = (source[2] & 0x00ff0000) >> 8;
				zoomy_m = (source[3] & 0xff000000) >> 16;
				zoomy_s = (source[3] & 0x00ff0000) >> 8;
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
				zoomx_s = (source[2] & 0xffff0000) >> 16;
				zoomy_m = 0;
				zoomy_s = (source[3] & 0xffff0000) >> 16;
			}

			endromoffs = rle_decode(romoffset, size);

			// in Cyvern

			//  train in tunnel pri = 0x00
			//  nothing?         = 0x01
			//  players etc. pri = 0x02
			//  pickups etc. pri = 0x03

			// PriTest
//          if (!((keyboard_pressed(KEYCODE_Q)&&(pri==0)) || (keyboard_pressed(KEYCODE_W)&&(pri==1)) || (keyboard_pressed(KEYCODE_E)&&(pri==2)) || (keyboard_pressed(KEYCODE_D)&&(pri==3))))
//          if (!(keyboard_pressed(KEYCODE_Q) && ((source[0] & 0x00800000)>>24)))

			{
				u16 const out_colour = (colour << 8) | (pri << 14);

				if (zoomx_m || zoomx_s || zoomy_m || zoomy_s)
				{
					blit_z[(xflip << 1) | yflip](m_sprite_bitmap[m_buffer], clip, m_decodebuffer.get(), sx, sy, xsize, ysize, zoomx_m, zoomx_s, zoomy_m, zoomy_s, out_colour);
				}
				else
				{
					sx >>= 6;
					sy >>= 6;
					int startx, incx;
					if (xflip)
					{
						sx -= xsize;
						startx = xsize - 1;
						incx = -1;
					}
					else
					{
						startx = 0;
						incx = 1;
					}
					int starty, incy;
					if (yflip)
					{
						sy -= ysize;
						starty = ysize - 1;
						incy = -1;
					}
					else
					{
						starty = 0;
						incy = 1;
					}

					for (int yy = 0, drawy = starty; yy < ysize; yy++, drawy += incy)
					{
						if ((sy + drawy < (clip.max_y + 1)) && (sy + drawy >= clip.min_y))
						{
							u16 *const dst = &m_sprite_bitmap[m_buffer].pix(sy + drawy);
							for (int xx = 0, drawx = startx; xx < xsize; xx++, drawx += incx)
							{
								if ((sx + drawx < (clip.max_x + 1)) && (sx + drawx >= clip.min_x))
								{
									u8 const pix = m_decodebuffer[xsize * yy + xx];
									if (pix)
										dst[sx + drawx] = pix + out_colour; // change later
								}
							}
						}
					}
				}
			}//End PriTest

			source += 4;
		}
	}
}
