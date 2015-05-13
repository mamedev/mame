// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Custom blitter GA9201 KA01-0249 (120pin IC)


This is a tile-based blitter that writes to a frame buffer. The buffer is
never cleared, so stuff drawn is left over from frame to frame until it is
overwritten.

Tiles are stored in ROM and have a fixed 16x16 size. The blitter can draw them
as single sprites (composed of one or more tiles in the horizontal direction),
or as larger sprites whose tile codes are picked from a tilemap in RAM.

Sprites can be zoomed, distorted and rotated.

Shadows are supported on pen 14 (shangha3 makes heavy use of them) but it's not
clear how they are turned on and off. heberpop definitely doesn't use them,
pen 14 is supposed to be solid black (outlines).

The chip addresses 0x100000 bytes of memory, containing both the command list,
tilemap, and spare RAM to be used by the CPU.

The command list starts at a variable point in memory, set by
gfxlist_addr_w(), and always ends at 0x0fffff.

Large sprites refer to a single tilemap starting at 0x000000, the command list
contains the coordinates of the place in the tilemap to pick data from.

The commands list is processed when blitter_go_w() is written to, so
it is not processed automatically every frame.

The commands have a fixed length of 16 words. The format is as follows:

Word | Bit(s)           | Use
-----+-fedcba9876543210-+----------------
  0  | ---------------- | unused?
  1  | xxxx------------ | high bits of tile #; for tilemaps, this is applied to all tiles
  1  | ----xxxxxxxxxxxx | low bits of tile #; probably unused for tilemaps
  2  | ---xxxxxxxxx---- | x coordinate of destination top left corner
  3  | ---xxxxxxxxx---- | y coordinate of destination top left corner
  4  | ------------x--- | 0 = use code as-is  1 = fetch codes from tilemap RAM
  4  | -------------x-- | 1 = draw "compressed" tilemap, as if tiles were 8x8 instead of 16x16
  4  | --------------x- | flip y
  4  | ---------------x | flip x
  5  | --------x------- | unknown (used by blocken)
  5  | ---------xxx---- | high bits of color #; for tilemaps, this is applied to all tiles
  5  | ------------xxxx | low bits of color #; probably unused for tilemaps
  6  | xxxxxxxxxxxxxxxx | width-1 of destination rectangle    \ if *both* of these are 0,
  7  | xxxxxxxxxxxxxxxx | height-1 of destination rectangle   / disable sprite
  8  | xxxxxxxxxxxxxxxx | x coordinate*0x10 of source top left corner (tilemaps only?)
  9  | xxxxxxxxxxxxxxxx | y coordinate*0x10 of source top left corner (tilemaps only?)
  a  | xxxxxxxxxxxxxxxx | x zoom
  b  | xxxxxxxxxxxxxxxx | rotation
  c  | xxxxxxxxxxxxxxxx | rotation
  d  | xxxxxxxxxxxxxxxx | x zoom
  e  | ---------------- | unknown
  f  | ---------------- | unknown

***************************************************************************/

#include "emu.h"
#include "includes/shangha3.h"



void shangha3_state::video_start()
{
	int i;

	m_screen->register_screen_bitmap(m_rawbitmap);

	for (i = 0;i < 14;i++)
		m_drawmode_table[i] = DRAWMODE_SOURCE;
	m_drawmode_table[14] = m_do_shadows ? DRAWMODE_SHADOW : DRAWMODE_SOURCE;
	m_drawmode_table[15] = DRAWMODE_NONE;

	if (m_do_shadows)
	{
		/* Prepare the shadow table */
		for (i = 0;i < 128;i++)
			m_palette->shadow_table()[i] = i+128;
	}

	save_item(NAME(m_gfxlist_addr));
	save_item(NAME(m_rawbitmap));
}



WRITE16_MEMBER(shangha3_state::flipscreen_w)
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 7 flips screen, the rest seems to always be set to 0x7e */
		flip_screen_set(data & 0x80);

		if ((data & 0x7f) != 0x7e) popmessage("flipscreen_w %02x",data);
	}
}

WRITE16_MEMBER(shangha3_state::gfxlist_addr_w)
{
	COMBINE_DATA(&m_gfxlist_addr);
}


