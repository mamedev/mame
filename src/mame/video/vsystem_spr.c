// Video System Sprites
// todo:
//  unify these functions (secondary stage lookup differs between games, use callback)

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

void vsystem_spr_device::get_sprite_attributes(UINT16* ram)
{
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
            --xx ---- ---- ---- priority? (upper color bits)
            ---- ---- ---- ---x map start (msb)

            attr_start + 0x0003
            xxxx xxxx xxxx xxxx map start (lsb)
        */

		curr_sprite.oy =    (ram[0] & 0x01ff);
		curr_sprite.ysize = (ram[0] & 0x0e00) >> 9;
		curr_sprite.zoomy = (ram[0] & 0xf000) >> 12;

		curr_sprite.ox =    (ram[1] & 0x01ff);
		curr_sprite.xsize = (ram[1] & 0x0e00) >> 9;
		curr_sprite.zoomx = (ram[1] & 0xf000) >> 12;

		curr_sprite.flipx = (ram[2] & 0x4000);
		curr_sprite.flipy = (ram[2] & 0x8000);
		curr_sprite.color = (ram[2] & 0x3f00) >> 8;
		curr_sprite.pri   = (ram[2] & 0x3000) >> 12;
		curr_sprite.map   = (ram[2] & 0x0001) << 16;

		curr_sprite.map  |= (ram[3] & 0xffff);

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
			int x, y;
			int priority_mask;

			attr_start = 4 * (spriteram[offs] & 0x03ff);

			get_sprite_attributes(&spriteram[attr_start]);

			curr_sprite.oy += 1;
			curr_sprite.map &= 0x7fff;

			switch (curr_sprite.pri)
			{
				default:
				case 0:	priority_mask = 0x00; break;
				case 3:	priority_mask = 0xfe; break;
				case 2:	priority_mask = 0xfc; break;
				case 1:	priority_mask = 0xf0; break;
			}

			curr_sprite.ox += (curr_sprite.xsize * curr_sprite.zoomx + 2) / 4;
			curr_sprite.oy += (curr_sprite.ysize * curr_sprite.zoomy + 2) / 4;

			curr_sprite.zoomx = 32 - curr_sprite.zoomx;
			curr_sprite.zoomy = 32 - curr_sprite.zoomy;

			for (y = 0; y <= curr_sprite.ysize; y++)
			{
				int sx, sy;

				if (curr_sprite.flipy)
					sy = (curr_sprite.oy + curr_sprite.zoomy * (curr_sprite.ysize - y) / 2 + 16) & 0x1ff;
				else
					sy = (curr_sprite.oy + curr_sprite.zoomy * y / 2 + 16) & 0x1ff;

				for (x = 0; x <= curr_sprite.xsize; x++)
				{
					int code;

					if (curr_sprite.flipx)
						sx = (curr_sprite.ox + curr_sprite.zoomx * (curr_sprite.xsize - x) / 2 + 16) & 0x1ff;
					else
						sx = (curr_sprite.ox + curr_sprite.zoomx * x / 2 + 16) & 0x1ff;

					code  = ((spriteram2[curr_sprite.map*2] & 0x0007) << 16) + spriteram2[(curr_sprite.map*2)+ 1];

					pdrawgfxzoom_transpen(bitmap, cliprect, machine.gfx[2],
							code,
							curr_sprite.color,
							curr_sprite.flipx, curr_sprite.flipy,
							sx - 16, sy - 16,
							curr_sprite.zoomx << 11, curr_sprite.zoomy << 11,
							machine.priority_bitmap,priority_mask, 15);

					curr_sprite.map ++;
				}
			}
		}
	}
}



