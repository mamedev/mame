// license:BSD-3-Clause
// copyright-holders:David Graves
#include "emu.h"
#include "includes/topspeed.h"
#include "screen.h"


/****************************************************************************

                                     NOTES

Raster line color control
-------------------------

Used to make the road move. Each word controls one pixel row.

0x800000 - 0x1ff  raster color control for one road tilemap
0x800200 - 0x3ff  raster color control for the other

Road tile colors are (all?) in the range 0x100-103. Top road section
(tilemap at 0xa08000) uses 0x100 and 0x101. Bottom section
(tilemap at 0xb00000) uses 0x102 and 0x103. This would allow colors
on left and right side of road to be different. In practice it seems
Taito didn't take advantage of this.

Each tilemap is usually all one color value. Every now and then (10s
or so) the value alternates. This seems to be determined by whether
the current section of road has white lines in the middle. (0x101/3
gives white lines.)

The raster line color control area has groups of four values which
cascade down through it so the road colors cascade down the screen.

There are three known groups (start is arbitrary; the cycles repeat ad
infinitum or until a different cycle starts; values given are from bottom
to top of screen):

(i) White lines in center of road

12  %10010
1f  %11111
00  %00000
0d  %01101

(ii) No lines in center of road

08  %01000
0c  %01100
1a  %11010
1e  %11110

(iii) Under bridge or in tunnel [note almost identical to (i)]

ffe0    %00000
ffed    %01101
fff2    %10010
ffef    %01111

(iv) Unknown 4th group for tunnels in later parts of the game that have
no white lines, analogous to (ii) ?


Correlating with screenshots suggests that these bits refer to:

x....  road body ?
.x...  lines in road center and inner edge
..x..  lines at road outer edge
...x.  outside road ?
....x  ???


Actual gfx tiles used for the road only use colors 1-5. Palette offsets:

(0 = transparency)
1 = lines in road center
2 = road edge (inner)
3 = road edge (outer)
4 = road body
5 = outside road

Each palette block contains three possible sets of 5 colors. Entries 1-5
(standard), 6-10 (alternate), 11-15 (tunnels).

In tunnels only 11-15 are used. Outside tunnels there is a choice between
the standard colors and the alternate colors. The road body could in theory
take a standard color while 'outside the road' took on an alternate. But
in practice the game is using a very limited choice of raster control words,
so we don't know.

Need to test whether sections of the road with unknown raster control words
(tunnels late in the game without central white lines) are correct against
a real machine.

Also are the 'prelines' shortly before white road lines appear correct?



CHECK screen inits at $1692

These suggest that rowscroll areas are all 0x1000 long and there are TWO
for each tilemap layer.

256 rows => 256 words => 0x200 bytes. So probably the inits are far too long.

Maybe the second area for each layer contains colscroll ?

****************************************************************************/


/****************************************************************************

                                     SPRITES

    Layout 8 bytes per sprite
    -------------------------

    +0x00   xxxxxxx. ........   Zoom Y
            .......x xxxxxxxx   Y

    +0x02   x....... ........   Flip Y
            ........ .xxxxxxx   Zoom X

    +0x04   x....... ........   Priority
            .x...... ........   Flip X
            ..x..... ........   Unknown
            .......x xxxxxxxx   X

    +0x06   xxxxxxxx ........   Color
            ........ xxxxxxxx   Tile number

********************************************************************************/

