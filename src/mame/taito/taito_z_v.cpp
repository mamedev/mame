// license:BSD-3-Clause
// copyright-holders:David Graves
#include "emu.h"
#include "taito_z.h"


/**********************************************************/

void contcirc_state::video_start()
{
	taitoz_z80_sound_state::video_start();

	m_road_palbank = 3;

	save_item(NAME(m_road_palbank));
}

void sci_state::video_start()
{
	taitoz_z80_sound_state::video_start();

	m_sci_spriteframe = 0;

	save_item(NAME(m_sci_spriteframe));
}


/********************************************************
            SPRITE READ AND WRITE HANDLERS
********************************************************/

u16 sci_state::sci_spriteframe_r()
{
	return m_sci_spriteframe << 8;
}

void sci_state::sci_spriteframe_w(u16 data)
{
	m_sci_spriteframe = (data >> 8) & 0xff;
}


/************************************************************
                   SPRITE DRAW ROUTINES

These draw a series of small tiles ("chunks") together to
create each big sprite. The spritemap rom provides the lookup
table for this. E.g. Spacegun looks up 16x8 sprite chunks
from the spritemap rom, creating this 64x64 sprite (numbers
are the word offset into the spritemap rom):

     0  1  2  3
     4  5  6  7
     8  9 10 11
    12 13 14 15
    16 17 18 19
    20 21 22 23
    24 25 26 27
    28 29 30 31

Chasehq/Nightstr are the only games to build from 16x16
tiles. They are also more complicated to draw, as they have
3 different aggregation formats [32/64/128 x 128]
whereas the other games stick to one, typically 64x64.

All the games make heavy use of sprite zooming.

I'm 99% sure there are probably just two levels of sprite
priority - under and over the road - but this isn't certain.

        ***

[The routines for the 16x8 tile games seem to have large
common elements that could be extracted as subroutines.]

NB: unused portions of the spritemap rom contain hex FF's.
It is a useful coding check to warn in the log if these
are being accessed. [They can be inadvertently while
spriteram is being tested, take no notice of that.]

BUT: Nightstr uses code 0x3ff as a mask sprite. This
references 0x1ff00-ff in spritemap rom, which is an 0xff fill.
This must be accessing sprite chunk 0x3fff: in other words the
top bits are thrown away and so accessing chunk 0xfe45 would
display chunk 0x3e45 etc.


        Aquajack/Spacegun (modified table from Raine; size of bit
        masks verified in Spacegun code)

        Byte | Bit(s) | Description
        -----+76543210+-------------------------------------
          0  |xxxxxxx.| ZoomY (0 min, 63 max - msb unused as sprites are 64x64)
          0  |.......x| Y position (High)
          1  |xxxxxxxx| Y position (Low)
          2  |x.......| Priority (0=sprites high)
          2  |.x......| Flip X
          2  |..?????.| unknown/unused ?
          2  |.......x| X position (High)
          3  |xxxxxxxx| X position (Low)
          4  |xxxxxxxx| Palette bank
          5  |?.......| unknown/unused ?
          5  |.xxxxxxx| ZoomX (0 min, 63 max - msb unused as sprites are 64x64)
          6  |x.......| Flip Y ?
          6  |.??.....| unknown/unused ?
          6  |...xxxxx| Sprite Tile high (msb unused - half of spritemap rom empty)
          7  |xxxxxxxx| Sprite Tile low

        Continental circus (modified Raine table): note similar format.
        The zoom msb is actually used, as sprites are 128x128.

        ---+-------------------+--------------
         0 | xxxxxxx. ........ | ZoomY
         0 | .......x xxxxxxxx | Y position
                        // unsure about Flip Y
         2 | .....xxx xxxxxxxx | Sprite Tile
         4 | x....... ........ | Priority (0=sprites high)
         4 | .x...... ........ | Flip X
         4 | .......x xxxxxxxx | X position
         6 | xxxxxxxx ........ | Palette bank
         6 | ........ .xxxxxxx | ZoomX
        ---+-------------------+--------------

         Bshark/Chasehq/Nightstr/SCI (modified Raine table): similar format.
         The zoom msb is only used for 128x128 sprites.

        -----+--------+------------------------
        Byte | Bit(s) | Description
        -----+76543210+------------------------
          0  |xxxxxxx.| ZoomY
          0  |.......x| Y position (High)
          1  |xxxxxxxx| Y position (Low)
          2  |x.......| Priority (0=sprites high)
          2  |.xxxxxxx| Palette bank (High)
          3  |x.......| Palette bank (Low)
          3  |.xxxxxxx| ZoomX
          4  |x.......| Flip Y
          4  |.x......| Flip X
          4  |.......x| X position (High)
          5  |xxxxxxxx| X position (Low)
          6  |...xxxxx| Sprite Tile (High)
          7  |xxxxxxxx| Sprite Tile (Low)
        -----+--------+------------------------

 [Raine Chasehq sprite plotting is peculiar. It determines
 the type of big sprite by reference to the zoomx and y.
 Therefore it says that the big sprite number is 0-0x7ff
 across ALL three sizes and you can't distinguish them by
 tilenum. FWIW I seem to be ok just using zoomx.]

TODO: Contcirc, Aquajack, Spacegun need flip y bit to be
confirmed

********************************************************/


