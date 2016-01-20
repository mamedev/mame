// license:BSD-3-Clause
// copyright-holders:David Graves
#include "emu.h"
#include "includes/othunder.h"


void othunder_state::video_start()
{
	/* Up to $800/8 big sprites, requires 0x100 * sizeof(*spritelist)
	   Multiply this by 32 to give room for the number of small sprites,
	   which are what actually get put in the structure. */
	m_spritelist = std::make_unique<othunder_tempsprite[]>(0x2000);
}


/************************************************************
            SPRITE DRAW ROUTINE

It draws a series of small tiles ("chunks") together to
create a big sprite. The spritemap rom provides the lookup
table for this. We look up the 16x8 sprite chunks from
the spritemap rom, creating each 64x64 sprite as follows:

     0  1  2  3
     4  5  6  7
     8  9 10 11
    12 13 14 15
    16 17 18 19
    20 21 22 23
    24 25 26 27
    28 29 30 31

The game makes heavy use of sprite zooming.

        ***

NB: unused portions of the spritemap rom contain hex FF's.
It is a useful coding check to warn in the log if these
are being accessed. [They can be inadvertently while
spriteram is being tested, take no notice of that.]


        Othunder (modified table from Raine)

        Byte | Bit(s) | Description
        -----+76543210+-------------------------------------
          0  |xxxxxxx.| ZoomY (0 min, 63 max - msb unused as sprites are 64x64)
          0  |.......x| Y position (High)
          1  |xxxxxxxx| Y position (Low)
          2  |x.......| Sprite/BG Priority (0=sprites high)
          2  |.x......| Flip X
          2  |..?????.| unknown/unused ?
          2  |.......x| X position (High)
          3  |xxxxxxxx| X position (Low)
          4  |xxxxxxxx| Palette bank
          5  |?.......| unknown/unused ?
          5  |.xxxxxxx| ZoomX (0 min, 63 max - msb unused as sprites are 64x64)
          6  |x.......| Flip Y
          6  |.??.....| unknown/unused ?
          6  |...xxxxx| Sprite Tile high (2 msbs unused - 3/4 of spritemap rom empty)
          7  |xxxxxxxx| Sprite Tile low

********************************************************/


void othunder_state::draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, const int *primasks, int y_offs )
{
	UINT16 *spritemap = (UINT16 *)memregion("user1")->base();
	UINT16 tile_mask = (m_gfxdecode->gfx(0)->elements()) - 1;
	UINT16 *spriteram16 = m_spriteram;
	int offs, data, tilenum, color, flipx, flipy;
	int x, y, priority, curx, cury;
	int sprites_flipscreen = 0;
	int zoomx, zoomy, zx, zy;
	int sprite_chunk, map_offset, code, j, k, px, py;
	int bad_chunks;

	/* pdrawgfx() needs us to draw sprites front to back, so we have to build a list
	   while processing sprite ram and then draw them all at the end */
	struct othunder_tempsprite *sprite_ptr = m_spritelist.get();

	for (offs = (m_spriteram.bytes() / 2) - 4; offs >= 0; offs -= 4)
	{
		data = spriteram16[offs + 0];
		zoomy = (data & 0xfe00) >> 9;
		y = data & 0x1ff;

		data = spriteram16[offs + 1];
		flipx = (data & 0x4000) >> 14;
		priority = (data & 0x8000) >> 15;
		x = data & 0x1ff;

		data = spriteram16[offs + 2];
		color = (data & 0xff00) >> 8;
		zoomx = (data & 0x7f);

		data = spriteram16[offs + 3];
		tilenum = data & 0x1fff;    // $80000 spritemap rom maps up to $2000 64x64 sprites
		flipy = (data & 0x8000) >> 15;

		if (!tilenum)
			continue;

		map_offset = tilenum << 5;

		zoomx += 1;
		zoomy += 1;

		y += y_offs;

		/* treat coords as signed */
		if (x > 0x140) x -= 0x200;
		if (y > 0x140) y -= 0x200;

		bad_chunks = 0;

		for (sprite_chunk = 0; sprite_chunk < 32; sprite_chunk++)
		{
			k = sprite_chunk % 4;   /* 4 chunks per row */
			j = sprite_chunk / 4;   /* 8 rows */

			px = k;
			py = j;
			if (flipx)  px = 3 - k; /* pick tiles back to front for x and y flips */
			if (flipy)  py = 7 - j;

			code = spritemap[map_offset + px + (py << 2)] & tile_mask;

			if (code == 0xffff)
			{
				bad_chunks += 1;
				continue;
			}

			curx = x + ((k * zoomx) / 4);
			cury = y + ((j * zoomy) / 8);

			zx= x + (((k + 1) * zoomx) / 4) - curx;
			zy= y + (((j + 1) * zoomy) / 8) - cury;

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

			sprite_ptr->code = code;
			sprite_ptr->color = color;
			sprite_ptr->flipx = flipx;
			sprite_ptr->flipy = flipy;
			sprite_ptr->x = curx;
			sprite_ptr->y = cury;
			sprite_ptr->zoomx = zx << 12;
			sprite_ptr->zoomy = zy << 13;

			if (primasks)
			{
				sprite_ptr->primask = primasks[priority];
				sprite_ptr++;
			}
			else
			{
				m_gfxdecode->gfx(0)->zoom_transpen(bitmap,cliprect,
						sprite_ptr->code,
						sprite_ptr->color,
						sprite_ptr->flipx,sprite_ptr->flipy,
						sprite_ptr->x,sprite_ptr->y,
						sprite_ptr->zoomx,sprite_ptr->zoomy,0);
			}
		}

		if (bad_chunks)
logerror("Sprite number %04x had %02x invalid chunks\n",tilenum,bad_chunks);
	}

	/* this happens only if primsks != NULL */
	while (sprite_ptr != m_spritelist.get())
	{
		sprite_ptr--;

		m_gfxdecode->gfx(0)->prio_zoom_transpen(bitmap,cliprect,
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

UINT32 othunder_state::screen_update_othunder(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layer[3];

	m_tc0100scn->tilemap_update();

	layer[0] = m_tc0100scn->bottomlayer();
	layer[1] = layer[0] ^ 1;
	layer[2] = 2;

	screen.priority().fill(0, cliprect);

	/* Ensure screen blanked even when bottom layer not drawn due to disable bit */
	bitmap.fill(0, cliprect);

	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);
	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 2);
	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 4);

	/* Sprites can be under/over the layer below text layer */
	{
		static const int primasks[2] = {0xf0, 0xfc};
		draw_sprites(screen, bitmap, cliprect, primasks, 3);
	}

	return 0;
}
