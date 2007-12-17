/***************************************************************************

                          -= Newer Seta Hardware =-

                    driver by   Luca Elia (l.elia@tin.it)


    This hardware only generates sprites. But they're of various types,
    including some large "floating tilemap" ones.

    Sprites RAM is 0x40000 bytes long. All games write the sprites list
    at offset 0x3000. Each entry in the list holds data for a multi-sprite
    of up to 256 single-sprites. The list looks like this:

    Offset:     Bits:                   Value:

        0.w     f--- ---- ---- ----     Last sprite
                -edc ---- ---- ----     ?
                ---c ---- ---- ----     0 = Each sprite specifies its size, 1 = use the size in the following words
                ---- b--- ---- ----     ?
                ---- -a98 ---- ----     tile color depth
                ---- ---- 7654 3210     Number of sprites - 1

        2.w     fedc ---- ---- ----     Number of tiles?
                ---- ba-- ---- ----     Number of tiles along X (1 << n)
                ---- --98 7654 3210     X displacement

        4.w     fedc ---- ---- ----     Number of tiles?
                ---- ba-- ---- ----     Number of tiles along Y (1 << n)
                ---- --98 7654 3210     Y displacement

        6.w     f--- ---- ---- ----     Single-sprite(s) type: tile (0) or row of tiles (1)
                -edc ba98 7654 3210     Offset of the single-sprite(s) data


    A single-sprite can be a tile or some horizontal rows of tiles.

    Tile case:

        0.w     fedc ---- ---- ----     Number of tiles?
                ---- ba-- ---- ----     Number of tiles along X (1 << n)
                ---- --98 7654 3210     X

        2.w     fedc ---- ---- ----     Number of tiles?
                ---- ba-- ---- ----     Number of tiles along Y (1 << n)
                ---- --98 7654 3210     Y

        4.w     fedc ba98 765- ----     Color code (16 color steps)
                ---- ---- ---4 ----     Flip X
                ---- ---- ---- 3---     Flip Y
                ---- ---- ---- -210     Code (high bits)

        6.w                             Code (low bits)


    Row case:

        0.w     fedc ba-- ---- ----
                ---- --98 7654 3210     X

        2.w     fedc ba-- ---- ----     Number of rows - 1
                ---- --98 7654 3210     Y

        4.w     f--- ---- ---- ----     Tile size: 8x8 (0) or 16x16 (1)
                -edc ba-- ---- ----     "Tilemap" page
                ---- --98 7654 3210     "Tilemap" Scroll X

        6.w     fedc ba9- ---- ----
                ---- ---8 7654 3210     "Tilemap" Scroll Y

***************************************************************************/

#include "driver.h"
#include "seta.h"

UINT16 *seta2_vregs;

static int yoffset;

/***************************************************************************


                                Video Registers

    Offset:     Bits:                   Value:

    0/2/4/6                             ? Horizontal
    8/a/c/e                             ? Vertical

    10
    12                                  Offset X?
    14                                  Zoom X? low bits
    16                                  Zoom X? high bits *

    18
    1a                                  Offset Y?
    1c                                  Zoom Y? low bits
    1e                                  Zoom Y? high bits

    30          fedc ba98 7654 321-
                ---- ---- ---- ---0     Disable video

    32..3f                              ?


    * A value of 1 is means no zoom, a value of 2 will halve the size.
      It's unknown whether a value less than 1 means magnification (probably yes)

***************************************************************************/