void contcirc_state::contcirc_draw_sprites_16x8(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int y_offs)
{
	int sprites_flipscreen = 0;
	static const u32 primasks[2] = { 0xf0, 0xfc };

	for (int offs = 0; offs < m_spriteram.bytes() / 2; offs += 4)
	{
		u16 data = m_spriteram[offs + 0];
		int zoomy = (data & 0xfe00) >> 9;
		int y =      data & 0x01ff;

		data = m_spriteram[offs + 1];
		const u32 tilenum = data & 0x7ff;     /* $80000 spritemap rom maps up to $7ff 128x128 sprites */

		data = m_spriteram[offs + 2];
		const u32 priority = (data & 0x8000) >> 15;
		int flipx =          (data & 0x4000) >> 14;
		int flipy =          (data & 0x2000) >> 13;  // ???
		int x =               data & 0x01ff;   // correct mask?

		data = m_spriteram[offs + 3];
		const u32 color = (data & 0xff00) >> 8;
		int zoomx =       (data & 0x007f);

		if (!tilenum)
			continue;

		const u32 map_offset = tilenum << 7;

		zoomx += 1;
		zoomy += 1;

		y += y_offs;
		y += (128 - zoomy);

		/* treat coords as signed */
		if (x > 0x140) x -= 0x200;
		if (y > 0x140) y -= 0x200;

		int bad_chunks = 0;

		for (int sprite_chunk = 0; sprite_chunk < 128; sprite_chunk++)
		{
			const int k = sprite_chunk % 8;   /* 8 sprite chunks per row */
			const int j = sprite_chunk / 8;   /* 16 rows */

			int px = flipx ?  (7 - k) : k;  /* pick tiles back to front for x and y flips */
			int py = flipy ? (15 - j) : j;

			const u16 code = m_spritemap[map_offset + px + (py << 3)];

			if (code == 0xffff)
				bad_chunks++;

			int curx = x + ((k * zoomx) / 8);
			int cury = y + ((j * zoomy) / 16);

			const int zx = x + (((k + 1) * zoomx) / 8) - curx;
			const int zy = y + (((j + 1) * zoomy) / 16) - cury;

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

			m_gfxdecode->gfx(0)->prio_zoom_transpen(bitmap,cliprect,
					code,
					color,
					flipx,flipy,
					curx,cury,
					zx << 12,zy << 13,
					screen.priority(),primasks[priority],0);
		}

		if (bad_chunks)
			logerror("Sprite number %04x had %02x invalid chunks\n", tilenum, bad_chunks);
	}
}



