// Video System Sprites
// todo:
//  unify these functions (secondary stage tile lookup differs between games, use callback)

//  according to gstriker this is probably the Fujitsu CG10103

// Aero Fighters (newer hardware)
// Quiz & Variety Sukusuku Inufuku
// 3 On 3 Dunk Madness
// Super Slams
// Formula 1 Grand Prix 2
// (Lethal) Crash Race
// Grand Striker
// V Goal Soccer
// Tecmo World Cup '94
// Tao Taido

#include "emu.h"
#include "vsystem_spr.h"


const device_type VSYSTEM_SPR = &device_creator<vsystem_spr_device>;

vsystem_spr_device::vsystem_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, VSYSTEM_SPR, "vsystem_spr_device", tag, owner, clock)
{
}



void vsystem_spr_device::device_start()
{

}

void vsystem_spr_device::device_reset()
{

}



// zooming is wrong for 3on3dunk ... suprslam implementation is probably better
void vsystem_spr_device::draw_sprites_inufuku( UINT16* spriteram, int spriteram_bytes, UINT16* spriteram2, running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;
	int end = 0;

	for (offs = 0; offs < (spriteram_bytes / 16 ); offs++)
	{
		if (spriteram[offs] & 0x4000) break;
	}
	end = offs;

	for (offs = end - 1; offs >= 0; offs--)
	{
		if ((spriteram[offs] & 0x8000) == 0x0000)
		{
			int attr_start;
			int map_start;
			int ox, oy, x, y, xsize, ysize, zoomx, zoomy, flipx, flipy, color;
			int priority, priority_mask;

			attr_start = 4 * (spriteram[offs] & 0x03ff);

			/*
                attr_start + 0x0000
                ---- ---x xxxx xxxx oy
                ---- xxx- ---- ---- ysize
                xxxx ---- ---- ---- zoomy

                attr_start + 0x0001
                ---- ---x xxxx xxxx ox
                ---- xxx- ---- ---- xsize
                xxxx ---- ---- ---- zoomx

                attr_start + 0x0002
                -x-- ---- ---- ---- flipx
                x--- ---- ---- ---- flipy
                --xx xxxx ---- ---- color
                --xx ---- ---- ---- priority?
                ---- ---- xxxx xxxx unused?

                attr_start + 0x0003
                -xxx xxxx xxxx xxxx map start
                x--- ---- ---- ---- unused?
            */

			ox = (spriteram[attr_start + 1] & 0x01ff) + 0;
			xsize = (spriteram[attr_start + 1] & 0x0e00) >> 9;
			zoomx = (spriteram[attr_start + 1] & 0xf000) >> 12;
			oy = (spriteram[attr_start + 0] & 0x01ff) + 1;
			ysize = (spriteram[attr_start + 0] & 0x0e00) >> 9;
			zoomy = (spriteram[attr_start + 0] & 0xf000) >> 12;
			flipx = spriteram[attr_start + 2] & 0x4000;
			flipy = spriteram[attr_start + 2] & 0x8000;
			color = (spriteram[attr_start + 2] & 0x3f00) >> 8;
			priority = (spriteram[attr_start + 2] & 0x3000) >> 12;
			map_start = (spriteram[attr_start + 3] & 0x7fff) << 1;

			switch (priority)
			{
				default:
				case 0:	priority_mask = 0x00; break;
				case 3:	priority_mask = 0xfe; break;
				case 2:	priority_mask = 0xfc; break;
				case 1:	priority_mask = 0xf0; break;
			}

			ox += (xsize * zoomx + 2) / 4;
			oy += (ysize * zoomy + 2) / 4;

			zoomx = 32 - zoomx;
			zoomy = 32 - zoomy;

			for (y = 0; y <= ysize; y++)
			{
				int sx, sy;

				if (flipy)
					sy = (oy + zoomy * (ysize - y) / 2 + 16) & 0x1ff;
				else
					sy = (oy + zoomy * y / 2 + 16) & 0x1ff;

				for (x = 0; x <= xsize; x++)
				{
					int code;

					if (flipx)
						sx = (ox + zoomx * (xsize - x) / 2 + 16) & 0x1ff;
					else
						sx = (ox + zoomx * x / 2 + 16) & 0x1ff;

					code  = ((spriteram2[map_start] & 0x0007) << 16) + spriteram2[map_start + 1];

					pdrawgfxzoom_transpen(bitmap, cliprect, machine.gfx[2],
							code,
							color,
							flipx, flipy,
							sx - 16, sy - 16,
							zoomx << 11, zoomy << 11,
							machine.priority_bitmap,priority_mask, 15);

					map_start += 2;
				}
			}
		}
	}
}