WRITE16_MEMBER(shangha3_state::blitter_go_w)
{
	UINT16 *shangha3_ram = m_ram;
	bitmap_ind16 &rawbitmap = m_rawbitmap;
	UINT8 *drawmode_table = m_drawmode_table;
	int offs;


	g_profiler.start(PROFILER_VIDEO);

	for (offs = m_gfxlist_addr << 3; offs < m_ram.bytes()/2; offs += 16)
	{
		int sx,sy,x,y,code,color,flipx,flipy,sizex,sizey,zoomx,zoomy;


		code = shangha3_ram[offs+1];
		color = shangha3_ram[offs+5] & 0x7f;
		flipx = shangha3_ram[offs+4] & 0x01;
		flipy = shangha3_ram[offs+4] & 0x02;
		sx = (shangha3_ram[offs+2] & 0x1ff0) >> 4;
		if (sx >= 0x180) sx -= 0x200;
		sy = (shangha3_ram[offs+3] & 0x1ff0) >> 4;
		if (sy >= 0x100) sy -= 0x200;
		sizex = shangha3_ram[offs+6];
		sizey = shangha3_ram[offs+7];
		zoomx = shangha3_ram[offs+10];
		zoomy = shangha3_ram[offs+13];

		if (flip_screen())
		{
			sx = 383 - sx - sizex;
			sy = 255 - sy - sizey;
			flipx = !flipx;
			flipy = !flipy;
		}

		if ((sizex || sizey)
				/* avoid garbage on startup */
&& sizex < 512 && sizey < 256 && zoomx < 0x1f0 && zoomy < 0x1f0)
		{
			rectangle myclip;

//if (shangha3_ram[offs+11] || shangha3_ram[offs+12])
//logerror("offs %04x: sx %04x sy %04x zoom %04x %04x %04x %04x fx %d fy %d\n",offs,sx,sy,zoomx,shangha3_ram[offs+11]),shangha3_ram[offs+12],zoomy,flipx,flipy);

			myclip.set(sx, sx + sizex, sy, sy + sizey);
			myclip &= rawbitmap.cliprect();

			if (shangha3_ram[offs+4] & 0x08)    /* tilemap */
			{
				int srcx,srcy,dispx,dispy,w,h,condensed;

				condensed = shangha3_ram[offs+4] & 0x04;

				srcx = shangha3_ram[offs+8]/16;
				srcy = shangha3_ram[offs+9]/16;
				dispx = srcx & 0x0f;
				dispy = srcy & 0x0f;

				h = (sizey+15)/16+1;
				w = (sizex+15)/16+1;

				if (condensed)
				{
					h *= 2;
					w *= 2;
					srcx /= 8;
					srcy /= 8;
				}
				else
				{
					srcx /= 16;
					srcy /= 16;
				}

				for (y = 0;y < h;y++)
				{
					for (x = 0;x < w;x++)
					{
						int dx,dy,tile;

						/* TODO: zooming algo is definitely wrong for Blocken here */
						if (condensed)
						{
							int addr = ((y+srcy) & 0x1f) +
										0x20 * ((x+srcx) & 0xff);
							tile = shangha3_ram[addr];
							dx = 8*x*(0x200-zoomx)/0x100 - dispx;
							dy = 8*y*(0x200-zoomy)/0x100 - dispy;
						}
						else
						{
							int addr = ((y+srcy) & 0x0f) +
										0x10 * ((x+srcx) & 0xff) +
										0x100 * ((y+srcy) & 0x10);
							tile = shangha3_ram[addr];
							dx = 16*x*(0x200-zoomx)/0x100 - dispx;
							dy = 16*y*(0x200-zoomy)/0x100 - dispy;
						}

						if (flipx) dx = sx + sizex-15 - dx;
						else dx = sx + dx;
						if (flipy) dy = sy + sizey-15 - dy;
						else dy = sy + dy;

						m_gfxdecode->gfx(0)->transpen(rawbitmap,myclip,
								(tile & 0x0fff) | (code & 0xf000),
								(tile >> 12) | (color & 0x70),
								flipx,flipy,
								dx,dy,15);
					}
				}
			}
			else    /* sprite */
			{
				int w;

if (zoomx <= 1 && zoomy <= 1)
	m_gfxdecode->gfx(0)->zoom_transtable(rawbitmap,myclip,
			code,
			color,
			flipx,flipy,
			sx,sy,
			0x1000000,0x1000000,
			drawmode_table);
else
{
				w = (sizex+15)/16;

				for (x = 0;x < w;x++)
				{
					m_gfxdecode->gfx(0)->zoom_transtable(rawbitmap,myclip,
							code,
							color,
							flipx,flipy,
							sx + 16*x,sy,
							(0x200-zoomx)*0x100,(0x200-zoomy)*0x100,
							drawmode_table);

					if ((code & 0x000f) == 0x0f)
						code = (code + 0x100) & 0xfff0;
					else
						code++;
				}
}
			}
		}
	}

	g_profiler.stop();
}


UINT32 shangha3_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_rawbitmap, 0, 0, 0, 0, cliprect);
	return 0;
}
