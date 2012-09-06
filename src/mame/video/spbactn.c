/* video/spbactn.c - see drivers/spbactn.c for more info */
/* rather similar to galspnbl.c */

#include "emu.h"
#include "includes/spbactn.h"


static void blendbitmaps(running_machine &machine,
		bitmap_rgb32 &dest,bitmap_ind16 &src1,bitmap_ind16 &src2,
		const rectangle &cliprect)
{
	int y,x;
	const pen_t *paldata = machine.pens;

	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT32 *dd  = &dest.pix32(y);
		UINT16 *sd1 = &src1.pix16(y);
		UINT16 *sd2 = &src2.pix16(y);

		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			if (sd2[x])
			{
				if (sd2[x] & 0x1000)
					dd[x] = paldata[sd1[x] & 0x07ff] + paldata[sd2[x]];
				else
					dd[x] = paldata[sd2[x]];
			}
			else
				dd[x] = paldata[sd1[x]];
		}
	}
}


/* from gals pinball (which was in turn from ninja gaiden) */
static int draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority)
{
	static const UINT8 layout[8][8] =
	{
		{ 0, 1, 4, 5,16,17,20,21},
		{ 2, 3, 6, 7,18,19,22,23},
		{ 8, 9,12,13,24,25,28,29},
		{10,11,14,15,26,27,30,31},
		{32,33,36,37,48,49,52,53},
		{34,35,38,39,50,51,54,55},
		{40,41,44,45,56,57,60,61},
		{42,43,46,47,58,59,62,63}
	};

	spbactn_state *state = machine.driver_data<spbactn_state>();
	int count = 0;
	int offs;

	for (offs = (0x1000 - 16) / 2; offs >= 0; offs -= 8)
	{
		int sx, sy, code, color, size, attr, flipx, flipy;
		int col, row;

		attr = state->m_spvideoram[offs];
		if ((attr & 0x0004) &&
			((attr & 0x0030) >> 4) == priority)
		{
			flipx = attr & 0x0001;
			flipy = attr & 0x0002;

			code = state->m_spvideoram[offs + 1];

			color = state->m_spvideoram[offs + 2];
			size = 1 << (color & 0x0003);				/* 1,2,4,8 */
			color = (color & 0x00f0) >> 4;

			sx = state->m_spvideoram[offs + 4];
			sy = state->m_spvideoram[offs + 3];

			attr &= ~0x0040;							/* !!! */

			if (attr & 0x0040)
				color |= 0x0180;
			else
				color |= 0x0080;


			for (row = 0; row < size; row++)
			{
				for (col = 0; col < size; col++)
				{
					int x = sx + 8 * (flipx ? (size - 1 - col) : col);
					int y = sy + 8 * (flipy ? (size - 1 - row) : row);

					drawgfx_transpen_raw(bitmap, cliprect, machine.gfx[2],
						code + layout[row][col],
						machine.gfx[2]->colorbase() + color * machine.gfx[2]->granularity(),
						flipx, flipy,
						x, y,
						0);
				}
			}

			count++;
		}
	}

	return count;
}


VIDEO_START( spbactn )
{
	spbactn_state *state = machine.driver_data<spbactn_state>();

	/* allocate bitmaps */
	machine.primary_screen->register_screen_bitmap(state->m_tile_bitmap_bg);
	machine.primary_screen->register_screen_bitmap(state->m_tile_bitmap_fg);
}

SCREEN_UPDATE_RGB32( spbactn )
{
	spbactn_state *state = screen.machine().driver_data<spbactn_state>();
	int offs, sx, sy;

	state->m_tile_bitmap_fg.fill(0, cliprect);

	/* draw table bg gfx */
	for (sx = sy = offs = 0; offs < 0x4000 / 2; offs++)
	{
		int attr, code, color;

		code = state->m_bgvideoram[offs + 0x4000 / 2];
		attr = state->m_bgvideoram[offs + 0x0000 / 2];

		color = ((attr & 0x00f0) >> 4) | 0x80;

		drawgfx_transpen_raw(state->m_tile_bitmap_bg, cliprect, screen.machine().gfx[1],
					code,
					screen.machine().gfx[1]->colorbase() + color * screen.machine().gfx[1]->granularity(),
					0, 0,
					16 * sx, 8 * sy,
					(UINT32)-1);

		sx++;
		if (sx > 63)
		{
			sy++;
			sx = 0;
		}
	}

	if (draw_sprites(screen.machine(), state->m_tile_bitmap_bg, cliprect, 0))
	{
		/* kludge: draw table bg gfx again if priority 0 sprites are enabled */
		for (sx = sy = offs = 0; offs < 0x4000 / 2; offs++)
		{
			int attr, code, color;

			code = state->m_bgvideoram[offs + 0x4000 / 2];
			attr = state->m_bgvideoram[offs + 0x0000 / 2];

			color = ((attr & 0x00f0) >> 4) | 0x80;

			drawgfx_transpen_raw(state->m_tile_bitmap_bg, cliprect, screen.machine().gfx[1],
					code,
					screen.machine().gfx[1]->colorbase() + color * screen.machine().gfx[1]->granularity(),
					0, 0,
					16 * sx, 8 * sy,
					0);

			sx++;
			if (sx > 63)
			{
				sy++;
				sx = 0;
			}
		}
	}

	draw_sprites(screen.machine(), state->m_tile_bitmap_bg, cliprect, 1);

	/* draw table fg gfx */
	for (sx = sy = offs = 0; offs < 0x4000 / 2; offs++)
	{
		int attr, code, color;

		code = state->m_fgvideoram[offs + 0x4000 / 2];
		attr = state->m_fgvideoram[offs + 0x0000 / 2];

		color = ((attr & 0x00f0) >> 4);

		/* blending */
		if (attr & 0x0008)
			color += 0x00f0;
		else
			color |= 0x0080;

		drawgfx_transpen_raw(state->m_tile_bitmap_fg, cliprect, screen.machine().gfx[0],
					code,
					screen.machine().gfx[0]->colorbase() + color * screen.machine().gfx[0]->granularity(),
					0, 0,
					16 * sx, 8 * sy,
					0);

		sx++;
		if (sx > 63)
		{
			sy++;
			sx = 0;
		}
	}

	draw_sprites(screen.machine(), state->m_tile_bitmap_fg, cliprect, 2);
	draw_sprites(screen.machine(), state->m_tile_bitmap_fg, cliprect, 3);

	/* mix & blend the tilemaps and sprites into a 32-bit bitmap */
	blendbitmaps(screen.machine(), bitmap, state->m_tile_bitmap_bg, state->m_tile_bitmap_fg, cliprect);
	return 0;
}
