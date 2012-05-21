#include "emu.h"
#include "segaic16.h"


UINT16 *segaic16_spriteram_0;
UINT16 *segaic16_spriteram_1;




/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE sega16sp_state *get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == SEGA16SP);

	return (sega16sp_state *)downcast<legacy_device_base *>(device)->token();
}

INLINE const sega16sp_interface *get_interface( device_t *device )
{
	assert(device != NULL);
	assert((device->type() == SEGA16SP));
	return (const sega16sp_interface *) device->static_config();
}

/*******************************************************************************************
 *
 *  Hang On-style sprites
 *
 *      Offs  Bits               Usage
 *       +0   bbbbbbbb --------  Bottom scanline of sprite - 1
 *       +0   -------- tttttttt  Top scanline of sprite - 1
 *       +2   bbbb---- --------  Sprite bank
 *       +2   -------x xxxxxxxx  X position of sprite (position $BD is screen position 0)
 *       +4   pppppppp pppppppp  Signed 16-bit pitch value between scanlines
 *       +6   -ooooooo oooooooo  Offset within selected sprite bank
 *       +6   f------- --------  Horizontal flip: read the data backwards if set
 *       +8   --cccccc --------  Sprite color palette
 *       +8   -------- zzzzzz--  Zoom factor
 *       +8   -------- ------pp  Sprite priority
 *       +E   dddddddd dddddddd  Scratch space for current address
 *
 *  Special notes:
 *
 *      There is an interaction between the horizonal flip bit and the offset.
 *      The offset is maintained as a 16-bit value, even though only the lower
 *      15 bits are used for the address. The top bit is used to control flipping.
 *      This means that if the low 15 bits overflow during rendering, the sprite
 *      data will be read backwards after the overflow. This is important to
 *      emulate correctly as many games make use of this feature to render sprites
 *      at the beginning of a bank.
 *
 *******************************************************************************************/

#define hangon_draw_pixel()													\
	/* only draw if onscreen, not 0 or 15 */								\
	if (pix != 0 && pix != 15)												\
	{																		\
		/* are we high enough priority to be visible? */					\
		if (sprpri > pri[x])												\
		{																	\
			/* shadow/hilight mode? */										\
			if (color == sega16sp->colorbase + (0x3f << 4))						\
				dest[x] += sega16sp->shadow ? segaic16_palette.entries*2 : segaic16_palette.entries;	\
																			\
			/* regular draw */												\
			else															\
				dest[x] = pix | color;										\
		}																	\
																			\
		/* always mark priority so no one else draws here */				\
		pri[x] = 0xff;														\
	}																		\

void segaic16_sprites_hangon_draw(running_machine &machine, device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 numbanks = machine.root_device().memregion("gfx2")->bytes() / 0x10000;
	const UINT16 *spritebase = (const UINT16 *)machine.root_device().memregion("gfx2")->base();
	const UINT8 *zoom = (const UINT8 *)machine.root_device().memregion("proms")->base();
	sega16sp_state *sega16sp = get_safe_token(device);
	UINT16* data = sega16sp->spriteram;

	/* first scan forward to find the end of the list */
	for (data = sega16sp->spriteram; data < sega16sp->spriteram + sega16sp->ramsize/2; data += 8)
		if ((data[0] >> 8) > 0xf0)
			break;

	/* now scan backwards and render the sprites in order */
	for (data -= 8; data >= sega16sp->spriteram; data -= 8)
	{
		int bottom  = (data[0] >> 8) + 1;
		int top     = (data[0] & 0xff) + 1;
		int bank    = sega16sp->bank[(data[1] >> 12) & 0xf];
		int xpos    = (data[1] & 0x1ff) - 0xbd;
		int pitch   = (INT16)data[2];
		UINT16 addr = data[3];
		int color   = sega16sp->colorbase + (((data[4] >> 8) & 0x3f) << 4);
		int vzoom   = (data[4] >> 2) & 0x3f;
		int hzoom   = vzoom << 1;
		int sprpri  = 1 << ((data[4] >> 0) & 0x3);
		int x, y, pix, zaddr, zmask;
		const UINT16 *spritedata;

		/* initialize the end address to the start address */
		data[7] = addr;

		/* if hidden, or top greater than/equal to bottom, or invalid bank, punt */
		if ((top >= bottom) || bank == 255)
			continue;

		/* clamp to within the memory region size */
		if (numbanks)
			bank %= numbanks;
		spritedata = spritebase + 0x8000 * bank;

		/* determine the starting zoom address and mask */
		zaddr = (vzoom & 0x38) << 5;
		zmask = 1 << (vzoom & 7);

		/* loop from top to bottom */
		for (y = top; y < bottom; y++)
		{
			/* advance a row */
			addr += pitch;

			/* if the zoom bit says so, add pitch a second time */
			if (zoom[zaddr++] & zmask)
				addr += pitch;

			/* skip drawing if not within the cliprect */
			if (y >= cliprect.min_y && y <= cliprect.max_y)
			{
				UINT16 *dest = &bitmap.pix16(y);
				UINT8 *pri = &machine.priority_bitmap.pix8(y);
				int xacc = 0x00;

				/* note that the System 16A sprites have a design flaw that allows the address */
				/* to carry into the flip flag, which is the topmost bit -- it is very important */
				/* to emulate this as the games compensate for it */

				/* non-flipped case */
				if (!(addr & 0x8000))
				{
					/* start at the word before because we preincrement below */
					data[7] = addr - 1;
					for (x = xpos; x <= cliprect.max_x; )
					{
						UINT16 pixels = spritedata[++data[7] & 0x7fff];

						/* draw four pixels */
						pix = (pixels >> 12) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x) hangon_draw_pixel(); x++; }
						pix = (pixels >>  8) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x) hangon_draw_pixel(); x++; }
						pix = (pixels >>  4) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x) hangon_draw_pixel(); x++; }
						pix = (pixels >>  0) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x) hangon_draw_pixel(); x++; }

						/* stop if the last pixel in the group was 0xf */
						if (pix == 15)
							break;
					}
				}

				/* flipped case */
				else
				{
					/* start at the word after because we predecrement below */
					data[7] = addr + 1;
					for (x = xpos; x <= cliprect.max_x; )
					{
						UINT16 pixels = spritedata[--data[7] & 0x7fff];

						/* draw four pixels */
						pix = (pixels >>  0) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x) hangon_draw_pixel(); x++; }
						pix = (pixels >>  4) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x) hangon_draw_pixel(); x++; }
						pix = (pixels >>  8) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x) hangon_draw_pixel(); x++; }
						pix = (pixels >> 12) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x) hangon_draw_pixel(); x++; }

						/* stop if the last pixel in the group was 0xf */
						if (pix == 15)
							break;
					}
				}
			}
		}
	}
}



/*******************************************************************************************
 *
 *  Space Harrier-style sprites
 *
 *      Offs  Bits               Usage
 *       +0   bbbbbbbb --------  Bottom scanline of sprite - 1
 *       +0   -------- tttttttt  Top scanline of sprite - 1
 *       +2   bbbb---- --------  Sprite bank
 *       +2   -------x xxxxxxxx  X position of sprite (position $BD is screen position 0)
 *       +4   s------- --------  Sprite shadow enable (0=enable, 1=disable)
 *       +4   -p------ --------  Sprite priority
 *       +4   --cccccc --------  Sprite color palette
 *       +4   -------- -ppppppp  Signed 7-bit pitch value between scanlines
 *       +6   f------- --------  Horizontal flip: read the data backwards if set
 *       +6   -ooooooo oooooooo  Offset within selected sprite bank
 *       +8   --zzzzzz --------  Horizontal zoom factor
 *       +8   -------- --zzzzzz  Vertical zoom factor
 *       +E   dddddddd dddddddd  Scratch space for current address
 *
 *  Special notes:
 *
 *      There is an interaction between the horizonal flip bit and the offset.
 *      The offset is maintained as a 16-bit value, even though only the lower
 *      15 bits are used for the address. The top bit is used to control flipping.
 *      This means that if the low 15 bits overflow during rendering, the sprite
 *      data will be read backwards after the overflow. This is important to
 *      emulate correctly as many games make use of this feature to render sprites
 *      at the beginning of a bank.
 *
 *******************************************************************************************/

