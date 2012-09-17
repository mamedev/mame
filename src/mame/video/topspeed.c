#include "emu.h"
#include "video/taitoic.h"
#include "includes/topspeed.h"


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

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	topspeed_state *state = machine.driver_data<topspeed_state>();
	UINT16 *spriteram = state->m_spriteram;
	int offs, map_offset, x, y, curx, cury, sprite_chunk;
	UINT16 *spritemap = state->m_spritemap;
	UINT16 data, tilenum, code, color;
	UINT8 flipx, flipy, priority, bad_chunks;
	UINT8 j, k, px, py, zx, zy, zoomx, zoomy;
	static const int primasks[2] = { 0xff00, 0xfffc };	/* Sprites are over bottom layer or under top layer */

	/* Most of spriteram is not used by the 68000: rest is scratch space for the h/w perhaps ? */
	for (offs = 0; offs < (0x2c0 / 2); offs += 4)
	{
		data = spriteram[offs + 2];

		tilenum = spriteram[offs + 3] & 0xff;
		color = (spriteram[offs + 3] & 0xff00) >> 8;
		flipx = (data & 0x4000) >> 14;
		flipy = (spriteram[offs + 1] & 0x8000) >> 15;
		x = data & 0x1ff;
		y = spriteram[offs] & 0x1ff;
		zoomx = (spriteram[offs + 1]& 0x7f);
		zoomy = (spriteram[offs] & 0xfe00) >> 9;
		priority = (data & 0x8000) >> 15;
//      unknown = (data & 0x2000) >> 13;

		if (y == 0x180)
			continue;	/* dead sprite */

		map_offset = tilenum << 7;

		zoomx += 1;
		zoomy += 1;

		y += 3 + (128-zoomy);

		/* treat coords as signed */
		if (x > 0x140) x -= 0x200;
		if (y > 0x140) y -= 0x200;

		bad_chunks = 0;

		for (sprite_chunk = 0; sprite_chunk < 128; sprite_chunk++)
		{
			k = sprite_chunk % 8;   /* 8 sprite chunks per row */
			j = sprite_chunk / 8;   /* 16 rows */

			/* pick tiles back to front for x and y flips */
			px = (flipx) ?  (7 - k) : (k);
			py = (flipy) ? (15 - j) : (j);

			code = spritemap[map_offset + (py << 3) + px];

			if (code & 0x8000)
			{
				bad_chunks += 1;
				continue;
			}

			curx = x + ((k * zoomx) / 8);
			cury = y + ((j * zoomy) / 16);

			zx = x + (((k + 1) * zoomx) / 8) - curx;
			zy = y + (((j + 1) * zoomy) / 16) - cury;

			pdrawgfxzoom_transpen(bitmap,cliprect,machine.gfx[0],
					code,
					color,
					flipx,flipy,
					curx,cury,
					zx<<12,zy<<13,
					machine.priority_bitmap,primasks[priority],0);
		}

		if (bad_chunks)
			logerror("Sprite number %04x had %02x invalid chunks\n",tilenum,bad_chunks);
	}
}


/***************************************************************************/

UINT32 topspeed_state::screen_update_topspeed(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 layer[4];

#ifdef MAME_DEBUG
	if (screen.machine().input().code_pressed_once (KEYCODE_V))
	{
		m_dislayer[0] ^= 1;
		popmessage("bg: %01x", m_dislayer[0]);
	}

	if (screen.machine().input().code_pressed_once (KEYCODE_B))
	{
		m_dislayer[1] ^= 1;
		popmessage("fg: %01x", m_dislayer[1]);
	}

	if (screen.machine().input().code_pressed_once (KEYCODE_N))
	{
		m_dislayer[2] ^= 1;
		popmessage("bg2: %01x", m_dislayer[2]);
	}

	if (screen.machine().input().code_pressed_once (KEYCODE_M))
	{
		m_dislayer[3] ^= 1;
		popmessage("fg2: %01x", m_dislayer[3]);
	}

	if (screen.machine().input().code_pressed_once (KEYCODE_C))
	{
		m_dislayer[4] ^= 1;
		popmessage("sprites: %01x", m_dislayer[4]);
	}
#endif

	pc080sn_tilemap_update(m_pc080sn_1);
	pc080sn_tilemap_update(m_pc080sn_2);

	/* Tilemap layer priority seems hardwired (the order is odd, too) */
	layer[0] = 1;
	layer[1] = 0;
	layer[2] = 1;
	layer[3] = 0;

	screen.machine().priority_bitmap.fill(0, cliprect);
	bitmap.fill(0, cliprect);

#ifdef MAME_DEBUG
	if (m_dislayer[3] == 0)
#endif
	pc080sn_tilemap_draw(m_pc080sn_2, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);

#ifdef MAME_DEBUG
	if (m_dislayer[2] == 0)
#endif
	pc080sn_tilemap_draw_special(m_pc080sn_2, bitmap, cliprect, layer[1], 0, 2, m_raster_ctrl);

#ifdef MAME_DEBUG
	if (m_dislayer[1] == 0)
#endif
	pc080sn_tilemap_draw_special(m_pc080sn_1, bitmap, cliprect, layer[2], 0, 4, m_raster_ctrl + 0x100);

#ifdef MAME_DEBUG
	if (m_dislayer[0] == 0)
#endif
	pc080sn_tilemap_draw(m_pc080sn_1, bitmap, cliprect, layer[3], 0, 8);

#ifdef MAME_DEBUG
	if (m_dislayer[4] == 0)
#endif

	draw_sprites(screen.machine(), bitmap,cliprect);
	return 0;
}