WRITE16_HANDLER( seta2_vregs_w )
{
	/* 02/04 = horizontal display start/end
               mj4simai = 0065/01E5 (0180 visible area)
               myangel =  005D/01D5 (0178 visible area)
               pzlbowl =  0058/01D8 (0180 visible area)
               penbros =  0065/01A5 (0140 visible area)
               grdians =  0059/0188 (012f visible area)
       06    = horizontal total?
               mj4simai = 0204
               myangel =  0200
               pzlbowl =  0204
               penbros =  01c0
               grdians =  019a
    */

	COMBINE_DATA(&seta2_vregs[offset]);
	switch( offset*2 )
	{
	case 0x1c:	// FLIP SCREEN (myangel)    <- this is actually zoom
		flip_screen_set( data & 1 );
		if (data & ~1)	logerror("CPU #0 PC %06X: flip screen unknown bits %04X\n",activecpu_get_pc(),data);
		break;
	case 0x2a:	// FLIP X (pzlbowl)
		flip_screen_x_set( data & 1 );
		if (data & ~1)	logerror("CPU #0 PC %06X: flipx unknown bits %04X\n",activecpu_get_pc(),data);
		break;
	case 0x2c:	// FLIP Y (pzlbowl)
		flip_screen_y_set( data & 1 );
		if (data & ~1)	logerror("CPU #0 PC %06X: flipy unknown bits %04X\n",activecpu_get_pc(),data);
		break;

	case 0x30:	// BLANK SCREEN (pzlbowl, myangel)
		if (data & ~1)	logerror("CPU #0 PC %06X: blank unknown bits %04X\n",activecpu_get_pc(),data);
		break;

	default:
		logerror("CPU #0 PC %06X: Video Reg %02X <- %04X\n",activecpu_get_pc(),offset*2,data);
	}
}