void topspeed_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	static const u32 primasks[2] = { 0xff00, 0xfffc };  /* Sprites are over bottom layer or under top layer */

	/* Most of spriteram is not used by the 68000: rest is scratch space for the h/w perhaps ? */
	for (int offs = 0; offs < m_spriteram.bytes() / 2; offs += 4)
	{
		const u16 data = m_spriteram[offs + 2];

		const int tilenum = m_spriteram[offs + 3] & 0xff;
		const u32 color = (m_spriteram[offs + 3] & 0xff00) >> 8;
		const int flipx = (data & 0x4000) >> 14;
		const int flipy = (m_spriteram[offs + 1] & 0x8000) >> 15;
		int x = data & 0x1ff;
		int y = m_spriteram[offs] & 0x1ff;
		int zoomx = (m_spriteram[offs + 1]& 0x7f);
		int zoomy = (m_spriteram[offs] & 0xfe00) >> 9;
		const int priority = (data & 0x8000) >> 15;
//      unknown = (data & 0x2000) >> 13;

		/* End of sprite list */
		if (y == 0x180)
			break;

		const int map_offset = tilenum << 7;

		zoomx += 1;
		zoomy += 1;

		y += 3 + (128-zoomy);

		/* treat coords as signed */
		if (x > 0x140) x -= 0x200;
		if (y > 0x140) y -= 0x200;

		int bad_chunks = 0;

		for (int sprite_chunk = 0; sprite_chunk < 128; sprite_chunk++)
		{
			const int k = sprite_chunk % 8;   /* 8 sprite chunks per row */
			const int j = sprite_chunk / 8;   /* 16 rows */

			/* pick tiles back to front for x and y flips */
			const int px = (flipx) ?  (7 - k) : (k);
			const int py = (flipy) ? (15 - j) : (j);

			const u16 code = m_spritemap[map_offset + (py << 3) + px];

			if (code & 0x8000)
			{
				bad_chunks += 1;
				continue;
			}

			const int curx = x + ((k * zoomx) / 8);
			const int cury = y + ((j * zoomy) / 16);

			const int zx = x + (((k + 1) * zoomx) / 8) - curx;
			const int zy = y + (((j + 1) * zoomy) / 16) - cury;

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


/***************************************************************************/

u32 topspeed_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 layer[4];

#ifdef MAME_DEBUG
	if (machine().input().code_pressed_once (KEYCODE_V))
	{
		m_dislayer[0] ^= 1;
		popmessage("bg: %01x", m_dislayer[0]);
	}

	if (machine().input().code_pressed_once (KEYCODE_B))
	{
		m_dislayer[1] ^= 1;
		popmessage("fg: %01x", m_dislayer[1]);
	}

	if (machine().input().code_pressed_once (KEYCODE_N))
	{
		m_dislayer[2] ^= 1;
		popmessage("bg2: %01x", m_dislayer[2]);
	}

	if (machine().input().code_pressed_once (KEYCODE_M))
	{
		m_dislayer[3] ^= 1;
		popmessage("fg2: %01x", m_dislayer[3]);
	}

	if (machine().input().code_pressed_once (KEYCODE_C))
	{
		m_dislayer[4] ^= 1;
		popmessage("sprites: %01x", m_dislayer[4]);
	}
#endif

	m_pc080sn[0]->tilemap_update();
	m_pc080sn[1]->tilemap_update();

	/* Tilemap layer priority seems hardwired (the order is odd, too) */
	layer[0] = 1;
	layer[1] = 0;
	layer[2] = 1;
	layer[3] = 0;

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);

#ifdef MAME_DEBUG
	if (m_dislayer[3] == 0)
#endif
	m_pc080sn[1]->tilemap_draw(screen, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);

#ifdef MAME_DEBUG
	if (m_dislayer[2] == 0)
#endif
	m_pc080sn[1]->tilemap_draw_special(screen, bitmap, cliprect, layer[1], 0, 2, m_raster_ctrl);

#ifdef MAME_DEBUG
	if (m_dislayer[1] == 0)
#endif
	m_pc080sn[0]->tilemap_draw_special(screen, bitmap, cliprect, layer[2], 0, 4, m_raster_ctrl + 0x100);

#ifdef MAME_DEBUG
	if (m_dislayer[0] == 0)
#endif
	m_pc080sn[0]->tilemap_draw(screen, bitmap, cliprect, layer[3], 0, 8);

#ifdef MAME_DEBUG
	if (m_dislayer[4] == 0)
#endif

	draw_sprites(screen,bitmap,cliprect);
	return 0;
}