/* todo, fix zooming correctly, it's _not_ like aerofgt */
void vsystem_spr_device::draw_sprites_suprslam( UINT16* spriteram, int spriteram_bytes, UINT16* spriteram2, running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	/* SPRITE INFO

    Video System hardware, like aerofgt etc.

    the sprites use 2 areas of ram, one containing a spritelist + sprite attributes, the other
    contains the sprite tile #'s to use

    sprite attribute info (4 words per sprite)

    |  ZZZZ hhhy yyyy yyyy  |  zzzz wwwx xxxx xxxx  |  -fpp pppp ---- ----  |  -ooo oooo oooo oooo  |

    x  = x position
    y  = y position
    w  = width
    h  = height
    zZ = y zoom / x zoom
    f  = xflip
    p  = palette / colour
    o  = offset to tile data in other ram area

    */

	gfx_element *gfx = machine.gfx[1];
	UINT16 *source = spriteram;
	UINT16 *source2 = spriteram;
	UINT16 *finish = source + 0x2000/2;

	while (source < finish)
	{
		UINT32 sprnum = source[0] & 0x03ff;
		if (source[0] == 0x4000) break;

		sprnum *= 4;

		source++;
		/* DRAW START */
		{
			int ypos = source2[sprnum + 0] & 0x1ff;
			int high = (source2[sprnum + 0] & 0x0e00) >> 9;
			int yzoom = (source2[sprnum + 0] & 0xf000) >> 12;

			int xpos = source2[sprnum + 1] & 0x1ff;
			int wide = (source2[sprnum + 1] & 0x0e00) >> 9;
			int xzoom = (source2[sprnum + 1] & 0xf000) >> 12;

			int col = (source2[sprnum + 2] & 0x3f00) >> 8;
			int flipx = (source2[sprnum + 2] & 0x4000) >> 14;
//          int flipy = (source2[sprnum + 2] & 0x8000) >> 15;

			int word_offset = source2[sprnum + 3] & 0x7fff;
			int xcnt, ycnt;

			int loopno = 0;

			xzoom = 32 - xzoom;
			yzoom = 32 - yzoom;

			if (ypos > 0xff) ypos -=0x200;

			for (ycnt = 0; ycnt < high+1; ycnt ++)
			{
				if (!flipx)
				{
					for (xcnt = 0; xcnt < wide+1; xcnt ++)
					{
						int tileno = spriteram2[word_offset + loopno];
						drawgfxzoom_transpen(bitmap, cliprect, gfx, tileno, col, 0, 0,xpos + xcnt * xzoom/2, ypos + ycnt * yzoom/2,xzoom << 11, yzoom << 11, 15);
						drawgfxzoom_transpen(bitmap, cliprect, gfx, tileno, col, 0, 0,-0x200+xpos + xcnt * xzoom/2, ypos + ycnt * yzoom/2,xzoom << 11, yzoom << 11, 15);
						loopno ++;
					}
				}
				else
				{
					for (xcnt = wide; xcnt >= 0; xcnt --)
					{
						int tileno = spriteram2[word_offset + loopno];
						drawgfxzoom_transpen(bitmap, cliprect, gfx, tileno, col, 1, 0,xpos + xcnt * xzoom/2, ypos + ycnt * yzoom/2,xzoom << 11, yzoom << 11, 15);
						drawgfxzoom_transpen(bitmap, cliprect, gfx, tileno, col, 1, 0,-0x200+xpos + xcnt * xzoom/2, ypos + ycnt * yzoom/2,xzoom << 11, yzoom << 11, 15);
						loopno ++;
					}
				}
			}
		}
	}
}



