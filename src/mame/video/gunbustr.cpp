// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Graves
#include "emu.h"
#include "includes/gunbustr.h"

/************************************************************/

void gunbustr_state::video_start()
{
	m_spritelist = std::make_unique<gb_tempsprite[]>(0x4000);
}

/************************************************************
            SPRITE DRAW ROUTINES

We draw a series of small tiles ("chunks") together to
create each big sprite. The spritemap rom provides the lookup
table for this. The game hardware looks up 16x16 sprite chunks
from the spritemap rom, creating a 64x64 sprite like this:

     0  1  2  3
     4  5  6  7
     8  9 10 11
    12 13 14 15

(where the number is the word offset into the spritemap rom).
It can also create 32x32 sprites.

NB: unused portions of the spritemap rom contain hex FF's.
It is a useful coding check to warn in the log if these
are being accessed. [They can be inadvertently while
spriteram is being tested, take no notice of that.]

Heavy use is made of sprite zooming.

        ***

    Sprite table layout (4 long words per entry)

    ------------------------------------------
     0 | ........ x....... ........ ........ | Flip X
     0 | ........ .xxxxxxx ........ ........ | ZoomX
     0 | ........ ........ .xxxxxxx xxxxxxxx | Sprite Tile
       |                                     |
     2 | ........ ....xx.. ........ ........ | Sprite/tile priority [*]
     2 | ........ ......xx xxxxxx.. ........ | Palette bank
     2 | ........ ........ ......xx xxxxxxxx | X position
       |                                     |
     3 | ........ .....x.. ........ ........ | Sprite size (0=32x32, 1=64x64)
     3 | ........ ......x. ........ ........ | Flip Y
     3 | ........ .......x xxxxxx.. ........ | ZoomY
     3 | ........ ........ ......xx xxxxxxxx | Y position
    ------------------------------------------

    [* 00=over BG1; 01=BG2; 10=BG3; 11=over text ???]

********************************************************/