/***************************************************************************


                                Sprites Drawing


***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	/* Sprites list */

	UINT16 *s1  = buffered_spriteram16 + 0x3000/2;
	UINT16 *end = &buffered_spriteram16[spriteram_size/2];

	for ( ; s1 < end; s1+=4 )
	{
		int gfx;
		int num		= s1[0];
		int xoffs	= s1[1];
		int yoffs	= s1[2];
		int sprite	= s1[3];

		/* Single-sprite address */
		UINT16 *s2 = &buffered_spriteram16[(sprite & 0x7fff) * 4];

		/* Single-sprite tile size */
		int global_sizex = xoffs & 0x0c00;
		int global_sizey = yoffs & 0x0c00;

		int use_global_size = num & 0x1000;

		xoffs &= 0x3ff;
		yoffs &= 0x3ff;

		/* Color depth */
		switch (num & 0x0700)
		{
			default:
				popmessage("unknown gfxset %x",(num & 0x0700)>>8);
				gfx = mame_rand(machine)&3; break;
			case 0x0700: 	// 8bpp tiles (76543210)
				gfx = 3; break;
			case 0x0600:	// 6bpp tiles (--543210) (myangel sliding blocks test)
				gfx = 2; break;
			case 0x0500:	// 4bpp tiles (3210----)
				gfx = 1; break;
			case 0x0400:	// 4bpp tiles (----3210)
				gfx = 0; break;
//          case 0x0300:
//              unknown
			case 0x0200:	// 3bpp tiles?  (-----210) (myangel "Graduate Tests")
				gfx = 4; break;
			case 0x0100:	// 2bpp tiles??? (--10----) (myangel2 question bubble, myangel endgame)
				gfx = 5; break;
			case 0x0000:	// no idea!
				gfx = 0; break;
		}

		/* Number of single-sprites */
		num = (num & 0x00ff) + 1;

		for( ; num > 0; num--,s2+=4 )
		{
			if (s2 >= end)	break;

// "tilemap" sprite

			if (sprite & 0x8000)
			{
				rectangle clip;
				int dx,x,y;
				int flipx;
				int flipy;
				int sx       = s2[0];
				int sy       = s2[1];
				int scrollx  = s2[2];
				int scrolly  = s2[3];
				int tilesize = (scrollx & 0x8000) >> 15;
				int page     = (scrollx & 0x7c00) >> 10;
				int height   = ((sy & 0xfc00) >> 10) + 1;

				sx &= 0x3ff;
				sy &= 0x1ff;
				scrollx &= 0x3ff;
				scrolly &= 0x1ff;

				clip.min_y = (sy + yoffs) & 0x1ff;
				clip.max_y = clip.min_y + height * 0x10 - 1;

				if (clip.min_y > cliprect->max_y)	continue;
				if (clip.max_y < cliprect->min_y)	continue;

				clip.min_x = cliprect->min_x;
				clip.max_x = cliprect->max_x;

				if (clip.min_y < cliprect->min_y)	clip.min_y = cliprect->min_y;
				if (clip.max_y > cliprect->max_y)	clip.max_y = cliprect->max_y;

				dx = sx + (scrollx & 0x3ff) + xoffs + 0x10;

				/* Draw the rows */
				/* I don't think the following is entirely correct (when using 16x16
                   tiles x should probably loop from 0 to 0x20) but it seems to work
                   fine in all the games we have for now. */
				for (y = 0; y < (0x40 >> tilesize); y++)
				{
					int py = ((scrolly - (y+1) * (8 << tilesize) + 0x10) & 0x1ff) - 0x10 - yoffset;

					if (py < clip.min_y - 0x10) continue;
					if (py > clip.max_y) continue;

					for (x = 0; x < 0x40;x++)
					{
						int px = ((dx + x * (8 << tilesize) + 0x10) & 0x3ff) - 0x10;
						int tx, ty;
						int attr, code, color;
						UINT16 *s3;

						if (px < clip.min_x - 0x10) continue;
						if (px > clip.max_x) continue;

						s3	=	&buffered_spriteram16[2 * ((page * 0x2000/4) + ((y & 0x1f) << 6) + (x & 0x03f))];

						attr  = s3[0];
						code  = s3[1] + ((attr & 0x0007) << 16);
						flipx = (attr & 0x0010);
						flipy = (attr & 0x0008);
						color = (attr & 0xffe0) >> 5;

						if (tilesize) code &= ~3;

						for (ty = 0; ty <= tilesize; ty++)
						{
							for (tx = 0; tx <= tilesize; tx++)
							{
								drawgfx(bitmap, machine->gfx[gfx],
										code ^ tx ^ (ty<<1),
										color,
										flipx, flipy,
										px + (flipx ? tilesize-tx : tx) * 8, py + (flipy ? tilesize-ty : ty) * 8,
										cliprect,TRANSPARENCY_PEN,0 );
							}
						}

					}
				}
			}
			else
// "normal" sprite
			{
				int sx    = s2[0];
				int sy    = s2[1];
				int attr  = s2[2];
				int code  = s2[3] + ((attr & 0x0007) << 16);
				int flipx = (attr & 0x0010);
				int flipy = (attr & 0x0008);
				int color = (attr & 0xffe0) >> 5;

				int sizex = use_global_size ? global_sizex : sx;
				int sizey = use_global_size ? global_sizey : sy;
				int x,y;
				sizex = (1 << ((sizex & 0x0c00)>> 10))-1;
				sizey = (1 << ((sizey & 0x0c00)>> 10))-1;


				sx += xoffs;
				sy += yoffs;

				sx = (sx & 0x1ff) - (sx & 0x200);
				sy &= 0x1ff;
				sy -= yoffset;

				code &= ~((sizex+1) * (sizey+1) - 1);	// see myangel, myangel2 and grdians

				for (y = 0; y <= sizey; y++)
				{
					for (x = 0; x <= sizex; x++)
					{
						drawgfx(bitmap, machine->gfx[gfx],
								code++,
								color,
								flipx, flipy,
								sx + (flipx ? sizex-x : x) * 8, sy + (flipy ? sizey-y : y) * 8,
								cliprect,TRANSPARENCY_PEN,0 );
					}
				}
			}
		}

		if (s1[0] & 0x8000) break;	/* end of list marker */
	}	/* sprite list */
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/

VIDEO_START( seta2 )
{
	machine->gfx[2]->color_granularity = 16;
	machine->gfx[3]->color_granularity = 16;
	machine->gfx[4]->color_granularity = 16;
	machine->gfx[5]->color_granularity = 16;

	buffered_spriteram16 = auto_malloc(spriteram_size);

	yoffset = 0;
}

VIDEO_START( seta2_offset )
{
	video_start_seta2(machine);

	yoffset = 0x10;
}

VIDEO_UPDATE( seta2 )
{
	/* Black or pens[0]? */
	fillbitmap(bitmap,machine->pens[0],cliprect);

	if (seta2_vregs[0x30/2] & 1)	return 0;		// BLANK SCREEN

	draw_sprites(machine,bitmap,cliprect);
	return 0;
}

VIDEO_EOF( seta2 )
{
	/* Buffer sprites by 1 frame */
	memcpy(buffered_spriteram16,spriteram16,spriteram_size);
}
