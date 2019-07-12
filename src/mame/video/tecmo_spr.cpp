// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Various Tecmo Sprite implementations

 - the various sprite implementations here are slightly different but can clearly be refactored to use
   a common base class for the chained drawing even if the position of the attributes etc. varies between
   PCB / chip.

 - what chips are involved in implementing these schemes? ttl logic on early ones? customs on later?

*/

#include "emu.h"
#include "tecmo_spr.h"
#include "screen.h"


DEFINE_DEVICE_TYPE(TECMO_SPRITE, tecmo_spr_device, "tecmo_spr", "Tecmo Chained Sprites")

tecmo_spr_device::tecmo_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TECMO_SPRITE, tag, owner, clock)
	, m_bootleg(false)
	, m_yoffset(0)
{
}


void tecmo_spr_device::device_start()
{
}

void tecmo_spr_device::device_reset()
{
}


static const uint8_t layout[8][8] =
{
	{ 0, 1, 4, 5, 16, 17, 20, 21 },
	{ 2, 3, 6, 7, 18, 19, 22, 23 },
	{ 8, 9, 12, 13, 24, 25, 28, 29 },
	{ 10, 11, 14, 15, 26, 27, 30, 31 },
	{ 32, 33, 36, 37, 48, 49, 52, 53 },
	{ 34, 35, 38, 39, 50, 51, 54, 55 },
	{ 40, 41, 44, 45, 56, 57, 60, 61 },
	{ 42, 43, 46, 47, 58, 59, 62, 63 }
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

#define NUM_SPRITES 256

void tecmo_spr_device::gaiden_draw_sprites(screen_device &screen, gfx_element *gfx, const rectangle &cliprect, uint16_t* spriteram, int sprite_sizey, int spr_offset_y, int flip_screen, bitmap_ind16 &sprite_bitmap)
{
	uint16_t *source = spriteram;
	int sourceinc = 8;

	int count = NUM_SPRITES;
	int screenwidth = screen.width();

	int attributes_word = 0;
	int tilenumber_word = 1;
	int colour_word = 2;
	int yposition_word = 3;
	int xposition_word = 4;

	int xmask;

	if (screenwidth == 512)
		xmask = 512;
	else
		xmask = 256;

	/* draw all sprites from front to back */
	while (count--)
	{
		uint32_t attributes = source[attributes_word];

		int enabled = source[attributes_word] & 0x04;

		if (enabled)
		{
			if (m_bootleg)
			{
				// I don't think the galspinbl / hotpinbl bootlegs have blending, instead they use this bit to flicker sprites on/off each frame, so handle it here (we can't handle it in the mixing)
				// alternatively these sprites could just be disabled like the tiles marked with the 'mix' bit appear to be (they're only used for ball / flipper trails afaik)
				if (source[attributes_word] & 0x0040)
				{
					int frame = screen.frame_number() & 1;
					if (frame==1)
						enabled = 0;
				}

			}
		}

		if (enabled)
		{
			uint32_t flipx = (attributes & 1);
			uint32_t flipy = (attributes & 2);

			uint32_t color = source[colour_word];
			uint32_t sizex = 1 << ((color >> 0) & 3);                     /* 1,2,4,8 */
			uint32_t sizey = 1 << ((color >> sprite_sizey) & 3); /* 1,2,4,8 */

			/* raiga & fstarfrc need something like this */
			uint32_t number = (source[tilenumber_word]);

			if (sizex >= 2) number &= ~0x01;
			if (sizey >= 2) number &= ~0x02;
			if (sizex >= 4) number &= ~0x04;
			if (sizey >= 4) number &= ~0x08;
			if (sizex >= 8) number &= ~0x10;
			if (sizey >= 8) number &= ~0x20;

			int ypos = (source[yposition_word] + spr_offset_y) & 0x01ff;
			int xpos = source[xposition_word] & ((xmask*2)-1);

			color = (color >> 4) & 0x0f;

			/* wraparound */
			if (xpos >= xmask)
				xpos -= (xmask*2);
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

			// this contains the blend bit and the priority bits, the spbactn proto uses 0x0300 for priority, spbactn uses 0x0030, others use 0x00c0
			color |= (source[attributes_word] & 0x03f0);
			bitmap_ind16* bitmap = &sprite_bitmap;

			for (int row = 0; row < sizey; row++)
			{
				for (int col = 0; col < sizex; col++)
				{
					int sx = xpos + 8 * (flipx ? (sizex - 1 - col) : col);
					int sy = ypos + 8 * (flipy ? (sizey - 1 - row) : row);

					gfx->transpen_raw(*bitmap, cliprect,
						number + layout[row][col],
						gfx->colorbase() + color * gfx->granularity(),
						flipx, flipy,
						sx, sy,
						0);

				}
			}
		}
		source += sourceinc;
	}
}


/* NOT identical to the version above */

/* sprite format (tecmo.cpp):
 *
 *  byte     bit        usage
 * --------+-76543210-+----------------
         0 | xxxxx--- | bank / upper tile bits
           | -----x-- | enable
           | ------x- | flip y
           | -------x | flip x
         1 | xxxxxxxx | tile number (low bits)
         2 | ------xx | size
         3 | xx-------| priority
           | --x----- | upper y co-ord
           | ---x---- | upper x co-ord
           | ----xxxx | colour
         4 | xxxxxxxx | ypos
         5 | xxxxxxxx | xpos
         6 | -------- |
         7 | -------- |

*/



void tecmo_spr_device::draw_sprites_8bit(screen_device &screen, bitmap_ind16 &bitmap, gfx_element *gfx, const rectangle &cliprect, uint8_t* spriteram, int size, int video_type, int flip_screen)
{
	for (int offs = size-8; offs >= 0; offs -= 8)
	{
		int flags = spriteram[offs+3];
		int priority = flags>>6;
		int bank = spriteram[offs+0];
		if (bank & 4)
		{ /* visible */
			int which = spriteram[offs+1];
			int code,priority_mask;
			int size = spriteram[offs + 2] & 3;

			if (video_type != 0)   /* gemini, silkworm */
				code = which + ((bank & 0xf8) << 5);
			else                        /* rygar */
				code = which + ((bank & 0xf0) << 4);

			code &= ~((1 << (size*2)) - 1);
			size = 1 << size;

			int xpos = spriteram[offs + 5] - ((flags & 0x10) << 4);
			int ypos = spriteram[offs + 4] - ((flags & 0x20) << 3);
			int flipx = bank & 1;
			int flipy = bank & 2;

			if (flip_screen)
			{
				xpos = 256 - (8 * size) - xpos;
				ypos = 256 - (8 * size) - ypos;
				flipx = !flipx;
				flipy = !flipy;
			}

			/* bg: 1; fg:2; text: 4 */
			switch (priority)
			{
				default:
				case 0x0: priority_mask = 0; break;
				case 0x1: priority_mask = 0xf0; break; /* obscured by text layer */
				case 0x2: priority_mask = 0xf0|0xcc; break; /* obscured by foreground */
				case 0x3: priority_mask = 0xf0|0xcc|0xaa; break; /* obscured by bg and fg */
			}

			for (int y = 0; y < size; y++)
			{
				for (int x = 0; x < size; x++)
				{
					int sx = xpos + 8*(flipx?(size-1-x):x);
					int sy = ypos + 8*(flipy?(size-1-y):y);
					gfx->prio_transpen(bitmap,cliprect,
							code + layout[y][x],
							flags & 0xf,
							flipx,flipy,
							sx,sy,
							screen.priority(),
							priority_mask,0);
				}
			}
		}
	}
}


/* sprite format (wc90.cpp):  - similar to the 16-bit one
 *
 *  byte     bit        usage
 * --------+-76543210-+----------------



*/


void tecmo_spr_device::draw_wc90_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx, uint8_t* spriteram, int size, int priority)
{
	/* draw all visible sprites of specified priority */
	for (int offs = 0; offs < size; offs += 16)
	{
		int bank = spriteram[offs+0];

		if ((bank >> 4) == priority)
		{
			if (bank & 4)
			{ /* visible */
				int code = ( spriteram[offs+2] ) + ( spriteram[offs+3] << 8 );

				int xpos = spriteram[offs + 8] + ( (spriteram[offs + 9] & 3 ) << 8 );
				int ypos = spriteram[offs + 6] + m_yoffset;
				ypos &= 0xff; // sprite wrap right on edge (top @ ROT0) of pac90
				ypos = ypos - ((spriteram[offs + 7] & 1) << 8); // sprite wrap on top of wc90

				if (xpos >= 0x0300) xpos -= 0x0400;

				int flags = spriteram[offs+4];

				int sizex = 1 << ((flags >> 0) & 3);
				int sizey = 1 << ((flags >> 2) & 3);

				int flipx = bank & 1;
				int flipy = bank & 2;

				for (int y = 0; y < sizey; y++)
				{
					for (int x = 0; x < sizex; x++)
					{
						int sx = xpos + 8*(flipx?(sizex-1-x):x);
						int sy = ypos + 8*(flipy?(sizey-1-y):y);
						gfx->transpen(bitmap,cliprect,
								code + layout[y][x],
								(flags>>4) & 0xf,
								flipx,flipy,
								sx,sy,
								0);
					}
				}
			}
		}
	}
}