void taitoz_z80_sound_state::chasehq_draw_sprites_16x16(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int y_offs)
{
	int sprites_flipscreen = 0;
	static const u32 primasks[2] = { 0xf0, 0xfc };

	for (int offs = m_spriteram.bytes() / 2 - 4; offs >= 0; offs -= 4)
	{
		u16 data = m_spriteram[offs + 0];
		int zoomy = (data & 0xfe00) >> 9;
		int y =      data & 0x01ff;

		data = m_spriteram[offs + 1];
		const u32 priority = (data & 0x8000) >> 15;
		const u32 color =    (data & 0x7f80) >> 7;
		int zoomx =          (data & 0x007f);

		data = m_spriteram[offs + 2];
		int flipy = (data & 0x8000) >> 15;
		int flipx = (data & 0x4000) >> 14;
		int x =      data & 0x01ff;

		data = m_spriteram[offs + 3];
		/* higher bits are sometimes used... e.g. sign over flashing enemy car...! */
		const u32 tilenum = data & 0x7ff;

		if (!tilenum)
			continue;

		zoomx += 1;
		zoomy += 1;

		y += y_offs;
		y += (128 - zoomy);

		/* treat coords as signed */
		if (x > 0x140) x -= 0x200;
		if (y > 0x140) y -= 0x200;

		int bad_chunks = 0;

		if ((zoomx - 1) & 0x40) /* 128x128 sprites, $0-$3ffff in spritemap rom, OBJA */
		{
			const u32 map_offset = tilenum << 6;

			for (int sprite_chunk = 0; sprite_chunk < 64; sprite_chunk++)
			{
				const int j = sprite_chunk / 8;   /* 8 rows */
				const int k = sprite_chunk % 8;   /* 8 sprite chunks per row */

				int px = flipx ? (7 - k) : k;   /* pick tiles back to front for x and y flips */
				int py = flipy ? (7 - j) : j;

				const u16 code = m_spritemap[map_offset + px + (py << 3)];

				if (code == 0xffff)
					bad_chunks++;

				int curx = x + ((k * zoomx) / 8);
				int cury = y + ((j * zoomy) / 8);

				const int zx = x + (((k + 1) * zoomx) / 8) - curx;
				const int zy = y + (((j + 1) * zoomy) / 8) - cury;

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

				m_gfxdecode->gfx(0)->prio_zoom_transpen(bitmap,cliprect,
						code,
						color,
						flipx,flipy,
						curx,cury,
						zx << 12,zy << 12,
						screen.priority(),primasks[priority],0);
			}
		}
		else if ((zoomx - 1) & 0x20)    /* 64x128 sprites, $40000-$5ffff in spritemap rom, OBJB */
		{
			const u32 map_offset = (tilenum << 5) + 0x20000;

			for (int sprite_chunk = 0; sprite_chunk < 32; sprite_chunk++)
			{
				const int j = sprite_chunk / 4;   /* 8 rows */
				const int k = sprite_chunk % 4;   /* 4 sprite chunks per row */

				int px = flipx ? (3 - k) : k;   /* pick tiles back to front for x and y flips */
				int py = flipy ? (7 - j) : j;

				const u16 code = m_spritemap[map_offset + px + (py << 2)];

				if (code == 0xffff) bad_chunks++;

				int curx = x + ((k * zoomx) / 4);
				int cury = y + ((j * zoomy) / 8);

				const int zx = x + (((k + 1) * zoomx) / 4) - curx;
				const int zy = y + (((j + 1) * zoomy) / 8) - cury;

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

				m_gfxdecode->gfx(1)->prio_zoom_transpen(bitmap,cliprect,
						code,
						color,
						flipx,flipy,
						curx,cury,
						zx << 12,zy << 12,
						screen.priority(),primasks[priority],0);
			}
		}
		else if (!((zoomx - 1) & 0x60)) /* 32x128 sprites, $60000-$7ffff in spritemap rom, OBJB */
		{
			const u32 map_offset = (tilenum << 4) + 0x30000;

			for (int sprite_chunk = 0; sprite_chunk < 16; sprite_chunk++)
			{
				const int j = sprite_chunk / 2;   /* 8 rows */
				const int k = sprite_chunk % 2;   /* 2 sprite chunks per row */

				int px = flipx ? (1 - k) : k;   /* pick tiles back to front for x and y flips */
				int py = flipy ? (7 - j) : j;

				const u16 code = m_spritemap[map_offset + px + (py << 1)];

				if (code == 0xffff) bad_chunks ++;

				int curx = x + ((k * zoomx) / 2);
				int cury = y + ((j * zoomy) / 8);

				const int zx = x + (((k + 1) * zoomx) / 2) - curx;
				const int zy = y + (((j + 1) * zoomy) / 8) - cury;

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

				m_gfxdecode->gfx(1)->prio_zoom_transpen(bitmap,cliprect,
						code,
						color,
						flipx,flipy,
						curx,cury,
						zx << 12,zy << 12,
						screen.priority(),primasks[priority],0);
			}
		}

		if (bad_chunks)
			logerror("Sprite number %04x had %02x invalid chunks\n", tilenum, bad_chunks);
	}
}



