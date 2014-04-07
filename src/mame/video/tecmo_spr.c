/* Various Tecmo Sprite implementations
 - for unifying and converting to a device

 - check wc90.c, tecmo.c, tbowl.c others? - they seem more significantly different but are they close to each other
   (but at the same time use the same 'layout' table as this implementation)

 - is there a single chip responsible for these, or is it just a family of closely connected implementations?
   (because we seem to need some per-game code right now)

*/


#include "emu.h"
#include "tecmo_spr.h"



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




/* sprite format (gaiden):
 *
 *  word        bit                 usage
 * --------+-fedcba9876543210-+----------------
 *    0    | ---------------x | flip x
 *         | --------------x- | flip y
 *         | -------------x-- | enable
 *         | ----------x----- | blend
 *         | --------xx------ | sprite-tile priority
 *    1    | xxxxxxxxxxxxxxxx | number
 *    2    | --------xxxx---- | palette
 *         | --------------xx | size: 8x8, 16x16, 32x32, 64x64
 *    3    | xxxxxxxxxxxxxxxx | y position
 *    4    | xxxxxxxxxxxxxxxx | x position
 *    5,6,7|                  | unused
 */

/* sprite format (galspnbl):
 *
 *  word        bit                 usage
 * --------+-fedcba9876543210-+----------------
 *    0    | ---------------x | flip x
 *         | --------------x- | flip y
 *         | -------------x-- | enable
 *         | ----------xx---- | priority?
 *         | ---------x------ | flicker?
 *    1    | xxxxxxxxxxxxxxxx | code
 *    2    | --------xxxx---- | color
 *         | --------------xx | size: 8x8, 16x16, 32x32, 64x64
 *    3    | xxxxxxxxxxxxxxxx | y position
 *    4    | xxxxxxxxxxxxxxxx | x position
 *    5,6,7|                  | unused
 */