static void draw_sprite_taotaido( UINT16* spriteram, int spriteram_bytes, UINT16* spriteram2, UINT16* spriteram3, running_machine &machine, UINT16 spriteno, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	/*- SPR RAM Format -**

      4 words per sprite

      zzzz sssp  pppp pppp (y zoom, y size, y position)
      zzzz sssp  pppp pppp (x zoom, x size, x position)
      yxpc cccc  ---- ---- (flipy, flipx, priority?, colour)
      -nnn nnnn  nnnn nnnn (tile lookup)

    */

	int x,y;

	UINT16 *source = &spriteram[spriteno*4];
	gfx_element *gfx = machine.gfx[0];


	int yzoom = (source[0] & 0xf000) >> 12;
	int xzoom = (source[1] & 0xf000) >> 12;

	int ysize = (source[0] & 0x0e00) >> 9;
	int xsize = (source[1] & 0x0e00) >> 9;

	int ypos = source[0] & 0x01ff;
	int xpos = source[1] & 0x01ff;

	int yflip = source[2] & 0x8000;
	int xflip = source[2] & 0x4000;
	int color = (source[2] & 0x1f00) >> 8;

	int tile = source[3] & 0xffff;

	xpos += (xsize*xzoom+2)/4;
	ypos += (ysize*yzoom+2)/4;

	xzoom = 32 - xzoom;
	yzoom = 32 - yzoom;


	for (y = 0;y <= ysize;y++)
	{
		int sx,sy;

		if (yflip) sy = ((ypos + yzoom * (ysize - y)/2 + 16) & 0x1ff) - 16;
			else sy = ((ypos + yzoom * y / 2 + 16) & 0x1ff) - 16;

		for (x = 0;x <= xsize;x++)
		{

			/* this indirection is a bit different to the other video system games */
			int realtile;

			realtile = spriteram2[tile&0x7fff];

			if (realtile > 0x3fff)
			{
				int block;

				block = (realtile & 0x3800)>>11;

				realtile &= 0x07ff;
				realtile |= spriteram3[block] * 0x800;
			}

			if (xflip) sx = ((xpos + xzoom * (xsize - x) / 2 + 16) & 0x1ff) - 16;
				else sx = ((xpos + xzoom * x / 2 + 16) & 0x1ff) - 16;


			drawgfxzoom_transpen(bitmap,cliprect,gfx,
						realtile,
						color,
						xflip,yflip,
						sx,sy,
						xzoom << 11, yzoom << 11,15);

			tile++;

		}
	}
}

void vsystem_spr_device::draw_sprites_taotaido( UINT16* spriteram, int spriteram_bytes, UINT16* spriteram2, UINT16* spriteram3, running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT16 *source = spriteram;
	UINT16 *finish = spriteram + spriteram_bytes/2;

	while( source<finish )
	{
		if (source[0] == 0x4000) break;

		draw_sprite_taotaido(spriteram, spriteram_bytes, spriteram2, spriteram3, machine, source[0]&0x3ff, bitmap, cliprect);

		source++;
	}
}