/* todo, fix zooming correctly, it's _not_ like aerofgt */
void vsystem_spr_device::draw_sprites_suprslam( UINT16* spriteram, int spriteram_bytes, UINT16* spriteram2, running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
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
			get_sprite_attributes(&source2[sprnum]);

			curr_sprite.map &= 0x7fff;
									
			int xcnt, ycnt;

			int loopno = 0;

			curr_sprite.zoomx = 32 - curr_sprite.zoomx;
			curr_sprite.zoomy = 32 - curr_sprite.zoomy;

			if (curr_sprite.oy > 0xff) curr_sprite.oy -=0x200;

			for (ycnt = 0; ycnt < curr_sprite.ysize+1; ycnt ++)
			{
				if (!curr_sprite.flipx)
				{
					for (xcnt = 0; xcnt < curr_sprite.xsize+1; xcnt ++)
					{
						int startno = spriteram2[curr_sprite.map + loopno];
						drawgfxzoom_transpen(bitmap, cliprect, gfx, startno, curr_sprite.color, 0, 0,curr_sprite.ox + xcnt * curr_sprite.zoomx/2, curr_sprite.oy + ycnt * curr_sprite.zoomy/2,curr_sprite.zoomx << 11, curr_sprite.zoomy << 11, 15);
						drawgfxzoom_transpen(bitmap, cliprect, gfx, startno, curr_sprite.color, 0, 0,-0x200+curr_sprite.ox + xcnt * curr_sprite.zoomx/2, curr_sprite.oy + ycnt * curr_sprite.zoomy/2,curr_sprite.zoomx << 11, curr_sprite.zoomy << 11, 15);
						loopno ++;
					}
				}
				else
				{
					for (xcnt = curr_sprite.xsize; xcnt >= 0; xcnt --)
					{
						int startno = spriteram2[curr_sprite.map + loopno];
						drawgfxzoom_transpen(bitmap, cliprect, gfx, startno, curr_sprite.color, 1, 0,curr_sprite.ox + xcnt * curr_sprite.zoomx/2, curr_sprite.oy + ycnt * curr_sprite.zoomy/2,curr_sprite.zoomx << 11, curr_sprite.zoomy << 11, 15);
						drawgfxzoom_transpen(bitmap, cliprect, gfx, startno, curr_sprite.color, 1, 0,-0x200+curr_sprite.ox + xcnt * curr_sprite.zoomx/2, curr_sprite.oy + ycnt * curr_sprite.zoomy/2,curr_sprite.zoomx << 11, curr_sprite.zoomy << 11, 15);
						loopno ++;
					}
				}
			}
		}
	}
}