#define sharrier_draw_pixel()												\
	/* only draw if onscreen, not 0 or 15 */								\
	if (pix != 0 && pix != 15)												\
	{																		\
		/* are we high enough priority to be visible? */					\
		if (sprpri > pri[x])												\
		{																	\
			/* shadow/hilight mode? */										\
			if (shadow && pix == 0xa)										\
				dest[x] += (segaic16_paletteram[dest[x]] & 0x8000) ? segaic16_palette.entries*2 : segaic16_palette.entries;	\
																			\
			/* regular draw */												\
			else															\
				dest[x] = pix | color;										\
		}																	\
																			\
		/* always mark priority so no one else draws here */				\
		pri[x] = 0xff;														\
	}																		\

void segaic16_sprites_sharrier_draw(running_machine &machine, device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 numbanks = machine.root_device().memregion("gfx2")->bytes() / 0x20000;
	const UINT32 *spritebase = (const UINT32 *)machine.root_device().memregion("gfx2")->base();
	const UINT8 *zoom = (const UINT8 *)machine.root_device().memregion("proms")->base();
	sega16sp_state *sega16sp = get_safe_token(device);
	UINT16* data = sega16sp->spriteram;

	/* first scan forward to find the end of the list */
	for (data = sega16sp->spriteram; data < sega16sp->spriteram + sega16sp->ramsize/2; data += 8)
		if ((data[0] >> 8) > 0xf0)
			break;

	/* now scan backwards and render the sprites in order */
	for (data -= 8; data >= sega16sp->spriteram; data -= 8)
	{
		int bottom  = (data[0] >> 8) + 1;
		int top     = (data[0] & 0xff) + 1;
		int bank    = sega16sp->bank[(data[1] >> 12) & 0x7];
		int xpos    = (data[1] & 0x1ff) - 0xbd;
		int shadow  = (~data[2] >> 15) & 1;
		int sprpri  = ((data[2] >> 14) & 1) ? (1<<3) : (1<<1);
		int color   = sega16sp->colorbase + (((data[2] >> 8) & 0x3f) << 4);
		int pitch   = (INT16)(data[2] << 9) >> 9;
		UINT16 addr = data[3];
		int hzoom   = ((data[4] >> 8) & 0x3f) << 1;
		int vzoom   = (data[4] >> 0) & 0x3f;
		int x, y, pix, zaddr, zmask;
		const UINT32 *spritedata;

		/* initialize the end address to the start address */
		data[7] = addr;

		/* if hidden, or top greater than/equal to bottom, or invalid bank, punt */
		if ((top >= bottom) || bank == 255)
			continue;

		/* clamp to within the memory region size */
		if (numbanks)
			bank %= numbanks;
		spritedata = spritebase + 0x8000 * bank;

		/* determine the starting zoom address and mask */
		zaddr = (vzoom & 0x38) << 5;
		zmask = 1 << (vzoom & 7);

		/* loop from top to bottom */
		for (y = top; y < bottom; y++)
		{
			/* advance a row */
			addr += pitch;

			/* if the zoom bit says so, add pitch a second time */
			if (zoom[zaddr++] & zmask)
				addr += pitch;

			/* skip drawing if not within the cliprect */
			if (y >= cliprect.min_y && y <= cliprect.max_y)
			{
				UINT16 *dest = &bitmap.pix16(y);
				UINT8 *pri = &machine.priority_bitmap.pix8(y);
				int xacc = 0x00;

				/* note that the System 16A sprites have a design flaw that allows the address */
				/* to carry into the flip flag, which is the topmost bit -- it is very important */
				/* to emulate this as the games compensate for it */

				/* non-flipped case */
				if (!(addr & 0x8000))
				{
					/* start at the word before because we preincrement below */
					data[7] = addr - 1;
					for (x = xpos; x <= cliprect.max_x; )
					{
						UINT32 pixels = spritedata[++data[7] & 0x7fff];

						/* draw 8 pixels */
						pix = (pixels >> 28) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x) sharrier_draw_pixel(); x++; }
						pix = (pixels >> 24) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x) sharrier_draw_pixel(); x++; }
						pix = (pixels >> 20) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x) sharrier_draw_pixel(); x++; }
						pix = (pixels >> 16) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x) sharrier_draw_pixel(); x++; }
						pix = (pixels >> 12) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x) sharrier_draw_pixel(); x++; }
						pix = (pixels >>  8) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x) sharrier_draw_pixel(); x++; }
						pix = (pixels >>  4) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x) sharrier_draw_pixel(); x++; }
						pix = (pixels >>  0) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x) sharrier_draw_pixel(); x++; }

						/* stop if the last pixel in the group was 0xf */
						if (pix == 15)
							break;
					}
				}

				/* flipped case */
				else
				{
					/* start at the word after because we predecrement below */
					data[7] = addr + 1;
					for (x = xpos; x <= cliprect.max_x; )
					{
						UINT32 pixels = spritedata[--data[7] & 0x7fff];

						/* draw 8 pixels */
						pix = (pixels >>  0) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x) sharrier_draw_pixel(); x++; }
						pix = (pixels >>  4) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x) sharrier_draw_pixel(); x++; }
						pix = (pixels >>  8) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x) sharrier_draw_pixel(); x++; }
						pix = (pixels >> 12) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x) sharrier_draw_pixel(); x++; }
						pix = (pixels >> 16) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x) sharrier_draw_pixel(); x++; }
						pix = (pixels >> 20) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x) sharrier_draw_pixel(); x++; }
						pix = (pixels >> 24) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x) sharrier_draw_pixel(); x++; }
						pix = (pixels >> 28) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= cliprect.min_x) sharrier_draw_pixel(); x++; }

						/* stop if the last pixel in the group was 0xf */
						if (pix == 15)
							break;
					}
				}
			}
		}
	}
}



/*******************************************************************************************
 *
 *  System 16A-style sprites
 *
 *      Offs  Bits               Usage
 *       +0   bbbbbbbb --------  Bottom scanline of sprite - 1
 *       +0   -------- tttttttt  Top scanline of sprite - 1
 *       +2   -------x xxxxxxxx  X position of sprite (position $BD is screen position 0)
 *       +4   pppppppp pppppppp  Signed 16-bit pitch value between scanlines
 *       +6   -ooooooo oooooooo  Offset within selected sprite bank
 *       +6   f------- --------  Horizontal flip: read the data backwards if set
 *       +8   --cccccc --------  Sprite color palette
 *       +8   -------- -bbb----  Sprite bank
 *       +8   -------- ------pp  Sprite priority
 *       +E   dddddddd dddddddd  Scratch space for current address
 *
 *  Special notes:
 *
 *      There is an interaction between the horizonal flip bit and the offset.
 *      The offset is maintained as a 16-bit value, even though only the lower
 *      15 bits are used for the address. The top bit is used to control flipping.
 *      This means that if the low 15 bits overflow during rendering, the sprite
 *      data will be read backwards after the overflow. This is important to
 *      emulate correctly as many games make use of this feature to render sprites
 *      at the beginning of a bank.
 *
 *******************************************************************************************/