void vsystem_spr_device::draw_sprites_crshrace(UINT16* spriteram, int spriteram_bytes, UINT16* spriteram2, running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect, int flipscreen)
{
	int offs;

	offs = 0;
	while (offs < 0x0400 && (spriteram[offs] & 0x4000) == 0)
	{
		int attr_start;
		int map_start;
		int ox, oy, x, y, xsize, ysize, zoomx, zoomy, flipx, flipy, color;
		/* table hand made by looking at the ship explosion in aerofgt attract mode */
		/* it's almost a logarithmic scale but not exactly */
		static const int zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };

		attr_start = 4 * (spriteram[offs++] & 0x03ff);

		ox = spriteram[attr_start + 1] & 0x01ff;
		xsize = (spriteram[attr_start + 1] & 0x0e00) >> 9;
		zoomx = (spriteram[attr_start + 1] & 0xf000) >> 12;
		oy = spriteram[attr_start + 0] & 0x01ff;
		ysize = (spriteram[attr_start + 0] & 0x0e00) >> 9;
		zoomy = (spriteram[attr_start + 0] & 0xf000) >> 12;
		flipx = spriteram[attr_start + 2] & 0x4000;
		flipy = spriteram[attr_start + 2] & 0x8000;
		color = (spriteram[attr_start + 2] & 0x1f00) >> 8;
		map_start = spriteram[attr_start + 3] & 0x7fff;

		zoomx = 16 - zoomtable[zoomx] / 8;
		zoomy = 16 - zoomtable[zoomy] / 8;

		if (spriteram[attr_start + 2] & 0x20ff) color = machine.rand();

		for (y = 0; y <= ysize; y++)
		{
			int sx,sy;

			if (flipy) sy = ((oy + zoomy * (ysize - y) + 16) & 0x1ff) - 16;
			else sy = ((oy + zoomy * y + 16) & 0x1ff) - 16;

			for (x = 0; x <= xsize; x++)
			{
				int code;

				if (flipx) sx = ((ox + zoomx * (xsize - x) + 16) & 0x1ff) - 16;
				else sx = ((ox + zoomx * x + 16) & 0x1ff) - 16;

				code = spriteram2[map_start & 0x7fff];
				map_start++;

				if (flipscreen)
					drawgfxzoom_transpen(bitmap,cliprect,machine.gfx[2],
							code,
							color,
							!flipx,!flipy,
							304-sx,208-sy,
							0x1000 * zoomx,0x1000 * zoomy,15);
				else
					drawgfxzoom_transpen(bitmap,cliprect,machine.gfx[2],
							code,
							color,
							flipx,flipy,
							sx,sy,
							0x1000 * zoomx,0x1000 * zoomy,15);
			}
		}
	}
}



void vsystem_spr_device::draw_sprites_aerofght( UINT16* spriteram3, int spriteram_bytes, UINT16* spriteram1, UINT16* spriteram2, running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority )
{
	int offs;
	priority <<= 12;

	offs = 0;
	while (offs < 0x0400 && (spriteram3[offs] & 0x8000) == 0)
	{
		int attr_start = 4 * (spriteram3[offs] & 0x03ff);

		/* is the way I handle priority correct? Or should I just check bit 13? */
		if ((spriteram3[attr_start + 2] & 0x3000) == priority)
		{
			int map_start;
			int ox, oy, x, y, xsize, ysize, zoomx, zoomy, flipx, flipy, color;

			ox = spriteram3[attr_start + 1] & 0x01ff;
			xsize = (spriteram3[attr_start + 1] & 0x0e00) >> 9;
			zoomx = (spriteram3[attr_start + 1] & 0xf000) >> 12;
			oy = spriteram3[attr_start + 0] & 0x01ff;
			ysize = (spriteram3[attr_start + 0] & 0x0e00) >> 9;
			zoomy = (spriteram3[attr_start + 0] & 0xf000) >> 12;
			flipx = spriteram3[attr_start + 2] & 0x4000;
			flipy = spriteram3[attr_start + 2] & 0x8000;
			color = (spriteram3[attr_start + 2] & 0x0f00) >> 8;
			map_start = spriteram3[attr_start + 3] & 0x3fff;

			ox += (xsize * zoomx + 2) / 4;
			oy += (ysize * zoomy + 2) / 4;

			zoomx = 32 - zoomx;
			zoomy = 32 - zoomy;

			for (y = 0; y <= ysize; y++)
			{
				int sx, sy;

				if (flipy)
					sy = ((oy + zoomy * (ysize - y)/2 + 16) & 0x1ff) - 16;
				else
					sy = ((oy + zoomy * y / 2 + 16) & 0x1ff) - 16;

				for (x = 0; x <= xsize; x++)
				{
					int code;

					if (flipx)
						sx = ((ox + zoomx * (xsize - x) / 2 + 16) & 0x1ff) - 16;
					else
						sx = ((ox + zoomx * x / 2 + 16) & 0x1ff) - 16;

					if (map_start < 0x2000)
						code = spriteram1[map_start & 0x1fff] & 0x1fff;
					else
						code = spriteram2[map_start & 0x1fff] & 0x1fff;

					drawgfxzoom_transpen(bitmap,cliprect,machine.gfx[2 + (map_start >= 0x2000 ? 1 : 0)],
							code,
							color,
							flipx,flipy,
							sx,sy,
							zoomx << 11, zoomy << 11,15);
					map_start++;
				}
			}
		}
		offs++;
	}
}