void tecmo_spr_device::tbowl_draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect, gfx_element *gfx, int xscroll, uint8_t* spriteram)
{
	for (int offs = 0; offs < 0x800; offs += 8)
	{
		if (spriteram[offs+0] & 0x80)  /* enable */
		{
			int code = (spriteram[offs+2])+(spriteram[offs+1]<<8);
			int color = (spriteram[offs+3])&0x1f;
			int sizex = 1 << ((spriteram[offs+0] & 0x03) >> 0);
			int sizey = 1 << ((spriteram[offs+0] & 0x0c) >> 2);

			int flipx = (spriteram[offs+0])&0x20;
			int flipy = 0;
			int xpos = (spriteram[offs+6])+((spriteram[offs+4]&0x03)<<8);
			int ypos = (spriteram[offs+5])+((spriteram[offs+4]&0x10)<<4);

			/* bg: 1; fg:2; text: 4 */

			for (int y = 0; y < sizey; y++)
			{
				for (int x = 0; x < sizex; x++)
				{
					int sx = xpos + 8*(flipx?(sizex-1-x):x);
					int sy = ypos + 8*(flipy?(sizey-1-y):y);

					sx -= xscroll;

					gfx->transpen(bitmap,cliprect,
							code + layout[y][x],
							color,
							flipx,flipy,
							sx,sy,0 );

					/* wraparound */
					gfx->transpen(bitmap,cliprect,
							code + layout[y][x],
							color,
							flipx,flipy,
							sx,sy-0x200,0 );

					/* wraparound */
					gfx->transpen(bitmap,cliprect,
							code + layout[y][x],
							color,
							flipx,flipy,
							sx-0x400,sy,0 );

					/* wraparound */
					gfx->transpen(bitmap,cliprect,
							code + layout[y][x],
							color,
							flipx,flipy,
							sx-0x400,sy-0x200,0 );

				}
			}
		}
	}
}