#define system16a_draw_pixel()												\
	/* only draw if onscreen, not 0 or 15 */								\
	if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) \
	{																		\
		/* are we high enough priority to be visible? */					\
		if (sprpri > pri[x])												\
		{																	\
			/* shadow/hilight mode? */										\
			if (color == sega16sp->colorbase + (0x3f << 4))						\
				dest[x] += (segaic16_paletteram[dest[x]] & 0x8000) ? segaic16_palette.entries*2 : segaic16_palette.entries;	\
																			\
			/* regular draw */												\
			else															\
				dest[x] = pix | color;										\
		}																	\
																			\
		/* always mark priority so no one else draws here */				\
		pri[x] = 0xff;														\
	}																		\

void segaic16_sprites_16a_draw(running_machine &machine, device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 numbanks = machine.root_device().memregion("gfx2")->bytes() / 0x10000;
	const UINT16 *spritebase = (const UINT16 *)machine.root_device().memregion("gfx2")->base();
	sega16sp_state *sega16sp = get_safe_token(device);
	UINT16* data = sega16sp->spriteram;

	/* first scan forward to find the end of the list */
	for (data = sega16sp->spriteram; data < sega16sp->spriteram + sega16sp->ramsize/2; data += 8)
		if ((data[0] >> 8) > 0xf0)
			break;

	/* now scan backwards and render the sprites in order */
	for (data -= 8; data >= sega16sp->spriteram; data -= 8)
	{
		int bottom  = (data[0] >> 8) + 1;
		int top     = (data[0] & 0xff) + 1;
		int xpos    = (data[1] & 0x1ff) - 0xbd;
		int pitch   = (INT16)data[2];
		UINT16 addr = data[3];
		int color   = sega16sp->colorbase + (((data[4] >> 8) & 0x3f) << 4);
		int bank    = sega16sp->bank[(data[4] >> 4) & 0x7];
		int sprpri  = 1 << ((data[4] >> 0) & 0x3);
		const UINT16 *spritedata;
		int x, y, pix, xdelta = 1;

		/* initialize the end address to the start address */
		data[7] = addr;

		/* if hidden, or top greater than/equal to bottom, or invalid bank, punt */
		if ((top >= bottom) || bank == 255)
			continue;

		/* clamp to within the memory region size */
		if (numbanks)
			bank %= numbanks;
		spritedata = spritebase + 0x8000 * bank;

		/* adjust positions for screen flipping */
		if (sega16sp->flip)
		{
			int temp = top;
			top = 224 - bottom;
			bottom = 224 - temp;
			xpos = 320 - xpos;
			xdelta = -1;
		}

		/* loop from top to bottom */
		for (y = top; y < bottom; y++)
		{
			/* advance a row */
			addr += pitch;

			/* skip drawing if not within the cliprect */
			if (y >= cliprect.min_y && y <= cliprect.max_y)
			{
				UINT16 *dest = &bitmap.pix16(y);
				UINT8 *pri = &machine.priority_bitmap.pix8(y);

				/* note that the System 16A sprites have a design flaw that allows the address */
				/* to carry into the flip flag, which is the topmost bit -- it is very important */
				/* to emulate this as the games compensate for it */

				/* non-flipped case */
				if (!(addr & 0x8000))
				{
					/* start at the word before because we preincrement below */
					data[7] = addr - 1;
					for (x = xpos; ((xpos - x) & 0x1ff) != 1; )
					{
						UINT16 pixels = spritedata[++data[7] & 0x7fff];

						/* draw four pixels */
						pix = (pixels >> 12) & 0xf; system16a_draw_pixel(); x += xdelta;
						pix = (pixels >>  8) & 0xf; system16a_draw_pixel(); x += xdelta;
						pix = (pixels >>  4) & 0xf; system16a_draw_pixel(); x += xdelta;
						pix = (pixels >>  0) & 0xf; system16a_draw_pixel(); x += xdelta;

						/* stop if the last pixel in the group was 0xf */
						if (pix == 15)
							break;
					}
				}

				/* flipped case */
				else
				{
					/* start at the word after because we predecrement below */
					data[7] = addr + 1;
					for (x = xpos; ((xpos - x) & 0x1ff) != 1; )
					{
						UINT16 pixels = spritedata[--data[7] & 0x7fff];

						/* draw four pixels */
						pix = (pixels >>  0) & 0xf; system16a_draw_pixel(); x += xdelta;
						pix = (pixels >>  4) & 0xf; system16a_draw_pixel(); x += xdelta;
						pix = (pixels >>  8) & 0xf; system16a_draw_pixel(); x += xdelta;
						pix = (pixels >> 12) & 0xf; system16a_draw_pixel(); x += xdelta;

						/* stop if the last pixel in the group was 0xf */
						if (pix == 15)
							break;
					}
				}
			}
		}
	}
}



/*******************************************************************************************
 *
 *  System 16B-style sprites
 *
 *      Offs  Bits               Usage
 *       +0   bbbbbbbb --------  Bottom scanline of sprite - 1
 *       +0   -------- tttttttt  Top scanline of sprite - 1
 *       +2   -------x xxxxxxxx  X position of sprite (position $BD is screen position 0)
 *       +4   e------- --------  Signify end of sprite list
 *       +4   -h------ --------  Hide this sprite
 *       +4   -------f --------  Horizontal flip: read the data backwards if set
 *       +4   -------- pppppppp  Signed 8-bit pitch value between scanlines
 *       +6   oooooooo oooooooo  Offset within selected sprite bank
 *       +8   ----bbbb --------  Sprite bank
 *       +8   -------- pp------  Sprite priority, relative to tilemaps
 *       +8   -------- --cccccc  Sprite color palette
 *       +A   ------vv vvv-----  Vertical zoom factor (0 = full size, 0x10 = half size)
 *       +A   -------- ---hhhhh  Horizontal zoom factor (0 = full size, 0x10 = half size)
 *       +E   dddddddd dddddddd  Scratch space for current address
 *
 *  Note that the zooming described below is 100% accurate to the real board.
 *
 *******************************************************************************************/

#define system16b_draw_pixel()												\
	/* only draw if onscreen, not 0 or 15 */								\
	if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) \
	{																		\
		/* are we high enough priority to be visible? */					\
		if (sprpri > pri[x])												\
		{																	\
			/* shadow/hilight mode? */										\
			if (color == sega16sp->colorbase + (0x3f << 4))						\
			{																\
				/* we have to check this for System 18 so that we don't */  \
				/* attempt to shadow VDP pixels */							\
				if (dest[x] < segaic16_palette.entries)								\
					dest[x] += (segaic16_paletteram[dest[x]] & 0x8000) ? segaic16_palette.entries*2 : segaic16_palette.entries; \
			}																\
																			\
			/* regular draw */												\
			else															\
				dest[x] = pix | color;										\
		}																	\
																			\
		/* always mark priority so no one else draws here */				\
		pri[x] = 0xff;														\
	}																		\