void taitoz_state::bshark_draw_sprites_16x8(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int y_offs)
{
	int sprites_flipscreen = 0;
	static const u32 primasks[2] = { 0xf0, 0xfc };

	for (int offs = m_spriteram.bytes() / 2 - 4; offs >= 0; offs -= 4)
	{
		u16 data = m_spriteram[offs + 0];
		int zoomy = (data & 0x7e00) >> 9;
		int y =      data & 0x01ff;

		data = m_spriteram[offs + 1];
		const u32 priority = (data & 0x8000) >> 15;
		const u32 color =    (data & 0x7f80) >> 7;
		int zoomx =          (data & 0x003f);

		data = m_spriteram[offs + 2];
		int flipy = (data & 0x8000) >> 15;
		int flipx = (data & 0x4000) >> 14;
		int x =      data & 0x01ff;

		data = m_spriteram[offs + 3];
		const u32 tilenum = data & 0x1fff;    /* $80000 spritemap rom maps up to $2000 64x64 sprites */

		if (!tilenum)
			continue;

		const u32 map_offset = tilenum << 5;

		zoomx += 1;
		zoomy += 1;

		y += y_offs;
		y += (64 - zoomy);

		/* treat coords as signed */
		if (x > 0x140) x -= 0x200;
		if (y > 0x140) y -= 0x200;

		int bad_chunks = 0;

		for (int sprite_chunk = 0; sprite_chunk < 32; sprite_chunk++)
		{
			const int k = sprite_chunk % 4;   /* 4 sprite chunks per row */
			const int j = sprite_chunk / 4;   /* 8 rows */

			int px = flipx ? (3 - k) : k;   /* pick tiles back to front for x and y flips */
			int py = flipy ? (7 - j) : j;

			const u16 code = m_spritemap[map_offset + px + (py << 2)];

			if (code == 0xffff)
				bad_chunks++;

			int curx = x + ((k * zoomx) / 4);
			int cury = y + ((j * zoomy) / 8);

			const int zx = x + (((k + 1) * zoomx) / 4) - curx;
			const int zy = y + (((j + 1) * zoomy) / 8) - cury;

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

			m_gfxdecode->gfx(0)->prio_zoom_transpen(bitmap,cliprect,
					code,
					color,
					flipx,flipy,
					curx,cury,
					zx << 12,zy << 13,
					screen.priority(),primasks[priority],0);
		}

		if (bad_chunks)
			logerror("Sprite number %04x had %02x invalid chunks\n",tilenum,bad_chunks);
	}
}



void sci_state::sci_draw_sprites_16x8(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int y_offs)
{
	int sprites_flipscreen = 0;
	static const u32 primasks[2] = { 0xf0, 0xfc };

	/* SCI alternates between two areas of its spriteram */

	// This gave back to front frames causing bad flicker... but
	// reversing it now only gives us sprite updates on alternate
	// frames. So we probably have to partly buffer spriteram?

	int start_offs = (m_sci_spriteframe & 1) * 0x800;
	start_offs = 0x800 - start_offs;

	for (int offs = (start_offs + 0x800 - 4); offs >= start_offs; offs -= 4)
	{
		u16 data = m_spriteram[offs + 0];
		int zoomy = (data & 0x7e00) >> 9;
		int y =      data & 0x01ff;

		data = m_spriteram[offs + 1];
		const u32 priority = (data & 0x8000) >> 15;
		const u32 color =    (data & 0x7f80) >> 7;
		int zoomx =          (data & 0x003f);

		data = m_spriteram[offs + 2];
		int flipy = (data & 0x8000) >> 15;
		int flipx = (data & 0x4000) >> 14;
		int x =      data & 0x01ff;

		data = m_spriteram[offs + 3];
		const u32 tilenum = data & 0x1fff;    /* $80000 spritemap rom maps up to $2000 64x64 sprites */

		if (!tilenum)
			continue;

		const u32 map_offset = tilenum << 5;

		zoomx += 1;
		zoomy += 1;

		y += y_offs;
		y += (64 - zoomy);

		/* treat coords as signed */
		if (x > 0x140) x -= 0x200;
		if (y > 0x140) y -= 0x200;

		int bad_chunks = 0;

		for (int sprite_chunk = 0; sprite_chunk < 32; sprite_chunk++)
		{
			const int j = sprite_chunk / 4;   /* 8 rows */
			const int k = sprite_chunk % 4;   /* 4 sprite chunks per row */

			int px = flipx ? (3 - k) : k;   /* pick tiles back to front for x and y flips */
			int py = flipy ? (7 - j) : j;

			const u16 code = m_spritemap[map_offset + px + (py << 2)];

			if (code == 0xffff)
				bad_chunks++;

			int curx = x + ((k * zoomx) / 4);
			int cury = y + ((j * zoomy) / 8);

			const int zx = x + (((k + 1) * zoomx) / 4) - curx;
			const int zy = y + (((j + 1) * zoomy) / 8) - cury;

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

			m_gfxdecode->gfx(0)->prio_zoom_transpen(bitmap,cliprect,
					code,
					color,
					flipx,flipy,
					curx,cury,
					zx << 12,zy << 13,
					screen.priority(),primasks[priority],0);
		}

		if (bad_chunks)
			logerror("Sprite number %04x had %02x invalid chunks\n",tilenum,bad_chunks);
	}
}