void vsystem_spr_device::f1gp2_draw_sprites(UINT16* spritelist, UINT16* sprcgram, int flipscreen, running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;

	offs = 0;
	while (offs < 0x0400 && (spritelist[offs] & 0x4000) == 0)
	{
		int attr_start;
		int map_start;
		int ox, oy, x, y, xsize, ysize, zoomx, zoomy, flipx, flipy, color;

		attr_start = 4 * (spritelist[offs++] & 0x01ff);

		ox = spritelist[attr_start + 1] & 0x01ff;
		xsize = (spritelist[attr_start + 1] & 0x0e00) >> 9;
		zoomx = (spritelist[attr_start + 1] & 0xf000) >> 12;
		oy = spritelist[attr_start + 0] & 0x01ff;
		ysize = (spritelist[attr_start + 0] & 0x0e00) >> 9;
		zoomy = (spritelist[attr_start + 0] & 0xf000) >> 12;
		flipx = spritelist[attr_start + 2] & 0x4000;
		flipy = spritelist[attr_start + 2] & 0x8000;
		color = (spritelist[attr_start + 2] & 0x1f00) >> 8;
		map_start = spritelist[attr_start + 3] & 0x7fff;

// aerofgt has the following adjustment, but doing it here would break the title screen
//      ox += (xsize*zoomx+2)/4;
//      oy += (ysize*zoomy+2)/4;

		zoomx = 32 - zoomx;
		zoomy = 32 - zoomy;

		if (spritelist[attr_start + 2] & 0x20ff)
			color = machine.rand();

		for (y = 0; y <= ysize; y++)
		{
			int sx,sy;

			if (flipy) sy = ((oy + zoomy * (ysize - y)/2 + 16) & 0x1ff) - 16;
			else sy = ((oy + zoomy * y / 2 + 16) & 0x1ff) - 16;

			for (x = 0; x <= xsize; x++)
			{
				int code;

				if (flipx) sx = ((ox + zoomx * (xsize - x) / 2 + 16) & 0x1ff) - 16;
				else sx = ((ox + zoomx * x / 2 + 16) & 0x1ff) - 16;

				code = sprcgram[map_start & 0x3fff];
				map_start++;

				if (flipscreen)
					drawgfxzoom_transpen(bitmap,cliprect,machine.gfx[1],
							code,
							color,
							!flipx,!flipy,
							304-sx,208-sy,
							zoomx << 11,zoomy << 11,15);
				else
					drawgfxzoom_transpen(bitmap,cliprect,machine.gfx[1],
							code,
							color,
							flipx,flipy,
							sx,sy,
							zoomx << 11,zoomy << 11,15);
			}
		}
	}
}


/*** Fujitsu CG10103 **********************************************/

/*
    Fujitsu CG10103 sprite generator
    --------------------------------

- Tile based
- 16x16 4bpp tiles
- Up to 7x7 in each block
- 5 bit of palette selection for the mixer
- Scaling (x/y)
- Flipping
- Indipendent sorting list
- 1 bit of priority for the mixer

Note that this chip can be connected to a VS9210 which adds a level of indirection for
tile numbers. Basically, the VS9210 indirects the tile number through a table in its attached
memory, before accessing the ROMs.


    Sorting list format (VideoRAM offset 0)
    ---------------------------------------

?e-- ---f ssss ssss

e=end of list
f=sprite present in this position
s=sprite index
?=used together with 'e' almost always


    Sprite format (VideoRAM offset 0x400)
    -------------------------------------

0: nnnn jjjy yyyy yyyy
1: mmmm iiix xxxx xxxx
2: fFpc cccc ---- ---t
3: tttt tttt tttt tttt

t=tile, x=posx, y=posy, i=blockx, j=blocky
c=color, m=zoomx, n=zoomy, p=priority

The zoom (scaling) is probably non-linear, it would require a hand-made table unless we find the correct
formula. I'd probably try 1/x. I'm almost sure that it scales between full size (value=0) and half size
(value=0xF) but I couldn't get much more than that from a soccer game.


TODO:
Priorities should be right, but they probably need to be orthogonal with the mixer priorities.
Zoom factor is not correct, the scale is probably non-linear
Horizontal wrapping is just a hack. The chip probably calculates if it needs to draw the sprite at the
  normal position, or wrapped along X/Y.
Abstracts the VS9210

*/