void segaic16_sprites_16b_draw(running_machine &machine, device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 numbanks;
	const UINT16 *spritebase;
	sega16sp_state *sega16sp = get_safe_token(device);
	UINT16* data = sega16sp->spriteram;

	spritebase = (const UINT16 *)machine.root_device().memregion("gfx2")->base();
	if (!spritebase)
		return;

	numbanks = machine.root_device().memregion("gfx2")->bytes() / 0x20000;

	/* first scan forward to find the end of the list */
	for (data = sega16sp->spriteram; data < sega16sp->spriteram + sega16sp->ramsize/2; data += 8)
		if (data[2] & 0x8000)
			break;

	/* now scan backwards and render the sprites in order */
	for (data -= 8; data >= sega16sp->spriteram; data -= 8)
	{
		int bottom  = data[0] >> 8;
		int top     = data[0] & 0xff;
		int xpos    = (data[1] & 0x1ff);
		int hide    = data[2] & 0x4000;
		int flip    = data[2] & 0x100;
		int pitch   = (INT8)(data[2] & 0xff);
		UINT16 addr = data[3];
		int bank    = sega16sp->bank[(data[4] >> 8) & 0xf];
		int sprpri  = 1 << ((data[4] >> 6) & 0x3);
		int color   = sega16sp->colorbase + ((data[4] & 0x3f) << 4);
		int vzoom   = (data[5] >> 5) & 0x1f;
		int hzoom   = data[5] & 0x1f;
		const UINT16 *spritedata;
		int x, y, pix, xdelta = 1;

		/* some bootlegs have offset sprites */
		xpos += sega16sp->xoffs;
		xpos &= 0x1ff;

		/* originals all have this offset */
		xpos -= 0xb8;

		/* initialize the end address to the start address */
		data[7] = addr;

		/* if hidden, or top greater than/equal to bottom, or invalid bank, punt */
		if (hide || (top >= bottom) || bank == 255)
			continue;

		/* clamp to within the memory region size */
		if (numbanks)
			bank %= numbanks;
		spritedata = spritebase + 0x10000 * bank;

		/* reset the yzoom counter */
		data[5] &= 0x03ff;

		/* adjust positions for screen flipping */
		if (sega16sp->flip)
		{
			int temp = top;
			top = 224 - bottom;
			bottom = 224 - temp;
			xpos = 320 - xpos;
			xdelta = -1;
		}

		/* loop from top to bottom */
		for (y = top; y < bottom; y++)
		{
			/* advance a row */
			addr += pitch;

			/* accumulate zoom factors; if we carry into the high bit, skip an extra row */
			data[5] += vzoom << 10;
			if (data[5] & 0x8000)
			{
				addr += pitch;
				data[5] &= ~0x8000;
			}

			/* skip drawing if not within the cliprect */
			if (y >= cliprect.min_y && y <= cliprect.max_y)
			{
				UINT16 *dest = &bitmap.pix16(y);
				UINT8 *pri = &machine.priority_bitmap.pix8(y);
				int xacc;

				/* compute the initial X zoom accumulator; this is verified on the real PCB */
				xacc = 4 * hzoom;

				/* non-flipped case */
				if (!flip)
				{
					/* start at the word before because we preincrement below */
					data[7] = addr - 1;
					for (x = xpos; ((xpos - x) & 0x1ff) != 1; )
					{
						UINT16 pixels = spritedata[++data[7]];

						/* draw four pixels */
						pix = (pixels >> 12) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { system16b_draw_pixel(); x += xdelta; }
						pix = (pixels >>  8) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { system16b_draw_pixel(); x += xdelta; }
						pix = (pixels >>  4) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { system16b_draw_pixel(); x += xdelta; }
						pix = (pixels >>  0) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { system16b_draw_pixel(); x += xdelta; }

						/* stop if the last pixel in the group was 0xf */
						if (pix == 15)
							break;
					}
				}

				/* flipped case */
				else
				{
					/* start at the word after because we predecrement below */
					data[7] = addr + 1;
					for (x = xpos; ((xpos - x) & 0x1ff) != 1; )
					{
						UINT16 pixels = spritedata[--data[7]];

						/* draw four pixels */
						pix = (pixels >>  0) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { system16b_draw_pixel(); x += xdelta; }
						pix = (pixels >>  4) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { system16b_draw_pixel(); x += xdelta; }
						pix = (pixels >>  8) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { system16b_draw_pixel(); x += xdelta; }
						pix = (pixels >> 12) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { system16b_draw_pixel(); x += xdelta; }

						/* stop if the last pixel in the group was 0xf */
						if (pix == 15)
							break;
					}
				}
			}
		}
	}
}


/*******************************************************************************************
 *
 *  The Y-board variant has different mixing properties. The sprite implementation itself
 *  is identical, however.
 *
 *******************************************************************************************/

#define yboard_16b_draw_pixel() 											\
	/* only draw if onscreen, not 0 or 15 */								\
	if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) \
	{																		\
		/* are we high enough priority to be visible? */					\
		if ((sprpri & 0x1f) < (pri[x] & 0x1f))								\
		{																	\
			/* shadow/hilight mode? */										\
			if (pix == 14)													\
				dest[x] += (segaic16_paletteram[dest[x]] & 0x8000) ? segaic16_palette.entries*2 : segaic16_palette.entries;	\
																			\
			/* regular draw */												\
			else															\
				dest[x] = pix | color;										\
		}																	\
																			\
		/* always mark priority so no one else draws here */				\
		pri[x] = 0;															\
	}																		\

void segaic16_sprites_yboard_16b_draw(running_machine &machine, device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 numbanks = machine.root_device().memregion("gfx2")->bytes() / 0x20000;
	const UINT16 *spritebase = (const UINT16 *)machine.root_device().memregion("gfx2")->base();
	sega16sp_state *sega16sp = get_safe_token(device);
	UINT16* data = sega16sp->spriteram;

	/* first scan forward to find the end of the list */
	for (data = sega16sp->spriteram; data < sega16sp->spriteram + sega16sp->ramsize/2; data += 8)
		if (data[2] & 0x8000)
			break;

	/* now scan backwards and render the sprites in order */
	for (data -= 8; data >= sega16sp->spriteram; data -= 8)
	{
		int bottom  = data[0] >> 8;
		int top     = data[0] & 0xff;
		int xpos    = (data[1] & 0x1ff) - 0xb8;
		int sprpri  = (data[1] >> 8) & 0x1e;	// 0x00 = high, 0x7f = low -- 0x71 = ship in gforce, 0x31 = strike fighter logo
		int hide    = data[2] & 0x4000;
		int flip    = data[2] & 0x100;
		int pitch   = (INT8)(data[2] & 0xff);
		UINT16 addr = data[3];
		int bank    = sega16sp->bank[(data[4] >> 8) & 0xf];
		int color   = sega16sp->colorbase + ((data[4] & 0x7f) << 4);
		int vzoom   = (data[5] >> 5) & 0x1f;
		int hzoom   = data[5] & 0x1f;
		const UINT16 *spritedata;
		int x, y, pix, xdelta = 1;

		/* initialize the end address to the start address */
		data[7] = addr;

		/* if hidden, or top greater than/equal to bottom, or invalid bank, punt */
		if (hide || (top >= bottom) || bank == 255)
			continue;

		/* clamp to within the memory region size */
		if (numbanks)
			bank %= numbanks;
		spritedata = spritebase + 0x10000 * bank;

		/* reset the yzoom counter */
		data[5] &= 0x03ff;

		/* adjust positions for screen flipping */
		if (sega16sp->flip)
		{
			int temp = top;
			top = 224 - bottom;
			bottom = 224 - temp;
			xpos = 320 - xpos;
			xdelta = -1;
		}

		/* loop from top to bottom */
		for (y = top; y < bottom; y++)
		{
			/* advance a row */
			addr += pitch;

			/* accumulate zoom factors; if we carry into the high bit, skip an extra row */
			data[5] += vzoom << 10;
			if (data[5] & 0x8000)
			{
				addr += pitch;
				data[5] &= ~0x8000;
			}

			/* skip drawing if not within the cliprect */
			if (y >= cliprect.min_y && y <= cliprect.max_y)
			{
				UINT16 *dest = &bitmap.pix16(y);
				UINT8 *pri = &machine.priority_bitmap.pix8(y);
				int xacc;

				/* compute the initial X zoom accumulator; this is verified on the real PCB */
				xacc = 4 * hzoom;

				/* non-flipped case */
				if (!flip)
				{
					/* start at the word before because we preincrement below */
					data[7] = addr - 1;
					for (x = xpos; ((xpos - x) & 0x1ff) != 1; )
					{
						UINT16 pixels = spritedata[++data[7]];

						/* draw four pixels */
						pix = (pixels >> 12) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { yboard_16b_draw_pixel(); x += xdelta; }
						pix = (pixels >>  8) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { yboard_16b_draw_pixel(); x += xdelta; }
						pix = (pixels >>  4) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { yboard_16b_draw_pixel(); x += xdelta; }
						pix = (pixels >>  0) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { yboard_16b_draw_pixel(); x += xdelta; }

						/* stop if the last pixel in the group was 0xf */
						if (pix == 15)
							break;
					}
				}

				/* flipped case */
				else
				{
					/* start at the word after because we predecrement below */
					data[7] = addr + 1;
					for (x = xpos; ((xpos - x) & 0x1ff) != 1; )
					{
						UINT16 pixels = spritedata[--data[7]];

						/* draw four pixels */
						pix = (pixels >>  0) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { yboard_16b_draw_pixel(); x += xdelta; }
						pix = (pixels >>  4) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { yboard_16b_draw_pixel(); x += xdelta; }
						pix = (pixels >>  8) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { yboard_16b_draw_pixel(); x += xdelta; }
						pix = (pixels >> 12) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { yboard_16b_draw_pixel(); x += xdelta; }

						/* stop if the last pixel in the group was 0xf */
						if (pix == 15)
							break;
					}
				}
			}
		}
	}
}