void vsystem_spr_device::draw_sprite_taotaido( UINT16* spriteram, int spriteram_bytes, UINT16* spriteram2, UINT16* spriteram3, running_machine &machine, UINT16 spriteno, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	/*- SPR RAM Format -**

      4 words per sprite

      zzzz sssp  pppp pppp (y zoom, y size, y position)
      zzzz sssp  pppp pppp (x zoom, x size, x position)
      yxpc cccc  ---- ---- (flipy, >flipx, priority?, color)
      -nnn nnnn  nnnn nnnn (map_start lookup)

    */

	int x,y;

	UINT16 *source = &spriteram[spriteno*4];
	gfx_element *gfx = machine.gfx[0];

	get_sprite_attributes(&source[0]);

	curr_sprite.map &= 0xffff;
	curr_sprite.color &= 0x1f;

	curr_sprite.ox += (curr_sprite.xsize*curr_sprite.zoomx+2)/4;
	curr_sprite.oy += (curr_sprite.ysize*curr_sprite.zoomy+2)/4;

	curr_sprite.zoomx = 32 - curr_sprite.zoomx;
	curr_sprite.zoomy = 32 - curr_sprite.zoomy;


	for (y = 0;y <= curr_sprite.ysize;y++)
	{
		int sx,sy;

		if (curr_sprite.flipy) sy = ((curr_sprite.oy + curr_sprite.zoomy * (curr_sprite.ysize - y)/2 + 16) & 0x1ff) - 16;
			else sy = ((curr_sprite.oy + curr_sprite.zoomy * y / 2 + 16) & 0x1ff) - 16;

		for (x = 0;x <= curr_sprite.xsize;x++)
		{

			/* this indirection is a bit different to the other video system games */
			int realstart;

			realstart = spriteram2[curr_sprite.map&0x7fff];

			if (realstart > 0x3fff)
			{
				int block;

				block = (realstart & 0x3800)>>11;

				realstart &= 0x07ff;
				realstart |= spriteram3[block] * 0x800;
			}

			if (curr_sprite.flipx) sx = ((curr_sprite.ox + curr_sprite.zoomx * (curr_sprite.xsize - x) / 2 + 16) & 0x1ff) - 16;
				else sx = ((curr_sprite.ox + curr_sprite.zoomx * x / 2 + 16) & 0x1ff) - 16;


			drawgfxzoom_transpen(bitmap,cliprect,gfx,
						realstart,
						curr_sprite.color,
						curr_sprite.flipx,curr_sprite.flipy,
						sx,sy,
						curr_sprite.zoomx << 11, curr_sprite.zoomy << 11,15);

			curr_sprite.map++;

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
		int x, y;
		/* table hand made by looking at the ship explosion in aerofgt attract mode */
		/* it's almost a logarithmic scale but not exactly */
		static const int zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };

		attr_start = 4 * (spriteram[offs++] & 0x03ff);

		get_sprite_attributes(&spriteram[attr_start]);

		curr_sprite.color &= 0x1f;
		curr_sprite.map &= 0x7fff;

		curr_sprite.zoomx = 16 - zoomtable[curr_sprite.zoomx] / 8;
		curr_sprite.zoomy = 16 - zoomtable[curr_sprite.zoomy] / 8;

		if (spriteram[attr_start + 2] & 0x20ff) curr_sprite.color = machine.rand();

		for (y = 0; y <= curr_sprite.ysize; y++)
		{
			int sx,sy;

			if (curr_sprite.flipy) sy = ((curr_sprite.oy + curr_sprite.zoomy * (curr_sprite.ysize - y) + 16) & 0x1ff) - 16;
			else sy = ((curr_sprite.oy + curr_sprite.zoomy * y + 16) & 0x1ff) - 16;

			for (x = 0; x <= curr_sprite.xsize; x++)
			{
				int code;

				if (curr_sprite.flipx) sx = ((curr_sprite.ox + curr_sprite.zoomx * (curr_sprite.xsize - x) + 16) & 0x1ff) - 16;
				else sx = ((curr_sprite.ox + curr_sprite.zoomx * x + 16) & 0x1ff) - 16;

				code = spriteram2[curr_sprite.map & 0x7fff];
				curr_sprite.map++;

				if (flipscreen)
					drawgfxzoom_transpen(bitmap,cliprect,machine.gfx[2],
							code,
							curr_sprite.color,
							!curr_sprite.flipx,!curr_sprite.flipy,
							304-sx,208-sy,
							0x1000 * curr_sprite.zoomx,0x1000 * curr_sprite.zoomy,15);
				else
					drawgfxzoom_transpen(bitmap,cliprect,machine.gfx[2],
							code,
							curr_sprite.color,
							curr_sprite.flipx,curr_sprite.flipy,
							sx,sy,
							0x1000 * curr_sprite.zoomx,0x1000 * curr_sprite.zoomy,15);
			}
		}
	}
}



