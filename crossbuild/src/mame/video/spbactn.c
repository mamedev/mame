/* video/spbactn.c - see drivers/spbactn.c for more info */
/* rather similar to galspnbl.c */

#include "driver.h"

extern UINT16 *spbactn_bgvideoram, *spbactn_fgvideoram, *spbactn_spvideoram;
static mame_bitmap *tile_bitmap_bg, *tile_bitmap_fg;

/* mix & blend the paletted 16-bit tile and sprite bitmaps into an RGB 32-bit bitmap */
static void blendbitmaps(running_machine *machine,
		mame_bitmap *dest,mame_bitmap *src1,mame_bitmap *src2,
		int sx,int sy,const rectangle *clip)
{
	int ox;
	int oy;
	int ex;
	int ey;

	/* check bounds */
	ox = sx;
	oy = sy;

	ex = sx + src1->width - 1;
	if (sx < 0) sx = 0;
	if (sx < clip->min_x) sx = clip->min_x;
	if (ex >= dest->width) ex = dest->width - 1;
	if (ex > clip->max_x) ex = clip->max_x;
	if (sx > ex) return;

	ey = sy + src1->height - 1;
	if (sy < 0) sy = 0;
	if (sy < clip->min_y) sy = clip->min_y;
	if (ey >= dest->height) ey = dest->height - 1;
	if (ey > clip->max_y) ey = clip->max_y;
	if (sy > ey) return;

	{
		const pen_t *paldata = machine->pens;
		UINT32 *end;

		UINT16 *sd1 = src1->base;												/* source data   */
		UINT16 *sd2 = src2->base;

		int sw = ex-sx+1;														/* source width  */
		int sh = ey-sy+1;														/* source height */
		int sm = src1->rowpixels;												/* source modulo */

		UINT32 *dd = BITMAP_ADDR32(dest, sy, sx);								/* dest data     */
		int dm = dest->rowpixels;												/* dest modulo   */

		sd1 += (sx-ox);
		sd1 += sm * (sy-oy);
		sd2 += (sx-ox);
		sd2 += sm * (sy-oy);

		sm -= sw;
		dm -= sw;

		while (sh)
		{

#define BLENDPIXEL(x)	if (sd2[x]) {												\
							if (sd2[x] & 0x1000) {									\
								dd[x] = paldata[sd1[x] & 0x07ff] + paldata[sd2[x]];	\
							} else {												\
								dd[x] = paldata[sd2[x]];							\
							}														\
						} else {													\
							dd[x] = paldata[sd1[x]];								\
						}

			end = dd + sw;
			while (dd <= end - 8)
			{
				BLENDPIXEL(0);
				BLENDPIXEL(1);
				BLENDPIXEL(2);
				BLENDPIXEL(3);
				BLENDPIXEL(4);
				BLENDPIXEL(5);
				BLENDPIXEL(6);
				BLENDPIXEL(7);
				dd += 8;
				sd1 += 8;
				sd2 += 8;
			}
			while (dd < end)
			{
				BLENDPIXEL(0);
				dd++;
				sd1++;
				sd2++;
			}
			sd1 += sm;
			sd2 += sm;
			dd += dm;
			sh--;

#undef BLENDPIXEL

		}
	}
}

/* from gals pinball (which was in turn from ninja gaiden) */
static int draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int priority)
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

	int count = 0;

	int offs;

	for (offs = (0x1000 - 16) / 2; offs >= 0; offs -= 8)
	{
		int sx, sy, code, color, size, attr, flipx, flipy;
		int col, row;

		attr = spbactn_spvideoram[offs];
		if ((attr & 0x0004) &&
			((attr & 0x0030) >> 4) == priority)
		{
			flipx = attr & 0x0001;
			flipy = attr & 0x0002;

			code = spbactn_spvideoram[offs + 1];

			color = spbactn_spvideoram[offs + 2];
			size = 1 << (color & 0x0003);				/* 1,2,4,8 */
			color = (color & 0x00f0) >> 4;

			sx = spbactn_spvideoram[offs + 4];
			sy = spbactn_spvideoram[offs + 3];

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

					drawgfx(bitmap, machine->gfx[2],
						code + layout[row][col],
						color,
						flipx, flipy,
						x, y,
						cliprect, TRANSPARENCY_PEN, 0);
				}
			}

			count++;
		}
	}

	return count;
}


VIDEO_START( spbactn )
{
	/* allocate bitmaps */
	tile_bitmap_bg = auto_bitmap_alloc(machine->screen[0].width, machine->screen[0].height, BITMAP_FORMAT_INDEXED16);
	tile_bitmap_fg = auto_bitmap_alloc(machine->screen[0].width, machine->screen[0].height, BITMAP_FORMAT_INDEXED16);
}

VIDEO_UPDATE( spbactn )
{
	int offs, sx, sy;

	fillbitmap(tile_bitmap_fg,      0, cliprect);

	/* draw table bg gfx */
	for (sx = sy = offs = 0; offs < 0x4000 / 2; offs++)
	{
		int attr, code, colour;

		code = spbactn_bgvideoram[offs + 0x4000 / 2];
		attr = spbactn_bgvideoram[offs + 0x0000 / 2];

		colour = ((attr & 0x00f0) >> 4) | 0x80;

		drawgfx(tile_bitmap_bg, machine->gfx[1],
					code,
					colour,
					0, 0,
					16 * sx, 8 * sy,
					cliprect, TRANSPARENCY_NONE, 0);

		sx++;
		if (sx > 63)
		{
			sy++;
			sx = 0;
		}
	}

	if (draw_sprites(machine, tile_bitmap_bg, cliprect, 0))
	{
		/* kludge: draw table bg gfx again if priority 0 sprites are enabled */
		for (sx = sy = offs = 0; offs < 0x4000 / 2; offs++)
		{
			int attr, code, colour;

			code = spbactn_bgvideoram[offs + 0x4000 / 2];
			attr = spbactn_bgvideoram[offs + 0x0000 / 2];

			colour = ((attr & 0x00f0) >> 4) | 0x80;

			drawgfx(tile_bitmap_bg, machine->gfx[1],
					code,
					colour,
					0, 0,
					16 * sx, 8 * sy,
					cliprect, TRANSPARENCY_PEN, 0);

			sx++;
			if (sx > 63)
			{
				sy++;
				sx = 0;
			}
		}
	}

	draw_sprites(machine, tile_bitmap_bg, cliprect, 1);

	/* draw table fg gfx */
	for (sx = sy = offs = 0; offs < 0x4000 / 2; offs++)
	{
		int attr, code, colour;

		code = spbactn_fgvideoram[offs + 0x4000 / 2];
		attr = spbactn_fgvideoram[offs + 0x0000 / 2];

		colour = ((attr & 0x00f0) >> 4);

		/* blending */
		if (attr & 0x0008)
			colour += 0x00f0;
		else
			colour |= 0x0080;

		drawgfx(tile_bitmap_fg, machine->gfx[0],
					code,
					colour,
					0, 0,
					16 * sx, 8 * sy,
					cliprect,TRANSPARENCY_PEN, 0);

		sx++;
		if (sx > 63)
		{
			sy++;
			sx = 0;
		}
	}
	draw_sprites(machine, tile_bitmap_fg, cliprect, 2);
	draw_sprites(machine, tile_bitmap_fg, cliprect, 3);

	/* mix & blend the tilemaps and sprites into a 32-bit bitmap */
	blendbitmaps(machine, bitmap, tile_bitmap_bg, tile_bitmap_fg, 0, 0, cliprect);
	return 0;
}