/*******************************************************************************************
 *
 *  Out Run/X-Board-style sprites
 *
 *      Offs  Bits               Usage
 *       +0   e------- --------  Signify end of sprite list
 *       +0   -h-h---- --------  Hide this sprite if either bit is set
 *       +0   ----bbb- --------  Sprite bank
 *       +0   -------t tttttttt  Top scanline of sprite + 256
 *       +2   oooooooo oooooooo  Offset within selected sprite bank
 *       +4   ppppppp- --------  Signed 7-bit pitch value between scanlines
 *       +4   -------x xxxxxxxx  X position of sprite (position $BE is screen position 0)
 *       +6   -s------ --------  Enable shadows
 *       +6   --pp---- --------  Sprite priority, relative to tilemaps
 *       +6   ------vv vvvvvvvv  Vertical zoom factor (0x200 = full size, 0x100 = half size, 0x300 = 2x size)
 *       +8   y------- --------  Render from top-to-bottom (1) or bottom-to-top (0) on screen
 *       +8   -f------ --------  Horizontal flip: read the data backwards if set
 *       +8   --x----- --------  Render from left-to-right (1) or right-to-left (0) on screen
 *       +8   ------hh hhhhhhhh  Horizontal zoom factor (0x200 = full size, 0x100 = half size, 0x300 = 2x size)
 *       +E   dddddddd dddddddd  Scratch space for current address
 *
 *    Out Run only:
 *       +A   hhhhhhhh --------  Height in scanlines - 1
 *       +A   -------- -ccccccc  Sprite color palette
 *
 *    X-Board only:
 *       +A   ----hhhh hhhhhhhh  Height in scanlines - 1
 *       +C   -------- cccccccc  Sprite color palette
 *
 *******************************************************************************************/

#define outrun_draw_pixel() 												\
	/* only draw if onscreen, not 0 or 15 */								\
	if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) \
	{																		\
		/* are we high enough priority to be visible? */					\
		if (sprpri > pri[x])												\
		{																	\
			/* shadow/hilight mode? */										\
			if (shadow && pix == 0xa)										\
				dest[x] += (segaic16_paletteram[dest[x]] & 0x8000) ? segaic16_palette.entries*2 : segaic16_palette.entries;	\
																			\
			/* regular draw */												\
			else															\
				dest[x] = pix | color;										\
		}																	\
																			\
		/* always mark priority so no one else draws here */				\
		pri[x] = 0xff;														\
	}																		\

static void segaic16_sprites_xboard_outrun_draw(running_machine &machine, device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int type)
{
	UINT8 numbanks = machine.root_device().memregion("gfx2")->bytes() / 0x40000;
	const UINT32 *spritebase = (const UINT32 *)machine.root_device().memregion("gfx2")->base();
	sega16sp_state *sega16sp = get_safe_token(device);
	UINT16* data = sega16sp->spriteram;

	/* first scan forward to find the end of the list */
	for (data = sega16sp->buffer; data < sega16sp->buffer + sega16sp->ramsize/2; data += 8)
		if (data[0] & 0x8000)
			break;

	/* now scan backwards and render the sprites in order */
	for (data -= 8; data >= sega16sp->buffer; data -= 8)
	{
		int hide    = (data[0] & 0x5000);
		int bank    = (data[0] >> 9) & 7;
		int top     = (data[0] & 0x1ff) - 0x100;
		UINT16 addr = data[1];
		int pitch   = (INT16)((data[2] >> 1) | ((data[4] & 0x1000) << 3)) >> 8;
		int xpos    = data[2] & 0x1ff;
		int shadow  = (data[3] >> 14) & 1;
		int sprpri  = 1 << ((data[3] >> 12) & 3);
		int vzoom   = data[3] & 0x7ff;
		int ydelta  = (data[4] & 0x8000) ? 1 : -1;
		int flip    = (~data[4] >> 14) & 1;
		int xdelta  = (data[4] & 0x2000) ? 1 : -1;
		int hzoom   = data[4] & 0x7ff;
		int height  = ((type == SEGAIC16_SPRITES_OUTRUN) ? (data[5] >> 8) : (data[5] & 0xfff)) + 1;
		int color   = sega16sp->colorbase + (((type == SEGAIC16_SPRITES_OUTRUN) ? (data[5] & 0x7f) : (data[6] & 0xff)) << 4);
		int x, y, ytarget, yacc = 0, pix;
		const UINT32 *spritedata;

		/* adjust X coordinate */
		/* note: the threshhold below is a guess. If it is too high, rachero will draw garbage */
		/* If it is too low, smgp won't draw the bottom part of the road */
		if (xpos < 0x80 && xdelta < 0)
			xpos += 0x200;
		xpos -= 0xbe;

		/* initialize the end address to the start address */
		data[7] = addr;

		/* if hidden, or top greater than/equal to bottom, or invalid bank, punt */
		if (hide || height == 0)
			continue;

		/* clamp to within the memory region size */
		if (numbanks)
			bank %= numbanks;
		spritedata = spritebase + 0x10000 * bank;

		/* clamp to a maximum of 8x (not 100% confirmed) */
		if (vzoom < 0x40) vzoom = 0x40;
		if (hzoom < 0x40) hzoom = 0x40;

		/* loop from top to bottom */
		ytarget = top + ydelta * height;
		for (y = top; y != ytarget; y += ydelta)
		{
			/* skip drawing if not within the cliprect */
			if (y >= cliprect.min_y && y <= cliprect.max_y)
			{
				UINT16 *dest = &bitmap.pix16(y);
				UINT8 *pri = &machine.priority_bitmap.pix8(y);
				int xacc = 0;

				/* non-flipped case */
				if (!flip)
				{
					/* start at the word before because we preincrement below */
					data[7] = addr - 1;
					for (x = xpos; (xdelta > 0 && x <= cliprect.max_x) || (xdelta < 0 && x >= cliprect.min_x); )
					{
						UINT32 pixels = spritedata[++data[7]];

						/* draw four pixels */
						pix = (pixels >> 28) & 0xf; while (xacc < 0x200) { outrun_draw_pixel(); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 24) & 0xf; while (xacc < 0x200) { outrun_draw_pixel(); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 20) & 0xf; while (xacc < 0x200) { outrun_draw_pixel(); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 16) & 0xf; while (xacc < 0x200) { outrun_draw_pixel(); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 12) & 0xf; while (xacc < 0x200) { outrun_draw_pixel(); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >>  8) & 0xf; while (xacc < 0x200) { outrun_draw_pixel(); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >>  4) & 0xf; while (xacc < 0x200) { outrun_draw_pixel(); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >>  0) & 0xf; while (xacc < 0x200) { outrun_draw_pixel(); x += xdelta; xacc += hzoom; } xacc -= 0x200;

						/* stop if the second-to-last pixel in the group was 0xf */
						if ((pixels & 0x000000f0) == 0x000000f0)
							break;
					}
				}

				/* flipped case */
				else
				{
					/* start at the word after because we predecrement below */
					data[7] = addr + 1;
					for (x = xpos; (xdelta > 0 && x <= cliprect.max_x) || (xdelta < 0 && x >= cliprect.min_x); )
					{
						UINT32 pixels = spritedata[--data[7]];

						/* draw four pixels */
						pix = (pixels >>  0) & 0xf; while (xacc < 0x200) { outrun_draw_pixel(); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >>  4) & 0xf; while (xacc < 0x200) { outrun_draw_pixel(); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >>  8) & 0xf; while (xacc < 0x200) { outrun_draw_pixel(); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 12) & 0xf; while (xacc < 0x200) { outrun_draw_pixel(); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 16) & 0xf; while (xacc < 0x200) { outrun_draw_pixel(); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 20) & 0xf; while (xacc < 0x200) { outrun_draw_pixel(); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 24) & 0xf; while (xacc < 0x200) { outrun_draw_pixel(); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 28) & 0xf; while (xacc < 0x200) { outrun_draw_pixel(); x += xdelta; xacc += hzoom; } xacc -= 0x200;

						/* stop if the second-to-last pixel in the group was 0xf */
						if ((pixels & 0x0f000000) == 0x0f000000)
							break;
					}
				}
			}

			/* accumulate zoom factors; if we carry into the high bit, skip an extra row */
			yacc += vzoom;
			addr += pitch * (yacc >> 9);
			yacc &= 0x1ff;
		}
	}
}