void vsystem_spr_device::CG10103_draw_sprite(running_machine &machine, bitmap_ind16& screen, const rectangle &cliprect, UINT16* spr, int drawpri)
{
	int ypos = spr[0] & 0x1FF;
	int xpos = (spr[1] & 0x1FF);
	UINT32 tile = (spr[3] & 0xFFFF) | ((spr[2] & 1) << 16);
	int ynum = (spr[0] >> 9) & 0x7;
	int xnum = (spr[1] >> 9) & 0x7;
	int color = (spr[2] >> 8) & 0x1F;
	int flipx = (spr[2] >> 14) & 1;
	int flipy = (spr[2] >> 15) & 1;
	int yzoom = (spr[0] >> 12) & 0xF;
	int xzoom = (spr[1] >> 12) & 0xF;
	int pri = (spr[2] >> 13) & 1;
	int x, y;
	int xstep, ystep;
	int xfact, yfact;

	// Check if we want to draw this sprite now
	if (pri != drawpri)
		return;

	// Convert in fixed point to handle the scaling
	xpos <<= 16;
	ypos <<= 16;

	xnum++;
	ynum++;
	xstep = ystep = 16;

	// Linear scale, surely wrong
	xfact = 0x10000 - ((0x8000 * xzoom) / 15);
	yfact = 0x10000 - ((0x8000 * yzoom) / 15);

	xstep *= xfact;
	ystep *= yfact;

	// Handle flipping
	if (flipy)
	{
		ypos += (ynum-1) * ystep;
		ystep = -ystep;
	}

	if (flipx)
	{
		xpos += (xnum-1) * xstep;
		xstep = -xstep;
	}

	// @@@ Add here optional connection to the VS9210 for extra level of tile number indirection
#if 0
	if (m_CG10103_cur_chip->connected_vs9210)
	{
		// ...
	}
#endif

	// Draw the block
	for (y=0;y<ynum;y++)
	{
		int xp = xpos;

		for (x=0;x<xnum;x++)
		{
			// Hack to handle horizontal wrapping
			drawgfxzoom_transpen(screen, cliprect, machine.gfx[m_CG10103_cur_chip->gfx_region], tile, color+m_CG10103_cur_chip->pal_base, flipx, flipy, xp>>16, ypos>>16, xfact, yfact, m_CG10103_cur_chip->transpen);
			drawgfxzoom_transpen(screen, cliprect, machine.gfx[m_CG10103_cur_chip->gfx_region], tile, color+m_CG10103_cur_chip->pal_base, flipx, flipy, (xp>>16) - 0x200, ypos>>16, xfact, yfact, m_CG10103_cur_chip->transpen);
			xp += xstep;
			tile++;
		}

		ypos += ystep;
	}
}


void vsystem_spr_device::CG10103_draw(running_machine &machine, int numchip, bitmap_ind16& screen, const rectangle &cliprect, int priority)
{
	UINT16* splist;
	int i;

	m_CG10103_cur_chip = &m_CG10103;

	splist = m_CG10103_cur_chip->vram;

	// Parse the sorting list
	for (i=0;i<0x400;i++)
	{
		UINT16 cmd = *splist++;

		// End of list
		if (cmd & 0x4000)
			break;

		// Normal sprite here
		if (cmd & 0x100)
		{
			// Extract sprite index
			int num = cmd & 0xFF;

			// Draw the sprite
			CG10103_draw_sprite(machine, screen, cliprect, m_CG10103_cur_chip->vram + 0x400 + num*4, priority);
		}
	}
}



void vsystem_spr_device::CG10103_set_pal_base(int pal_base)
{
	m_CG10103.pal_base = pal_base;
}

void vsystem_spr_device::CG10103_set_gfx_region(int gfx_region)
{
	m_CG10103.gfx_region = gfx_region;
}

void vsystem_spr_device::CG10103_set_transpen(int transpen)
{
	m_CG10103.transpen = transpen;
}

void vsystem_spr_device::CG10103_set_ram(UINT16* vram)
{
	m_CG10103.vram = vram;
}