void gunbustr_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect,const int *primasks,int x_offs,int y_offs)
{
	UINT32 *spriteram32 = m_spriteram;
	UINT16 *spritemap = (UINT16 *)memregion("user1")->base();
	int offs, data, tilenum, color, flipx, flipy;
	int x, y, priority, dblsize, curx, cury;
	int sprites_flipscreen = 0;
	int zoomx, zoomy, zx, zy;
	int sprite_chunk,map_offset,code,j,k,px,py;
	int dimension,total_chunks,bad_chunks;

	/* pdrawgfx() needs us to draw sprites front to back, so we have to build a list
	   while processing sprite ram and then draw them all at the end */
	struct gb_tempsprite *sprite_ptr = m_spritelist.get();

	for (offs = (m_spriteram.bytes()/4-4);offs >= 0;offs -= 4)
	{
		data = spriteram32[offs+0];
		flipx =    (data & 0x00800000) >> 23;
		zoomx =    (data & 0x007f0000) >> 16;
		tilenum =  (data & 0x00007fff);

		data = spriteram32[offs+2];
		priority = (data & 0x000c0000) >> 18;
		color =    (data & 0x0003fc00) >> 10;
		x =        (data & 0x000003ff);

		data = spriteram32[offs+3];
		dblsize =  (data & 0x00040000) >> 18;
		flipy =    (data & 0x00020000) >> 17;
		zoomy =    (data & 0x0001fc00) >> 10;
		y =        (data & 0x000003ff);

		color |= 0x80;

		if (!tilenum) continue;

		flipy = !flipy;
		zoomx += 1;
		zoomy += 1;

		y += y_offs;

		/* treat coords as signed */
		if (x>0x340) x -= 0x400;
		if (y>0x340) y -= 0x400;

		x -= x_offs;

		bad_chunks = 0;
		dimension = ((dblsize*2) + 2);  // 2 or 4
		total_chunks = ((dblsize*3) + 1) << 2;  // 4 or 16
		map_offset = tilenum << 2;

		{
			for (sprite_chunk=0;sprite_chunk<total_chunks;sprite_chunk++)
			{
				j = sprite_chunk / dimension;   /* rows */
				k = sprite_chunk % dimension;   /* chunks per row */

				px = k;
				py = j;
				/* pick tiles back to front for x and y flips */
				if (flipx)  px = dimension-1-k;
				if (flipy)  py = dimension-1-j;

				code = spritemap[map_offset + px + (py<<(dblsize+1))];

				if (code==0xffff)
				{
					bad_chunks += 1;
					continue;
				}

				curx = x + ((k*zoomx)/dimension);
				cury = y + ((j*zoomy)/dimension);

				zx= x + (((k+1)*zoomx)/dimension) - curx;
				zy= y + (((j+1)*zoomy)/dimension) - cury;

				if (sprites_flipscreen)
				{
					/* -zx/y is there to fix zoomed sprite coords in screenflip.
					   drawgfxzoom does not know to draw from flip-side of sprites when
					   screen is flipped; so we must correct the coords ourselves. */

					curx = 320 - curx - zx;
					cury = 256 - cury - zy;
					flipx = !flipx;
					flipy = !flipy;
				}

				sprite_ptr->gfx = 0;
				sprite_ptr->code = code;
				sprite_ptr->color = color;
				sprite_ptr->flipx = !flipx;
				sprite_ptr->flipy = flipy;
				sprite_ptr->x = curx;
				sprite_ptr->y = cury;
				sprite_ptr->zoomx = zx << 12;
				sprite_ptr->zoomy = zy << 12;

				if (primasks)
				{
					sprite_ptr->primask = primasks[priority];

					sprite_ptr++;
				}
				else
				{
					m_gfxdecode->gfx(sprite_ptr->gfx)->zoom_transpen(bitmap,cliprect,
							sprite_ptr->code,
							sprite_ptr->color,
							sprite_ptr->flipx,sprite_ptr->flipy,
							sprite_ptr->x,sprite_ptr->y,
							sprite_ptr->zoomx,sprite_ptr->zoomy,0);
				}
			}
		}

		if (bad_chunks)
logerror("Sprite number %04x had %02x invalid chunks\n",tilenum,bad_chunks);
	}

	/* this happens only if primsks != NULL */
	while (sprite_ptr != m_spritelist.get())
	{
		sprite_ptr--;

		m_gfxdecode->gfx(sprite_ptr->gfx)->prio_zoom_transpen(bitmap,cliprect,
				sprite_ptr->code,
				sprite_ptr->color,
				sprite_ptr->flipx,sprite_ptr->flipy,
				sprite_ptr->x,sprite_ptr->y,
				sprite_ptr->zoomx,sprite_ptr->zoomy,
				screen.priority(),sprite_ptr->primask,0);
	}
}



/**************************************************************
                SCREEN REFRESH
**************************************************************/

UINT32 gunbustr_state::screen_update_gunbustr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 layer[5];
	UINT16 priority;
	static const int primasks[4] = {0xfffc, 0xfff0, 0xff00, 0x0};

	m_tc0480scp->tilemap_update();

	priority = m_tc0480scp->get_bg_priority();
	layer[0] = (priority & 0xf000) >> 12;   /* tells us which bg layer is bottom */
	layer[1] = (priority & 0x0f00) >>  8;
	layer[2] = (priority & 0x00f0) >>  4;
	layer[3] = (priority & 0x000f) >>  0;   /* tells us which is top */
	layer[4] = 4;   /* text layer always over bg layers */

	screen.priority().fill(0, cliprect);

	/* We have to assume 2nd to bottom layer is always underneath
	   sprites as pdrawgfx cannot yet cope with more than 4 layers */

#ifdef MAME_DEBUG
	if (!machine().input().code_pressed (KEYCODE_Z)) m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[0],TILEMAP_DRAW_OPAQUE, 0);
	if (!machine().input().code_pressed (KEYCODE_X)) m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 1);
	if (!machine().input().code_pressed (KEYCODE_C)) m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 2);
	if (!machine().input().code_pressed (KEYCODE_V)) m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[3], 0, 4);
	if (!machine().input().code_pressed (KEYCODE_B)) m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[4], 0, 8);
	if (!machine().input().code_pressed (KEYCODE_N)) draw_sprites(screen, bitmap, cliprect, primasks, 48, -116);
#else
	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 0);
	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 1);
	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 2);
	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[3], 0, 4);
	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[4], 0, 8);    /* text layer */
	draw_sprites(screen, bitmap, cliprect, primasks, 48, -116);
#endif
	return 0;
}