/* from gals pinball (which was in turn from ninja gaiden) */
int spbactn_draw_sprites(screen_device &screen, gfxdecode_device *gfxdecode, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority, bool alt_sprites, UINT16* spriteram)
{
	int count = 0;
	int offs;

	for (offs = (0x1000 - 16) / 2; offs >= 0; offs -= 8)
	{
		int sx, sy, code, color, size, attr, flipx, flipy;
		int col, row;

		attr = spriteram[offs];

		int pri = (spriteram[offs] & 0x0030);
//      int pri = (spriteram[offs+2] & 0x0030);


		if ((attr & 0x0004) &&
			((pri & 0x0030) >> 4) == priority)
		{
			flipx = attr & 0x0001;
			flipy = attr & 0x0002;

			code = spriteram[offs + 1];

			if (alt_sprites)
			{
				color = spriteram[offs + 0];
			}
			else
			{
				color = spriteram[offs + 2];
			}

			size = 1 << (spriteram[offs + 2] & 0x0003);               /* 1,2,4,8 */
			color = (color & 0x00f0) >> 4;

			sx = spriteram[offs + 4];
			sy = spriteram[offs + 3];

			attr &= ~0x0040;                            /* !!! */

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

						gfxdecode->gfx(2)->transpen_raw(bitmap,cliprect,
						code + layout[row][col],
						gfxdecode->gfx(2)->colorbase() + color * gfxdecode->gfx(2)->granularity(),
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


// comad bootleg of spbactn
void galspnbl_draw_sprites( screen_device &screen, gfxdecode_device *gfxdecode, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority, UINT16* spriteram, int spriteram_bytes )
{
	int offs;


	for (offs = (spriteram_bytes - 16) / 2; offs >= 0; offs -= 8)
	{
		int sx, sy, code, color, size, attr, flipx, flipy;
		int col, row;

		attr = spriteram[offs];
		if ((attr & 0x0004) && ((attr & 0x0040) == 0 || (screen.frame_number() & 1))
//              && ((attr & 0x0030) >> 4) == priority)
				&& ((attr & 0x0020) >> 5) == priority)
		{
			code = spriteram[offs + 1];
			color = spriteram[offs + 2];
			size = 1 << (color & 0x0003); // 1,2,4,8
			color = (color & 0x00f0) >> 4;
//          sx = spriteram[offs + 4] + screenscroll;
			sx = spriteram[offs + 4];
			sy = spriteram[offs + 3];
			flipx = attr & 0x0001;
			flipy = attr & 0x0002;

			for (row = 0; row < size; row++)
			{
				for (col = 0; col < size; col++)
				{
					int x = sx + 8 * (flipx ? (size - 1 - col) : col);
					int y = sy + 8 * (flipy ? (size - 1 - row) : row);
					gfxdecode->gfx(1)->transpen(bitmap,cliprect,
						code + layout[row][col],
						color,
						flipx,flipy,
						x,y,0);
				}
			}
		}
	}
}

void tecmo16_draw_sprites(screen_device &screen, gfxdecode_device *gfxdecode, bitmap_ind16 &bitmap_bg, bitmap_ind16 &bitmap_fg, bitmap_ind16 &bitmap_sp, const rectangle &cliprect, UINT16* spriteram, UINT16 spriteram16_bytes, int game_is_riot, int flipscreen )
{
	UINT16 *spriteram16 = spriteram;
	int offs;

	bitmap_ind16 &bitmap = bitmap_bg;

	for (offs = spriteram16_bytes/2 - 8;offs >= 0;offs -= 8)
	{
		if (spriteram16[offs] & 0x04)   /* enable */
		{
			int code,color,sizex,sizey,flipx,flipy,xpos,ypos;
			int x,y,priority,priority_mask;

			code = spriteram16[offs+1];
			color = (spriteram16[offs+2] & 0xf0) >> 4;
			sizex = 1 << ((spriteram16[offs+2] & 0x03) >> 0);

			if(game_is_riot)
				sizey = sizex;
			else
				sizey = 1 << ((spriteram16[offs+2] & 0x0c) >> 2);

			if (sizex >= 2) code &= ~0x01;
			if (sizey >= 2) code &= ~0x02;
			if (sizex >= 4) code &= ~0x04;
			if (sizey >= 4) code &= ~0x08;
			if (sizex >= 8) code &= ~0x10;
			if (sizey >= 8) code &= ~0x20;
			flipx = spriteram16[offs] & 0x01;
			flipy = spriteram16[offs] & 0x02;
			xpos = spriteram16[offs+4];
			if (xpos >= 0x8000) xpos -= 0x10000;
			ypos = spriteram16[offs+3];
			if (ypos >= 0x8000) ypos -= 0x10000;
			priority = (spriteram16[offs] & 0xc0) >> 6;

			/* bg: 1; fg:2; text: 4 */
			switch (priority)
			{
				default:
				case 0x0: priority_mask = 0; break;
				case 0x1: priority_mask = 0xf0; break; /* obscured by text layer */
				case 0x2: priority_mask = 0xf0|0xcc; break; /* obscured by foreground */
				case 0x3: priority_mask = 0xf0|0xcc|0xaa; break; /* obscured by bg and fg */
			}

			if (flipscreen)
			{
				flipx = !flipx;
				flipy = !flipy;
			}

			/* blending */
			if (spriteram16[offs] & 0x20)
			{
				color |= 0x80;

				for (y = 0;y < sizey;y++)
				{
					for (x = 0;x < sizex;x++)
					{
						int sx,sy;

						if (!flipscreen)
						{
							sx = xpos + 8*(flipx?(sizex-1-x):x);
							sy = ypos + 8*(flipy?(sizey-1-y):y);
						} else {
							sx = 256 - (xpos + 8*(!flipx?(sizex-1-x):x) + 8);
							sy = 256 - (ypos + 8*(!flipy?(sizey-1-y):y) + 8);
						}
						gfxdecode->gfx(2)->prio_transpen_raw(bitmap,cliprect,
								code + layout[y][x],
								gfxdecode->gfx(2)->colorbase() + color * gfxdecode->gfx(2)->granularity(),
								flipx,flipy,
								sx,sy,
								screen.priority(), priority_mask,0);

						/* wrap around x */
						gfxdecode->gfx(2)->prio_transpen_raw(bitmap,cliprect,
								code + layout[y][x],
								gfxdecode->gfx(2)->colorbase() + color * gfxdecode->gfx(2)->granularity(),
								flipx,flipy,
								sx-512,sy,
								screen.priority(), priority_mask,0);

						/* wrap around x */
						gfxdecode->gfx(2)->prio_transpen_raw(bitmap,cliprect,
								code + layout[y][x],
								gfxdecode->gfx(2)->colorbase() + color * gfxdecode->gfx(2)->granularity(),
								flipx,flipy,
								sx+512,sy,
								screen.priority(), priority_mask,0);
					}
				}
			}
			else
			{
				bitmap_ind16 &bitmap = (priority >= 2) ? bitmap_bg : bitmap_fg;

				for (y = 0;y < sizey;y++)
				{
					for (x = 0;x < sizex;x++)
					{
						int sx,sy;

						if (!flipscreen)
						{
							sx = xpos + 8*(flipx?(sizex-1-x):x);
							sy = ypos + 8*(flipy?(sizey-1-y):y);
						} else {
							sx = 256 - (xpos + 8*(!flipx?(sizex-1-x):x) + 8);
							sy = 256 - (ypos + 8*(!flipy?(sizey-1-y):y) + 8);
						}
						gfxdecode->gfx(2)->prio_transpen_raw(bitmap,cliprect,
								code + layout[y][x],
								gfxdecode->gfx(2)->colorbase() + color * gfxdecode->gfx(2)->granularity(),
								flipx,flipy,
								sx,sy,
								screen.priority(), priority_mask,0);

						/* wrap around x */
						gfxdecode->gfx(2)->prio_transpen_raw(bitmap,cliprect,
								code + layout[y][x],
								gfxdecode->gfx(2)->colorbase() + color * gfxdecode->gfx(2)->granularity(),
								flipx,flipy,
								sx-512,sy,
								screen.priority(), priority_mask,0);

						/* wrap around x */
						gfxdecode->gfx(2)->prio_transpen_raw(bitmap,cliprect,
								code + layout[y][x],
								gfxdecode->gfx(2)->colorbase() + color * gfxdecode->gfx(2)->granularity(),
								flipx,flipy,
								sx+512,sy,
								screen.priority(), priority_mask,0);
					}
				}
			}
		}
	}
}

#define NUM_SPRITES 256

void gaiden_draw_sprites( screen_device &screen, gfxdecode_device *gfxdecode, bitmap_ind16 &bitmap_bg, bitmap_ind16 &bitmap_fg, bitmap_ind16 &bitmap_sp, const rectangle &cliprect, UINT16* spriteram, int sprite_sizey, int spr_offset_y, int flip_screen )
{
	gfx_element *gfx = gfxdecode->gfx(3);
	const UINT16 *source = (NUM_SPRITES - 1) * 8 + spriteram;
	int count = NUM_SPRITES;

	/* draw all sprites from front to back */
	while (count--)
	{
		UINT32 attributes = source[0];
		UINT32 priority_mask;
		int col,row;

		if (attributes & 0x04)
		{
			UINT32 priority = (attributes >> 6) & 3;
			UINT32 flipx = (attributes & 1);
			UINT32 flipy = (attributes & 2);

			UINT32 color = source[2];
			UINT32 sizex = 1 << ((color >> 0) & 3);                     /* 1,2,4,8 */
			UINT32 sizey = 1 << ((color >> sprite_sizey) & 3); /* 1,2,4,8 */

			/* raiga needs something like this */
			UINT32 number = (source[1] & (sizex > 2 ? 0x7ff8 : 0x7ffc));

			int ypos = (source[3] + spr_offset_y) & 0x01ff;
			int xpos = source[4] & 0x01ff;

			color = (color >> 4) & 0x0f;

			/* wraparound */
			if (xpos >= 256)
				xpos -= 512;
			if (ypos >= 256)
				ypos -= 512;

			if (flip_screen)
			{
				flipx = !flipx;
				flipy = !flipy;

				xpos = 256 - (8 * sizex) - xpos;
				ypos = 256 - (8 * sizey) - ypos;

				if (xpos <= -256)
					xpos += 512;
				if (ypos <= -256)
					ypos += 512;
			}

			/* bg: 1; fg:2; text: 4 */
			switch( priority )
			{
				default:
				case 0x0: priority_mask = 0;                    break;
				case 0x1: priority_mask = 0xf0;                 break;  /* obscured by text layer */
				case 0x2: priority_mask = 0xf0 | 0xcc;          break;  /* obscured by foreground */
				case 0x3: priority_mask = 0xf0 | 0xcc | 0xaa;   break;  /* obscured by bg and fg  */
			}


			/* blending */
			if (attributes & 0x20)
			{
				color |= 0x80;

				for (row = 0; row < sizey; row++)
				{
					for (col = 0; col < sizex; col++)
					{
						int sx = xpos + 8 * (flipx ? (sizex - 1 - col) : col);
						int sy = ypos + 8 * (flipy ? (sizey - 1 - row) : row);

						gfx->prio_transpen_raw(bitmap_sp,cliprect,
							number + layout[row][col],
							gfx->colorbase() + color * gfx->granularity(),
							flipx, flipy,
							sx, sy,
							screen.priority(), priority_mask, 0);
					}
				}
			}
			else
			{
				bitmap_ind16 &bitmap = (priority >= 2) ? bitmap_bg : bitmap_fg;

				for (row = 0; row < sizey; row++)
				{
					for (col = 0; col < sizex; col++)
					{
						int sx = xpos + 8 * (flipx ? (sizex - 1 - col) : col);
						int sy = ypos + 8 * (flipy ? (sizey - 1 - row) : row);

						gfx->prio_transpen_raw(bitmap,cliprect,
							number + layout[row][col],
							gfx->colorbase() + color * gfx->granularity(),
							flipx, flipy,
							sx, sy,
							screen.priority(), priority_mask, 0);
					}
				}
			}
		}
		source -= 8;
	}
}


void raiga_draw_sprites( screen_device &screen, gfxdecode_device *gfxdecode, bitmap_ind16 &bitmap_bg, bitmap_ind16 &bitmap_fg, bitmap_ind16 &bitmap_sp, const rectangle &cliprect, UINT16* spriteram, int sprite_sizey, int spr_offset_y, int flip_screen  )
{
	gfx_element *gfx = gfxdecode->gfx(3);
	const UINT16 *source = (NUM_SPRITES - 1) * 8 + spriteram;
	int count = NUM_SPRITES;

	/* draw all sprites from front to back */
	while (count--)
	{
		UINT32 attributes = source[0];
		UINT32 priority_mask;
		int col,row;

		if (attributes & 0x04)
		{
			UINT32 priority = (attributes >> 6) & 3;
			UINT32 flipx = (attributes & 1);
			UINT32 flipy = (attributes & 2);

			UINT32 color = source[2];
			UINT32 sizex = 1 << ((color >> 0) & 3);                     /* 1,2,4,8 */
			UINT32 sizey = 1 << ((color >> sprite_sizey) & 3); /* 1,2,4,8 */

			/* raiga needs something like this */
			UINT32 number = (source[1] & (sizex > 2 ? 0x7ff8 : 0x7ffc));

			int ypos = (source[3] + spr_offset_y) & 0x01ff;
			int xpos = source[4] & 0x01ff;

			color = (color >> 4) & 0x0f;

			/* wraparound */
			if (xpos >= 256)
				xpos -= 512;
			if (ypos >= 256)
				ypos -= 512;

			if (flip_screen)
			{
				flipx = !flipx;
				flipy = !flipy;

				xpos = 256 - (8 * sizex) - xpos;
				ypos = 256 - (8 * sizey) - ypos;

				if (xpos <= -256)
					xpos += 512;
				if (ypos <= -256)
					ypos += 512;
			}

			/* bg: 1; fg:2; text: 4 */
			switch( priority )
			{
				default:
				case 0x0: priority_mask = 0;                    break;
				case 0x1: priority_mask = 0xf0;                 break;  /* obscured by text layer */
				case 0x2: priority_mask = 0xf0 | 0xcc;          break;  /* obscured by foreground */
				case 0x3: priority_mask = 0xf0 | 0xcc | 0xaa;   break;  /* obscured by bg and fg  */
			}

			/* blending */
			if (attributes & 0x20)
			{
				color |= 0x80;

				for (row = 0; row < sizey; row++)
				{
					for (col = 0; col < sizex; col++)
					{
						int sx = xpos + 8 * (flipx ? (sizex - 1 - col) : col);
						int sy = ypos + 8 * (flipy ? (sizey - 1 - row) : row);

						gfx->prio_transpen_raw(bitmap_sp,cliprect,
							number + layout[row][col],
							gfx->colorbase() + color * gfx->granularity(),
							flipx, flipy,
							sx, sy,
							screen.priority(), priority_mask, 0);
					}
				}
			}
			else
			{
				bitmap_ind16 &bitmap = (priority >= 2) ? bitmap_bg : bitmap_fg;

				for (row = 0; row < sizey; row++)
				{
					for (col = 0; col < sizex; col++)
					{
						int sx = xpos + 8 * (flipx ? (sizex - 1 - col) : col);
						int sy = ypos + 8 * (flipy ? (sizey - 1 - row) : row);

						gfx->prio_transpen_raw(bitmap,cliprect,
							number + layout[row][col],
							gfx->colorbase() + color * gfx->granularity(),
							flipx, flipy,
							sx, sy,
							screen.priority(), priority_mask, 0);
					}
				}
			}
		}

		source -= 8;
	}
}