void segaic16_sprites_outrun_draw(running_machine &machine, device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	segaic16_sprites_xboard_outrun_draw(machine, device, bitmap, cliprect, SEGAIC16_SPRITES_OUTRUN);
}

void segaic16_sprites_xboard_draw(running_machine &machine, device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	segaic16_sprites_xboard_outrun_draw(machine, device, bitmap, cliprect, SEGAIC16_SPRITES_XBOARD);
}


/*******************************************************************************************
 *
 *  Y-Board-style sprites
 *
 *      Offs  Bits               Usage
 *       +0   e------- --------  Signify end of sprite list
 *       +0   -----iii iiiiiiii  Address of indirection table (/16)
 *       +2   bbbb---- --------  Upper 4 bits of bank index
 *       +2   ----xxxx xxxxxxxx  X position of sprite (position $600 is screen position 0)
 *       +4   bbbb---- --------  Lower 4 bits of bank index
 *       +4   ----yyyy yyyyyyyy  Y position of sprite (position $600 is screen position 0)
 *       +6   oooooooo oooooooo  Offset within selected sprite bank
 *       +8   hhhhhhhh hhhhhhhh  Height of sprite
 *       +A   -y------ --------  Render from top-to-bottom (1) or bottom-to-top (0) on screen
 *       +A   --f----- --------  Horizontal flip: read the data backwards if set
 *       +A   ---x---- --------  Render from left-to-right (1) or right-to-left (0) on screen
 *       +A   -----zzz zzzzzzzz  Zoom factor
 *       +C   -ccc---- --------  Sprite color
 *       +C   ----rrrr --------  Sprite priority
 *       +C   -------- pppppppp  Signed 8-bit pitch value between scanlines
 *       +E   ----nnnn nnnnnnnn  Index of next sprite
 *
 *  In addition to these parameters, the sprite area is clipped using scanline extents
 *  stored for every pair of scanlines in the rotation RAM. It's a bit of a cheat for us
 *  to poke our nose into the rotation structure, but there are no known cases of Y-board
 *  sprites without rotation RAM.
 *
 *******************************************************************************************/

#define yboard_draw_pixel() 												\
	/* only draw if onscreen */												\
	if (x >= minx && x <= maxx && ind < 0x1fe)								\
		dest[x] = ind | colorpri;											\

void segaic16_sprites_yboard_draw(running_machine &machine, device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 numbanks = machine.root_device().memregion("gfx1")->bytes() / 0x80000;
	const UINT64 *spritebase = (const UINT64 *)machine.root_device().memregion("gfx1")->base();
	const UINT16 *rotatebase = segaic16_rotate[0].buffer ? segaic16_rotate[0].buffer : segaic16_rotate[0].rotateram;
	UINT8 visited[0x1000];
	sega16sp_state *sega16sp = get_safe_token(device);
	int next = 0;
	int y;
	UINT16* data = sega16sp->spriteram;

	/* reset the visited list */
	memset(visited, 0, sizeof(visited));

	/* clear out any scanlines we might be using */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
		if (!(rotatebase[y & ~1] & 0xc000))
			memset(&bitmap.pix16(y, cliprect.min_x), 0xff, cliprect.width() * sizeof(UINT16));

	/* now scan backwards and render the sprites in order */
	for (data = sega16sp->spriteram; !(data[0] & 0x8000) && !visited[next]; data = sega16sp->spriteram + next * 8)
	{
		int hide    = (data[0] & 0x5000);
		UINT16 *indirect = sega16sp->spriteram + ((data[0] & 0x7ff) << 4);
		int bank    = ((data[1] >> 8) & 0x10) | ((data[2] >> 12) & 0x0f);
		int xpos    = (data[1] & 0xfff) - 0x600;
		int top     = (data[2] & 0xfff) - 0x600;
		UINT16 addr = data[3];
		int height  = data[4];
		int ydelta  = (data[5] & 0x4000) ? 1 : -1;
		int flip    = (~data[5] >> 13) & 1;
		int xdelta  = (data[5] & 0x1000) ? 1 : -1;
		int zoom    = data[5] & 0x7ff;
		int colorpri= (data[6] << 1) & 0xfe00;
		int pitch   = (INT8)data[6];
		int x, y, ytarget, yacc = 0, pix, ind;
		const UINT64 *spritedata;
		UINT16 offs;

		/* note that we've visited this entry and get the offset of the next one */
		visited[next] = 1;
		next = data[7] & 0xfff;

		/* if hidden, or top greater than/equal to bottom, or invalid bank, punt */
		if (hide || height == 0)
			continue;

		/* clamp to within the memory region size */
		if (numbanks)
			bank %= numbanks;
		spritedata = spritebase + 0x10000 * bank;

		/* clamp to a maximum of 8x (not 100% confirmed) */
		if (zoom == 0) zoom = 1;

		/* loop from top to bottom */
		ytarget = top + ydelta * height;
		for (y = top; y != ytarget; y += ydelta)
		{
			/* skip drawing if not within the cliprect */
			if (y >= cliprect.min_y && y <= cliprect.max_y)
			{
				UINT16 *dest = &bitmap.pix16(y);
				int minx = rotatebase[y & ~1];
				int maxx = rotatebase[y |  1];
				int xacc = 0;

				/* bit 0x8000 from rotate RAM means that Y is above the top of the screen */
				if ((minx & 0x8000) && ydelta < 0)
					break;

				/* bit 0x4000 from rotate RAM means that Y is below the bottom of the screen */
				if ((minx & 0x4000) && ydelta > 0)
					break;

				/* if either bit is set, skip the rest for this scanline */
				if (!(minx & 0xc000))
				{
					/* clamp min/max to the cliprect */
					minx -= 0x600;
					maxx -= 0x600;
					if (minx < cliprect.min_x)
						minx = cliprect.min_x;
					if (maxx > cliprect.max_x)
						maxx = cliprect.max_x;

					/* non-flipped case */
					if (!flip)
					{
						/* start at the word before because we preincrement below */
						offs = addr - 1;
						for (x = xpos; (xdelta > 0 && x <= maxx) || (xdelta < 0 && x >= minx); )
						{
							UINT64 pixels = spritedata[++offs];

							/* draw four pixels */
							pix = (pixels >> 60) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 56) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 52) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 48) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 44) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 40) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 36) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 32) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 28) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 24) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 20) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 16) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 12) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >>  8) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >>  4) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >>  0) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;

							/* stop if the second-to-last pixel in the group was 0xf */
							if (pix == 0x0f)
								break;
						}
					}

					/* flipped case */
					else
					{
						/* start at the word after because we predecrement below */
						offs = addr + 1;
						for (x = xpos; (xdelta > 0 && x <= maxx) || (xdelta < 0 && x >= minx); )
						{
							UINT64 pixels = spritedata[--offs];

							/* draw four pixels */
							pix = (pixels >>  0) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >>  4) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >>  8) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 12) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 16) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 20) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 24) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 28) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 32) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 36) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 40) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 44) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 48) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 52) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 56) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 60) & 0xf; ind = indirect[pix]; while (xacc < 0x200) { yboard_draw_pixel(); x += xdelta; xacc += zoom; } xacc -= 0x200;

							/* stop if the second-to-last pixel in the group was 0xf */
							if (pix == 0x0f)
								break;
						}
					}
				}
			}

			/* accumulate zoom factors; if we carry into the high bit, skip an extra row */
			yacc += zoom;
			addr += pitch * (yacc >> 9);
			yacc &= 0x1ff;
		}
	}
}

