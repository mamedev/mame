/* Various Tecmo Sprite implementations

 - check wc90.c, tecmo.c, tbowl.c others? - they seem more significantly different but are they close to each other
   (but at the same time use the same 'layout' table as this implementation)

 - is there a single chip responsible for these, or is it just a family of closely connected implementations?
   (because we seem to need some per-game code right now)

*/


#include "emu.h"
#include "tecmo_spr.h"


const device_type TECMO_SPRITE = &device_creator<tecmo_spr_device>;

tecmo_spr_device::tecmo_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TECMO_SPRITE, "Teccmo 16-bit Sprite", tag, owner, clock, "tecmo_spr", __FILE__),
		device_video_interface(mconfig, *this),
m_gfxregion(0),
m_bootleg(0)
{
}


void tecmo_spr_device::device_start()
{
}

void tecmo_spr_device::device_reset()
{
}


void tecmo_spr_device::set_gfx_region(device_t &device, int gfxregion)
{
	tecmo_spr_device &dev = downcast<tecmo_spr_device &>(device);
	dev.m_gfxregion = gfxregion;
}

void tecmo_spr_device::set_bootleg(device_t &device, int bootleg)
{
	tecmo_spr_device &dev = downcast<tecmo_spr_device &>(device);
	dev.m_bootleg = bootleg;
}


static const UINT8 layout[8][8] =
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

void tecmo_spr_device::gaiden_draw_sprites(screen_device &screen, gfxdecode_device *gfxdecode, const rectangle &cliprect, UINT16* spriteram, int sprite_sizey, int spr_offset_y, int flip_screen, bitmap_ind16 &sprite_bitmap)
{
	gfx_element *gfx = gfxdecode->gfx(m_gfxregion);
	UINT16 *source;
	int sourceinc;


	source = spriteram;
	sourceinc = 8;

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
		UINT32 attributes = source[attributes_word];
		int col, row;

		int enabled = source[attributes_word] & 0x04;

		if (enabled)
		{
			if (m_bootleg == 1)
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
			UINT32 flipx = (attributes & 1);
			UINT32 flipy = (attributes & 2);

			UINT32 color = source[colour_word];
			UINT32 sizex = 1 << ((color >> 0) & 3);                     /* 1,2,4,8 */
			UINT32 sizey = 1 << ((color >> sprite_sizey) & 3); /* 1,2,4,8 */

			/* raiga & fstarfrc need something like this */
			UINT32 number = (source[tilenumber_word]);

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


			bitmap_ind16* bitmap;




			// this contains the blend bit and the priority bits, the spbactn proto uses 0x0300 for priority, spbactn uses 0x0030, others use 0x00c0
			color |= (source[attributes_word] & 0x03f0);
			bitmap = &sprite_bitmap;
	

			for (row = 0; row < sizey; row++)
			{
				for (col = 0; col < sizex; col++)
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