void taitoz_z80_sound_state::aquajack_draw_sprites_16x8(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect,int y_offs)
{
	int sprites_flipscreen = 0;
	static const u32 primasks[2] = { 0xf0, 0xfc };

	for (int offs = 0; offs < m_spriteram.bytes() / 2; offs += 4)
	{
		u16 data = m_spriteram[offs + 0];
		int zoomy = (data & 0x7e00) >> 9;
		int y =      data & 0x01ff;

		data = m_spriteram[offs + 1];
		const u32 priority = (data & 0x8000) >> 15;
		int flipx =          (data & 0x4000) >> 14;
		int x =               data & 0x01ff;   // correct mask?

		data = m_spriteram[offs + 2];
		const u32 color = (data & 0xff00) >> 8;
		int zoomx =       (data & 0x003f);

		data = m_spriteram[offs + 3];
		int flipy =        (data & 0x8000) >> 15;  // ???
		const u32 tilenum = data & 0x1fff;    /* $80000 spritemap rom maps up to $2000 64x64 sprites */

		if (!tilenum)
			continue;

		const u32 map_offset = tilenum << 5;

		zoomx += 1;
		zoomy += 1;

		y += y_offs;

		/* treat coords as signed */
		if (x > 0x140) x -= 0x200;
		if (y > 0x140) y -= 0x200;

		int bad_chunks = 0;

		for (int sprite_chunk=0;sprite_chunk<32;sprite_chunk++)
		{
			const int k = sprite_chunk % 4;   /* 4 sprite chunks per row */
			const int j = sprite_chunk / 4;   /* 8 rows */

			int px = flipx ? (3 - k) : k;   /* pick tiles back to front for x and y flips */
			int py = flipy ? (7 - j) : j;

			const u16 code = m_spritemap[map_offset + px + (py << 2)];

			if (code == 0xffff)
				bad_chunks++;

			int curx = x + ((k * zoomx)/4);
			int cury = y + ((j * zoomy)/8);

			const int zx = x + (((k + 1) * zoomx)/4) - curx;
			const int zy = y + (((j + 1) * zoomy)/8) - cury;

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

			m_gfxdecode->gfx(0)->prio_zoom_transpen(bitmap,cliprect,
					code,
					color,
					flipx,flipy,
					curx,cury,
					zx << 12,zy << 13,
					screen.priority(),primasks[priority],0);
		}

		if (bad_chunks)
			logerror("Sprite number %04x had %02x invalid chunks\n",tilenum,bad_chunks);
	}
}