/* bootlegs */

/*

 the system16a bootleg sprite hardware differs in subtle ways on a per game basis
 with each game having the words swapped around.

 there are also some subtle, but important changes when compared to the original
 system16a sprites, mainly the increment of the address not happening until
 the end of the loop

*/

/* ignores the sprite priority until we understand priority better on the bootlegs */
#define system16a_bootleg_draw_pixel()												\
	/* only draw if onscreen, not 0 or 15 */								\
	if (x >= cliprect.min_x && x <= cliprect.max_x && pix != 0 && pix != 15) \
	{																		\
		/* are we high enough priority to be visible? */					\
		if (sprpri || 1)												\
		{																	\
			/* shadow/hilight mode? */										\
			if (color == sega16sp->colorbase + (0x3f << 4))						\
				dest[x] += (segaic16_paletteram[dest[x]] & 0x8000) ? segaic16_palette.entries*2 : segaic16_palette.entries;	\
																			\
			/* regular draw */												\
			else															\
				dest[x] = pix | color;										\
		}																	\
																			\
		/* always mark priority so no one else draws here */				\
		pri[x] = 0xff;														\
	}																		\


/* make this an actual function */
#define system16a_bootleg_draw_core()													\
	{																					\
		const UINT16 *spritedata;														\
		int x, y, pix, xdelta = 1;														\
																						\
		xpos += sega16sp->xoffs;															\
		xpos &= 0x1ff;																	\
																						\
		xpos -= 0xbd;																	\
																						\
		/* initialize the end address to the start address */							\
		data[7] = addr;																\
																						\
		/* if hidden, or top greater than/equal to bottom, or invalid bank, punt */		\
		if ((top >= bottom) || bank == 255)												\
			continue;																	\
																						\
		/* clamp to within the memory region size */									\
		if (numbanks)																	\
			bank %= numbanks;															\
		spritedata = spritebase + 0x8000 * bank;										\
																						\
		/* adjust positions for screen flipping */										\
		if (sega16sp->flip)																	\
		{																				\
			int temp = top;																\
			top = 224 - bottom;															\
			bottom = 224 - temp;														\
			xpos = 320 - xpos;															\
			xdelta = -1;																\
		}																				\
																						\
		/* loop from top to bottom */													\
		for (y = top; y < bottom; y++)													\
		{																				\
																						\
			/* skip drawing if not within the cliprect */								\
			if (y >= cliprect.min_y && y <= cliprect.max_y)							\
			{																			\
				UINT16 *dest = &bitmap.pix16(y);								\
				UINT8 *pri = &machine.priority_bitmap.pix8(y);						\
																						\
				/* note that the System 16A sprites have a design flaw that allows the address */		\
				/* to carry into the flip flag, which is the topmost bit -- it is very important */		\
				/* to emulate this as the games compensate for it */									\
																										\
				/* non-flipped case */																	\
				if (!(addr & 0x8000))																	\
				{																						\
					/* start at the word before because we preincrement below */						\
					data[7] = addr - 1;																\
					for (x = xpos; ((xpos - x) & 0x1ff) != 1; )											\
					{																					\
						UINT16 pixels = spritedata[++data[7] & 0x7fff];									\
																										\
						/* draw four pixels */															\
						pix = (pixels >> 12) & 0xf; system16a_bootleg_draw_pixel(); x += xdelta;		\
						pix = (pixels >>  8) & 0xf; system16a_bootleg_draw_pixel(); x += xdelta;		\
						pix = (pixels >>  4) & 0xf; system16a_bootleg_draw_pixel(); x += xdelta;		\
						pix = (pixels >>  0) & 0xf; system16a_bootleg_draw_pixel(); x += xdelta;		\
																										\
						/* stop if the last pixel in the group was 0xf */								\
						if (pix == 15)																	\
							break;																		\
					}																					\
				}																						\
																										\
				/* flipped case */																		\
				else																					\
				{																						\
					/* start at the word after because we predecrement below */							\
					data[7] = addr + 1;																	\
					for (x = xpos; ((xpos - x) & 0x1ff) != 1; )											\
					{																					\
						UINT16 pixels = spritedata[--data[7] & 0x7fff];									\
																										\
						/* draw four pixels */															\
						pix = (pixels >>  0) & 0xf; system16a_bootleg_draw_pixel(); x += xdelta;		\
						pix = (pixels >>  4) & 0xf; system16a_bootleg_draw_pixel(); x += xdelta;		\
						pix = (pixels >>  8) & 0xf; system16a_bootleg_draw_pixel(); x += xdelta;		\
						pix = (pixels >> 12) & 0xf; system16a_bootleg_draw_pixel(); x += xdelta;		\
																										\
						/* stop if the last pixel in the group was 0xf */								\
						if (pix == 15)																	\
							break;																		\
					}																					\
				}																						\
			}																							\
																										\
			/* advance a row - must be done at the end on the bootlegs! */								\
			addr += pitch;																				\
		}																								\
	}																									\



void segaic16_sprites_16a_bootleg_wb3bl_draw(running_machine &machine, device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 numbanks = machine.root_device().memregion("gfx2")->bytes() / 0x10000;
	const UINT16 *spritebase = (const UINT16 *)machine.root_device().memregion("gfx2")->base();
	sega16sp_state *sega16sp = get_safe_token(device);
	UINT16* data = sega16sp->spriteram;

	for (data = sega16sp->spriteram; data < sega16sp->spriteram+ sega16sp->ramsize/2; data += 8)
	{
		int bottom  = (data[4] >> 8);
		int top     = (data[4] & 0xff);
		int xpos    = (data[0]);
		int pitch   = (INT16)data[5];
		UINT16 addr = data[1];
		int color   = sega16sp->colorbase + (((data[6] >> 8) & 0x3f) << 4);
		int bank    = sega16sp->bank[(data[6] >> 4) & 0x7];
		int sprpri  = 1 << ((data[6] >> 0) & 0x3);

		system16a_bootleg_draw_core();
	}
}

