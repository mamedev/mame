// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Graves
#include "emu.h"
#include "includes/groundfx.h"

/******************************************************************/

void groundfx_state::video_start()
{
	m_spritelist = auto_alloc_array(machine(), struct gfx_tempsprite, 0x4000);

	/* Hack */
	m_hack_cliprect.set(69, 250, 24 + 5, 24 + 44);
}

/***************************************************************
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

    [*  00=over BG0, 01=BG1, 10=BG2, 11=BG3 ]
    [or 00=over BG1, 01=BG2, 10=BG3, 11=BG3 ]

***************************************************************/

void groundfx_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect,int do_hack,int x_offs,int y_offs)
{
	UINT32 *spriteram32 = m_spriteram;
	UINT16 *spritemap = (UINT16 *)memregion("user1")->base();
	int offs, data, tilenum, color, flipx, flipy;
	int x, y, priority, dblsize, curx, cury;
	int sprites_flipscreen = 0;
	int zoomx, zoomy, zx, zy;
	int sprite_chunk,map_offset,code,j,k,px,py;
	int dimension,total_chunks;
	static const int primasks[4] = {0xffff, 0xfffc, 0xfff0, 0xff00 };

	/* pdrawgfx() needs us to draw sprites front to back, so we have to build a list
	   while processing sprite ram and then draw them all at the end */
	struct gfx_tempsprite *sprite_ptr = m_spritelist;

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

//      color |= (0x100 + (priority << 6));     /* priority bits select color bank */
		color /= 2;     /* as sprites are 5bpp */
		flipy = !flipy;
		y = (-y &0x3ff);

		if (!tilenum) continue;

		flipy = !flipy;
		zoomx += 1;
		zoomy += 1;

		y += y_offs;

		/* treat coords as signed */
		if (x>0x340) x -= 0x400;
		if (y>0x340) y -= 0x400;

		x -= x_offs;

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
				sprite_ptr->pri = priority;
				sprite_ptr++;
			}
		}
	}

	/* this happens only if primsks != NULL */
	while (sprite_ptr != m_spritelist)
	{
		const rectangle *clipper;

		sprite_ptr--;

		if (do_hack && sprite_ptr->pri==1 && sprite_ptr->y<100)
			clipper=&m_hack_cliprect;
		else
			clipper=&cliprect;

		m_gfxdecode->gfx(sprite_ptr->gfx)->prio_zoom_transpen(bitmap,*clipper,
				sprite_ptr->code,
				sprite_ptr->color,
				sprite_ptr->flipx,sprite_ptr->flipy,
				sprite_ptr->x,sprite_ptr->y,
				sprite_ptr->zoomx,sprite_ptr->zoomy,
				screen.priority(),primasks[sprite_ptr->pri],0);
	}
}

/**************************************************************
                SCREEN REFRESH
**************************************************************/

UINT32 groundfx_state::screen_update_groundfx(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space &space = machine().driver_data()->generic_space();
	UINT8 layer[5];
	UINT8 scclayer[3];
	UINT16 priority;

	m_tc0100scn->tilemap_update();
	m_tc0480scp->tilemap_update();

	priority = m_tc0480scp->get_bg_priority();

	layer[0] = (priority & 0xf000) >> 12;   /* tells us which bg layer is bottom */
	layer[1] = (priority & 0x0f00) >>  8;
	layer[2] = (priority & 0x00f0) >>  4;
	layer[3] = (priority & 0x000f) >>  0;   /* tells us which is top */
	layer[4] = 4;   /* text layer always over bg layers */

	scclayer[0] = m_tc0100scn->bottomlayer();
	scclayer[1] = scclayer[0]^1;
	scclayer[2] = 2;

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);   /* wrong color? */

	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, scclayer[0], TILEMAP_DRAW_OPAQUE, 0);
	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, scclayer[1], 0, 0);

	/*  BIG HACK!

	    The rear view mirror is a big priority trick - the text
	    layer of TC0100SCN is used as a stencil to display
	    the bottom layer of TC0480SCP and a particular sprite
	    priority.  These never appear outside of the stencil.

	    I'm not sure how the game turns this effect on/off
	    (the 480 layer is used normally in the frontend
	    of the game).

	    I haven't implemented it properly yet, instead I'm
	    doing a hacky cliprect around the rearview and drawing
	    it's contents the usual way.

	*/
	if (m_tc0100scn->long_r(space, 0x4090 / 4, 0xffffffff) ||
			m_tc0480scp->long_r(space, 0x20 / 4, 0xffffffff) == 0x240866)  /* Anything in text layer - really stupid hack */
	{
		m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 2);
		m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 4);
		m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[3], 0, 8);

		//m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, 0, scclayer[2], 0, 0);

		if (m_tc0480scp->long_r(space, 0x20 / 4, 0xffffffff) != 0x240866) /* Stupid hack for start of race */
			m_tc0480scp->tilemap_draw(screen, bitmap, m_hack_cliprect, layer[0], 0, 0);
		draw_sprites(screen, bitmap, cliprect, 1, 44, -574);
	}
	else
	{
		m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[0], 0, 1);
		m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 2);
		m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 4);
		m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[3], 0, 8);

		m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, scclayer[2], 0, 0);

		draw_sprites(screen, bitmap, cliprect, 0, 44, -574);
	}

	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[4], 0, 0);    /* TC0480SCP text layer */
	return 0;
}