void spacegun_state::spacegun_draw_sprites_16x8(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect,int y_offs)
{
	int sprites_flipscreen = 0;
	static const u32 primasks[2] = { 0xf0, 0xfc };

	for (int offs = 0; offs < m_spriteram.bytes() / 2 - 4; offs += 4)
	{
		u16 data = m_spriteram[offs + 0];
		int zoomy = (data & 0xfe00) >> 9;
		int y =      data & 0x01ff;

		data = m_spriteram[offs + 1];
		const u32 priority = (data & 0x8000) >> 15;
		int flipx =          (data & 0x4000) >> 14;
		int x =               data & 0x01ff;   // correct mask?

		data = m_spriteram[offs + 2];
		const u32 color = (data & 0xff00) >> 8;
		int zoomx =       (data & 0x007f);

		data = m_spriteram[offs + 3];
		int flipy =        (data & 0x8000) >> 15;  // ???
		const u32 tilenum = data & 0x1fff;    /* $80000 spritemap rom maps up to $2000 64x64 sprites */

		if (!tilenum)
			continue;

		const u32 map_offset = tilenum << 5;

		zoomx += 1;
		zoomy += 1;

		y += y_offs;

		/* treat coords as signed */
		if (x > 0x140) x -= 0x200;
		if (y > 0x140) y -= 0x200;

		int bad_chunks = 0;

		for (int sprite_chunk = 0; sprite_chunk < 32; sprite_chunk++)
		{
			const int k = sprite_chunk % 4;   /* 4 sprite chunks per row */
			const int j = sprite_chunk / 4;   /* 8 rows */

			int px = flipx ? (3 - k) : k;   /* pick tiles back to front for x and y flips */
			int py = flipy ? (7 - j) : j;

			const u16 code = m_spritemap[map_offset + px + (py << 2)];

			if (code == 0xffff)
				bad_chunks++;

			int curx = x + ((k * zoomx) / 4);
			int cury = y + ((j * zoomy) / 8);

			const int zx = x + (((k + 1) * zoomx) / 4) - curx;
			const int zy = y + (((j + 1) * zoomy) / 8) - cury;

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

			m_gfxdecode->gfx(0)->prio_zoom_transpen(bitmap,cliprect,
					code,
					color,
					flipx,flipy,
					curx,cury,
					zx << 12,zy << 13,
					screen.priority(),primasks[priority],0);
		}

		if (bad_chunks)
			logerror("Sprite number %04x had %02x invalid chunks\n",tilenum,bad_chunks);
	}
}


/**************************************************************
                        SCREEN REFRESH
**************************************************************/

rgb_t taitoz_z80_sound_state::color_xbgr555(u16 data)
{
	return rgb_t(pal5bit(data >> 0), pal5bit(data >> 5), pal5bit(data >> 10));
}

rgb_t taitoz_state::color_xrgb555(u16 data)
{
	return rgb_t(pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));
}

void contcirc_state::contcirc_out_w(u8 data)
{
	/* bit 0 = reset sub CPU */
	m_audiocpu->set_input_line(INPUT_LINE_RESET, BIT(data, 0) ? CLEAR_LINE : ASSERT_LINE);

	/* bits 1-3 n.c. */

	/* 3d scope control */
	/* bit 4 = SCPSW */
	/* bit 5 = _SCP */
	if (~m_shutter_control & 2 && data & 0x20)
		m_shutter_toggle = 0;
	m_shutter_control = data >> 4 & 3;

	/* bits 6 and 7 select the road palette bank */
	m_road_palbank = (data & 0xc0) >> 6;
}

void contcirc_state::scope_vblank(int state)
{
	if (state)
	{
		// it outputs 3d scope shutters at vblank
		if (m_shutter_control & 1)
		{
			m_shutter_out[0] = m_shutter_toggle;
			m_shutter_out[1] = m_shutter_toggle ^ 1;
		}
		else
		{
			m_shutter_out[0] = 0;
			m_shutter_out[1] = 0;
		}

		if (m_shutter_control & 2)
			m_shutter_toggle ^= 1;
	}
}

u32 contcirc_state::screen_update_contcirc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 layer[3];

	m_tc0100scn->tilemap_update();

	layer[0] = m_tc0100scn->bottomlayer();
	layer[1] = layer[0] ^ 1;
	layer[2] = 2;

	screen.priority().fill(0, cliprect);

	bitmap.fill(0, cliprect);

	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 0);
	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 1);
	m_tc0150rod->draw(bitmap, cliprect, -3, m_road_palbank << 6, 1, 0, screen.priority(), 1, 2); // -6
	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 4);

	contcirc_draw_sprites_16x8(screen, bitmap, cliprect, 5); // 7
	return 0;
}


/* Nightstr and ChaseHQ */

u32 taitoz_z80_sound_state::screen_update_chasehq(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 layer[3];

	m_tc0100scn->tilemap_update();

	layer[0] = m_tc0100scn->bottomlayer();
	layer[1] = layer[0] ^ 1;
	layer[2] = 2;

	screen.priority().fill(0, cliprect);

	/* Ensure screen blanked even when bottom layer not drawn due to disable bit */
	bitmap.fill(0, cliprect);

	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 0);
	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 1);
	m_tc0150rod->draw(bitmap, cliprect, -1, 0xc0, 0, 0, screen.priority(), 1, 2);
	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 4);

	chasehq_draw_sprites_16x16(screen, bitmap, cliprect, 7);
	return 0;
}