/* 4 player passing shot is different to this.. */
void segaic16_sprites_16a_bootleg_passhtb_draw(running_machine &machine, device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 numbanks = machine.root_device().memregion("gfx2")->bytes() / 0x10000;
	const UINT16 *spritebase = (const UINT16 *)machine.root_device().memregion("gfx2")->base();
	sega16sp_state *sega16sp = get_safe_token(device);
	UINT16* data = sega16sp->spriteram;

	for (data = sega16sp->spriteram; data < sega16sp->spriteram+ sega16sp->ramsize/2; data += 8)
	{
		int bottom  = (data[1] >> 8)-1;
		int top     = (data[1] & 0xff)-1;
		int xpos    = (data[0]);
		int pitch   = (INT16)data[3];
		UINT16 addr = data[2];
		int color   = sega16sp->colorbase + (((data[5] >> 8) & 0x3f) << 4);
		int bank    = sega16sp->bank[(data[5] >> 4) & 0x7];
		int sprpri  = 1 << ((data[5] >> 0) & 0x3);

		system16a_bootleg_draw_core();
	}
}

void segaic16_sprites_16a_bootleg_shinobld_draw(running_machine &machine, device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 numbanks = machine.root_device().memregion("gfx2")->bytes() / 0x10000;
	const UINT16 *spritebase = (const UINT16 *)machine.root_device().memregion("gfx2")->base();
	sega16sp_state *sega16sp = get_safe_token(device);
	UINT16* data = sega16sp->spriteram;

	for (data = sega16sp->spriteram; data < sega16sp->spriteram+ sega16sp->ramsize/2; data += 8)
	{
		int bottom  = (data[0] >> 8)-1;
		int top     = (data[0] & 0xff)-1;
		int xpos    = (data[1]);
		int pitch   = (INT16)data[2];
		UINT16 addr = data[3];
		int color   = sega16sp->colorbase + (((data[4] >> 8) & 0x3f) << 4);
		int bank    = sega16sp->bank[(data[4] >> 4) & 0x7];
		int sprpri  = 1 << ((data[4] >> 0) & 0x3);

		system16a_bootleg_draw_core();
	}
}

/*************************************
 *
 *  General sprite drawing
 *
 *************************************/

void segaic16_sprites_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int which)
{
	device_t* device = 0;
	sega16sp_state *sega16sp;

	if (!which)
		device = screen.machine().device("segaspr1");
	else
		device = screen.machine().device("segaspr2");

	if (!device)
		fatalerror("segaic16_sprites_draw device not found\n");

	sega16sp = get_safe_token(device);

	if (!sega16sp->which)
		sega16sp->spriteram = segaic16_spriteram_0;
	else
		sega16sp->spriteram = segaic16_spriteram_1;

	(*sega16sp->draw)(screen.machine(), device, bitmap, cliprect);
}



/*************************************
 *
 *  General sprite banking
 *
 *************************************/

void segaic16_sprites_set_bank(running_machine &machine, int which, int banknum, int offset)
{
	device_t* device = 0;

	if (!which)
		device = machine.device("segaspr1");
	else
		device = machine.device("segaspr2");

	if (!device)
		fatalerror("segaic16_sprites_set_bank device not found\n");

	sega16sp_state *sega16sp = get_safe_token(device);

	if (sega16sp->bank[banknum] != offset)
	{
		screen_device &screen = *machine.primary_screen;
		screen.update_partial(screen.vpos());
		sega16sp->bank[banknum] = offset;
	}
}



/*************************************
 *
 *  General sprite screen flipping
 *
 *************************************/

void segaic16_sprites_set_flip(running_machine &machine, int which, int flip)
{
	device_t* device = 0;

	if (!which)
		device = machine.device("segaspr1");
	else
		device = machine.device("segaspr2");

	if (!device)
		fatalerror("segaic16_sprites_set_flip device not found\n");

	sega16sp_state *sega16sp = get_safe_token(device);

	flip = (flip != 0);
	if (sega16sp->flip != flip)
	{
		screen_device &screen = *machine.primary_screen;
		screen.update_partial(screen.vpos());
		sega16sp->flip = flip;
	}
}



/*************************************
 *
 *  General sprite shadows
 *
 *************************************/

void segaic16_sprites_set_shadow(running_machine &machine, int which, int shadow)
{
	device_t* device = 0;

	if (!which)
		device = machine.device("segaspr1");
	else
		device = machine.device("segaspr2");

	if (!device)
		fatalerror("segaic16_sprites_set_shadow device not found\n");

	sega16sp_state *sega16sp = get_safe_token(device);

	shadow = (shadow != 0);
	if (sega16sp->shadow != shadow)
	{
		screen_device &screen = *machine.primary_screen;
		screen.update_partial(screen.vpos());
		sega16sp->shadow = shadow;
	}
}



/*************************************
 *
 *  General sprite buffer control
 *
 *************************************/

static void segaic16_sprites_buffer(device_t* device)
{
	sega16sp_state *sega16sp = get_safe_token(device);

	if (!sega16sp->which)
		sega16sp->spriteram = segaic16_spriteram_0;
	else
		sega16sp->spriteram = segaic16_spriteram_1;


	if (sega16sp->buffer)
	{
		UINT32 *src = (UINT32 *)sega16sp->spriteram;
		UINT32 *dst = (UINT32 *)sega16sp->buffer;
		int i;

		/* swap the halves of the sprite RAM */
		for (i = 0; i < sega16sp->ramsize/4; i++)
		{
			UINT32 temp = *src;
			*src++ = *dst;
			*dst++ = temp;
		}

		/* hack for thunderblade */
		*sega16sp->spriteram = 0xffff;
	}

	/* we will render the sprites when the video update happens */
}


WRITE16_HANDLER( segaic16_sprites_draw_0_w )
{
	device_t* device = 0;

	device = space->machine().device("segaspr1");

	if (!device)
		fatalerror("segaic16_sprites_draw_0_w device not found\n");

	segaic16_sprites_buffer(device);
}


WRITE16_HANDLER( segaic16_sprites_draw_1_w )
{
	device_t* device = 0;

	device = space->machine().device("segaspr2");

	if (!device)
		fatalerror("segaic16_sprites_draw_1_w device not found\n");

	if (device)
		segaic16_sprites_buffer(device);
}


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( sega16sp )
{
	sega16sp_state *sega16sp = get_safe_token(device);
	const sega16sp_interface *intf = get_interface(device);
	int i;

	sega16sp->flip = 0;
	sega16sp->shadow = 0;

	for (i=0;i<16;i++)
		sega16sp->bank[i] = i;// intf->bank[i];

	sega16sp->which = intf->which;
	sega16sp->colorbase = intf->colorbase;
	sega16sp->ramsize = intf->ramsize;
	sega16sp->xoffs = intf->xoffs;
	sega16sp->draw = intf->draw;

	if (intf->buffer)
		sega16sp->buffer = auto_alloc_array(device->machine(), UINT16, sega16sp->ramsize/2);


	device->save_item(NAME(sega16sp->flip));
	device->save_item(NAME(sega16sp->shadow));
	device->save_item(NAME(sega16sp->bank));
	device->save_item(NAME(sega16sp->colorbase));
	device->save_item(NAME(sega16sp->xoffs));

	if (intf->buffer)
		device->save_pointer(NAME(((UINT8 *) sega16sp->buffer)), sega16sp->ramsize);


}


DEVICE_GET_INFO( sega16sp )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(sega16sp_state);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(sega16sp);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					/*info->reset = DEVICE_RESET_NAME(sega16sp);*/	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Sega System SH/HO/OR/16/18/X/Y Sprites");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Sega Video ICs");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}