void vsystem_spr_device::draw_sprites_aerofght( UINT16* spriteram3, int spriteram_bytes, UINT16* spriteram1, UINT16* spriteram2, running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri )
{
	int offs;
	pri <<= 12;

	offs = 0;
	while (offs < 0x0400 && (spriteram3[offs] & 0x8000) == 0)
	{
		int attr_start = 4 * (spriteram3[offs] & 0x03ff);

		/* is the way I handle pri correct? Or should I just check bit 13? */
		if ((spriteram3[attr_start + 2] & 0x3000) == pri)
		{
			int x, y;

			get_sprite_attributes(&spriteram3[attr_start]);

			curr_sprite.color &=0xf;
			curr_sprite.map &= 0x3fff;
			curr_sprite.ox += (curr_sprite.xsize * curr_sprite.zoomx + 2) / 4;
			curr_sprite.oy += (curr_sprite.ysize * curr_sprite.zoomy + 2) / 4;

			curr_sprite.zoomx = 32 - curr_sprite.zoomx;
			curr_sprite.zoomy = 32 - curr_sprite.zoomy;

			for (y = 0; y <= curr_sprite.ysize; y++)
			{
				int sx, sy;

				if (curr_sprite.flipy)
					sy = ((curr_sprite.oy + curr_sprite.zoomy * (curr_sprite.ysize - y)/2 + 16) & 0x1ff) - 16;
				else
					sy = ((curr_sprite.oy + curr_sprite.zoomy * y / 2 + 16) & 0x1ff) - 16;

				for (x = 0; x <= curr_sprite.xsize; x++)
				{
					int code;

					if (curr_sprite.flipx)
						sx = ((curr_sprite.ox + curr_sprite.zoomx * (curr_sprite.xsize - x) / 2 + 16) & 0x1ff) - 16;
					else
						sx = ((curr_sprite.ox + curr_sprite.zoomx * x / 2 + 16) & 0x1ff) - 16;

					if (curr_sprite.map < 0x2000)
						code = spriteram1[curr_sprite.map & 0x1fff] & 0x1fff;
					else
						code = spriteram2[curr_sprite.map & 0x1fff] & 0x1fff;

					drawgfxzoom_transpen(bitmap,cliprect,machine.gfx[2 + (curr_sprite.map >= 0x2000 ? 1 : 0)],
							code,
							curr_sprite.color,
							curr_sprite.flipx,curr_sprite.flipy,
							sx,sy,
							curr_sprite.zoomx << 11, curr_sprite.zoomy << 11,15);
					curr_sprite.map++;
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
		int x, y;

		attr_start = 4 * (spritelist[offs++] & 0x01ff);

		get_sprite_attributes(&spritelist[attr_start]);

		curr_sprite.color &= 0x1f;
		curr_sprite.map &= 0x7fff;

// aerofgt has the following adjustment, but doing it here would break the title screen
//      curr_sprite.ox += (curr_sprite.xsize*curr_sprite.zoomx+2)/4;
//      curr_sprite.oy += (curr_sprite.ysize*curr_sprite.zoomy+2)/4;

		curr_sprite.zoomx = 32 - curr_sprite.zoomx;
		curr_sprite.zoomy = 32 - curr_sprite.zoomy;

		if (spritelist[attr_start + 2] & 0x20ff)
			curr_sprite.color = machine.rand();

		for (y = 0; y <= curr_sprite.ysize; y++)
		{
			int sx,sy;

			if (curr_sprite.flipy) sy = ((curr_sprite.oy + curr_sprite.zoomy * (curr_sprite.ysize - y)/2 + 16) & 0x1ff) - 16;
			else sy = ((curr_sprite.oy + curr_sprite.zoomy * y / 2 + 16) & 0x1ff) - 16;

			for (x = 0; x <= curr_sprite.xsize; x++)
			{
				int code;

				if (curr_sprite.flipx) sx = ((curr_sprite.ox + curr_sprite.zoomx * (curr_sprite.xsize - x) / 2 + 16) & 0x1ff) - 16;
				else sx = ((curr_sprite.ox + curr_sprite.zoomx * x / 2 + 16) & 0x1ff) - 16;

				code = sprcgram[curr_sprite.map & 0x3fff];
				curr_sprite.map++;

				if (flipscreen)
					drawgfxzoom_transpen(bitmap,cliprect,machine.gfx[1],
							code,
							curr_sprite.color,
							!curr_sprite.flipx,!curr_sprite.flipy,
							304-sx,208-sy,
							curr_sprite.zoomx << 11,curr_sprite.zoomy << 11,15);
				else
					drawgfxzoom_transpen(bitmap,cliprect,machine.gfx[1],
							code,
							curr_sprite.color,
							curr_sprite.flipx,curr_sprite.flipy,
							sx,sy,
							curr_sprite.zoomx << 11,curr_sprite.zoomy << 11,15);
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
- 1 bit of pri for the mixer

Note that this chip can be connected to a VS9210 which adds a level of indirection for
tile numbers. Basically, the VS9210 indirects the tilet number through a table in its attached
memory, before accessing the ROMs.


    Sorting list format (VideoRAM offset 0)
    ---------------------------------------

de-- ---f ssss ssss

e=end of list
f=sprite present in this position
s=sprite index
d=disable sprite?


TODO:
Priorities should be right, but they probably need to be orthogonal with the mixer priorities.
Zoom factor is not correct, the scale is probably non-linear
Horizontal wrapping is just a hack. The chip probably calculates if it needs to draw the sprite at the
  normal position, or wrapped along X/Y.
Abstracts the VS9210

*/


void vsystem_spr_device::CG10103_draw_sprite(running_machine &machine, bitmap_ind16& screen, const rectangle &cliprect, UINT16* spr, int drawpri)
{
	get_sprite_attributes(&spr[0]);
	curr_sprite.color &=0x1f;
	curr_sprite.pri >>= 1;

	int x, y;
	int xstep, ystep;
	int xfact, yfact;


	// Check if we want to draw this sprite now
	if (curr_sprite.pri != drawpri)
		return;

	// Convert in fixed point to handle the scaling
	curr_sprite.ox <<= 16;
	curr_sprite.oy <<= 16;

	curr_sprite.xsize++;
	curr_sprite.ysize++;
	xstep = ystep = 16;

	// Linear scale, surely wrong
	xfact = 0x10000 - ((0x8000 * curr_sprite.zoomx) / 15);
	yfact = 0x10000 - ((0x8000 * curr_sprite.zoomy) / 15);

	xstep *= xfact;
	ystep *= yfact;

	// Handle flipping
	if (curr_sprite.flipy)
	{
		curr_sprite.oy += (curr_sprite.ysize-1) * ystep;
		ystep = -ystep;
	}

	if (curr_sprite.flipx)
	{
		curr_sprite.ox += (curr_sprite.xsize-1) * xstep;
		xstep = -xstep;
	}

	// Draw the block
	for (y=0;y<curr_sprite.ysize;y++)
	{
		int xp = curr_sprite.ox;

		for (x=0;x<curr_sprite.xsize;x++)
		{
			// Hack to handle horizontal wrapping
			drawgfxzoom_transpen(screen, cliprect, machine.gfx[m_CG10103_cur_chip->gfx_region], curr_sprite.map, curr_sprite.color+m_CG10103_cur_chip->pal_base, curr_sprite.flipx, curr_sprite.flipy, xp>>16, curr_sprite.oy>>16, xfact, yfact, m_CG10103_cur_chip->transpen);
			drawgfxzoom_transpen(screen, cliprect, machine.gfx[m_CG10103_cur_chip->gfx_region], curr_sprite.map, curr_sprite.color+m_CG10103_cur_chip->pal_base, curr_sprite.flipx, curr_sprite.flipy, (xp>>16) - 0x200, curr_sprite.oy>>16, xfact, yfact, m_CG10103_cur_chip->transpen);

			drawgfxzoom_transpen(screen, cliprect, machine.gfx[m_CG10103_cur_chip->gfx_region], curr_sprite.map, curr_sprite.color+m_CG10103_cur_chip->pal_base, curr_sprite.flipx, curr_sprite.flipy, xp>>16, (curr_sprite.oy>>16)-0x200, xfact, yfact, m_CG10103_cur_chip->transpen);
			drawgfxzoom_transpen(screen, cliprect, machine.gfx[m_CG10103_cur_chip->gfx_region], curr_sprite.map, curr_sprite.color+m_CG10103_cur_chip->pal_base, curr_sprite.flipx, curr_sprite.flipy, (xp>>16) - 0x200, (curr_sprite.oy>>16)-0x200, xfact, yfact, m_CG10103_cur_chip->transpen);

			xp += xstep;
			curr_sprite.map++;
		}

		curr_sprite.oy += ystep;
	}
}


void vsystem_spr_device::CG10103_draw(running_machine &machine, int numchip, bitmap_ind16& screen, const rectangle &cliprect, int pri)
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

		if (!(cmd & 0x8000))
		{
			// Extract sprite index
			int num = cmd & 0x3FF;

			// Draw the sprite
			CG10103_draw_sprite(machine, screen, cliprect, m_CG10103_cur_chip->vram + num*4, pri);
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