u32 taitoz_state::screen_update_bshark(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 layer[3];

	m_tc0100scn->tilemap_update();

	layer[0] = m_tc0100scn->bottomlayer();
	layer[1] = layer[0] ^ 1;
	layer[2] = 2;

	screen.priority().fill(0, cliprect);

	/* Ensure screen blanked even when bottom layer not drawn due to disable bit */
	bitmap.fill(0, cliprect);

	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 0);
	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 1);
	m_tc0150rod->draw(bitmap, cliprect, -1, 0xc0, 0, 1, screen.priority(), 1, 2);
	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 4);

	bshark_draw_sprites_16x8(screen, bitmap, cliprect, 8);
	return 0;
}


u32 sci_state::screen_update_sci(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 layer[3];

	m_tc0100scn->tilemap_update();

	layer[0] = m_tc0100scn->bottomlayer();
	layer[1] = layer[0] ^ 1;
	layer[2] = 2;

	screen.priority().fill(0, cliprect);

	/* Ensure screen blanked even when bottom layer not drawn due to disable bit */
	bitmap.fill(0, cliprect);

	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 0);
	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 1);
	m_tc0150rod->draw(bitmap, cliprect, -1, 0xc0, 0, 0, screen.priority(), 1, 2);
	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 4);

	sci_draw_sprites_16x8(screen, bitmap, cliprect, 6);
	return 0;
}


u32 taitoz_z80_sound_state::screen_update_aquajack(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 layer[3];

	m_tc0100scn->tilemap_update();

	layer[0] = m_tc0100scn->bottomlayer();
	layer[1] = layer[0] ^ 1;
	layer[2] = 2;

	screen.priority().fill(0, cliprect);

	/* Ensure screen blanked even when bottom layer not drawn due to disable bit */
	bitmap.fill(0, cliprect);

	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 0);
	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 1);
	m_tc0150rod->draw(bitmap, cliprect, -1, 0, 2, 1, screen.priority(), 1, 2);
	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 4);

	aquajack_draw_sprites_16x8(screen, bitmap, cliprect, 3);
	return 0;
}


u32 spacegun_state::screen_update_spacegun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 layer[3];

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

	spacegun_draw_sprites_16x8(screen, bitmap, cliprect, 4);

	return 0;
}


u32 taitoz_z80_sound_state::screen_update_dblaxle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 layer[5];
	u16 priority;

	m_tc0480scp->tilemap_update();

	priority = m_tc0480scp->get_bg_priority();

	layer[0] = (priority & 0xf000) >> 12;   /* tells us which bg layer is bottom */
	layer[1] = (priority & 0x0f00) >>  8;
	layer[2] = (priority & 0x00f0) >>  4;
	layer[3] = (priority & 0x000f) >>  0;   /* tells us which is top */
	layer[4] = 4;   /* text layer always over bg layers */

	screen.priority().fill(0, cliprect);

	/* Ensure screen blanked - this shouldn't be necessary! */
	bitmap.fill(0, cliprect);

	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 0);
	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 0);
	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 1);

	m_tc0150rod->draw(bitmap, cliprect, -1, 0xc0, 0, 0, screen.priority(), 1, 2);
	bshark_draw_sprites_16x8(screen, bitmap, cliprect, 7);

	/* This layer used for the big numeric displays */
	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[3], 0, 4);

	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[4], 0, 0);  /* Text layer */
	return 0;
}


u32 sci_state::screen_update_racingb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 layer[5];
	u16 priority;

	m_tc0480scp->tilemap_update();

	priority = m_tc0480scp->get_bg_priority();

	layer[0] = (priority & 0xf000) >> 12;   /* tells us which bg layer is bottom */
	layer[1] = (priority & 0x0f00) >>  8;
	layer[2] = (priority & 0x00f0) >>  4;
	layer[3] = (priority & 0x000f) >>  0;   /* tells us which is top */
	layer[4] = 4;   /* text layer always over bg layers */

	screen.priority().fill(0, cliprect);

	/* Ensure screen blanked - this shouldn't be necessary! */
	bitmap.fill(0, cliprect);

	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 0);
	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 0);
	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 2);
	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[3], 0, 2);

	m_tc0150rod->draw(bitmap, cliprect, -1, 0xc0, 0, 0, screen.priority(), 1, 2);
	sci_draw_sprites_16x8(screen, bitmap, cliprect, 7);

	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[4], 0, 4);
	return 0;
}
