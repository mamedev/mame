/*************************************************************************

    Atari Football hardware

*************************************************************************/

#include "driver.h"
#include "atarifb.h"
#include "ui.h"

/* local */
size_t atarifb_alphap1_vram_size;
size_t atarifb_alphap2_vram_size;
UINT8 *atarifb_alphap1_vram;
UINT8 *atarifb_alphap2_vram;
UINT8 *atarifb_scroll_register;


static const rectangle bigfield_area = {  4*8, 34*8-1, 0*8, 32*8-1 };
static const rectangle left_area =     {  0*8,  3*8-1, 0*8, 32*8-1 };
static const rectangle right_area =    { 34*8, 38*8-1, 0*8, 32*8-1 };

/***************************************************************************
***************************************************************************/
WRITE8_HANDLER( atarifb_alphap1_vram_w )
{
	atarifb_alphap1_vram[offset] = data;
}

WRITE8_HANDLER( atarifb_alphap2_vram_w )
{
	atarifb_alphap2_vram[offset] = data;
}

/***************************************************************************
***************************************************************************/
WRITE8_HANDLER( atarifb_scroll_w )
{
	*atarifb_scroll_register = data - 8;
}


VIDEO_UPDATE( atarifb )
{
	int offs,obj;
	int sprite_bank;

	/* Soccer uses a different graphics set for sprites */
	if (atarifb_game == 4)
		sprite_bank = 2;
	else
		sprite_bank = 1;

	for (offs = atarifb_alphap1_vram_size - 1;offs >= 0;offs--)
	{
		int charcode;
		int flipbit;
		int disable;
		int sx,sy;

		sx = 8 * (offs / 32) + 35*8;
		sy = 8 * (offs % 32) + 8;

		charcode = atarifb_alphap1_vram[offs] & 0x3f;
		flipbit = (atarifb_alphap1_vram[offs] & 0x40) >> 6;
		disable = (atarifb_alphap1_vram[offs] & 0x80) >> 7;

		if (!disable)
		{
			drawgfx(bitmap,machine->gfx[0],
				charcode, 0,
				flipbit,flipbit,sx,sy,
				&right_area,TRANSPARENCY_NONE,0);
		}
	}

	for (offs = atarifb_alphap2_vram_size - 1;offs >= 0;offs--)
	{
		int charcode;
		int flipbit;
		int disable;
		int sx,sy;

		sx = 8 * (offs / 32);
		sy = 8 * (offs % 32) + 8;

		charcode = atarifb_alphap2_vram[offs] & 0x3f;
		flipbit = (atarifb_alphap2_vram[offs] & 0x40) >> 6;
		disable = (atarifb_alphap2_vram[offs] & 0x80) >> 7;

		if (!disable)
		{
			drawgfx(bitmap,machine->gfx[0],
				charcode, 0,
				flipbit,flipbit,sx,sy,
				&left_area,TRANSPARENCY_NONE,0);
		}
	}

	for (offs = videoram_size - 1;offs >= 0;offs--)
	{
		int charcode;
		int flipx,flipy;
		int sx,sy;

		dirtybuffer[offs]=0;

		charcode = videoram[offs] & 0x3f;
		flipx = (videoram[offs] & 0x40) >> 6;
		flipy = (videoram[offs] & 0x80) >> 7;

		sx = (8 * (offs % 32) - *atarifb_scroll_register);
		sy = 8 * (offs / 32) + 8;

		/* Soccer hack */
		if (atarifb_game == 4)
		{
			sy += 8;
		}

		/* Baseball hack */
		if (atarifb_game == 0x03) sx -= 8;

		if (sx < 0) sx += 256;
		if (sx > 255) sx -= 256;

		drawgfx(tmpbitmap,machine->gfx[1],
				charcode, 0,
				flipx,flipy,sx,sy,
				0,TRANSPARENCY_NONE,0);
	}

	/* copy the character mapped graphics */
	copybitmap(bitmap,tmpbitmap,0,0,8*3,0,&bigfield_area,TRANSPARENCY_NONE,0);

	/* Draw our motion objects */
	for (obj=0;obj<16;obj++)
	{
		int charcode;
		int flipx,flipy;
		int sx,sy;
		int shade = 0;

		sy = 255 - spriteram[obj*2 + 1];
		if (sy == 255) continue;

		charcode = spriteram[obj*2] & 0x3f;
		flipx = (spriteram[obj*2] & 0x40);
		flipy = (spriteram[obj*2] & 0x80);
		sx = spriteram[obj*2 + 0x20] + 8*3;

		/* Note on Atari Soccer: */
		/* There are 3 sets of 2 bits each, where the 2 bits represent */
		/* black, dk grey, grey and white. I think the 3 sets determine the */
		/* color of each bit in the sprite, but I haven't implemented it that way. */
		if (atarifb_game == 4)
		{
			shade = ((spriteram[obj*2+1 + 0x20]) & 0x07);

			drawgfx(bitmap,machine->gfx[sprite_bank+1],
				charcode, shade,
				flipx,flipy,sx,sy,
				&bigfield_area,TRANSPARENCY_PEN,0);

			shade = ((spriteram[obj*2+1 + 0x20]) & 0x08) >> 3;
		}

		drawgfx(bitmap,machine->gfx[sprite_bank],
				charcode, shade,
				flipx,flipy,sx,sy,
				&bigfield_area,TRANSPARENCY_PEN,0);

		/* If this isn't soccer, handle the multiplexed sprites */
		if (atarifb_game != 4)
		{
			/* The down markers are multiplexed by altering the y location during */
			/* mid-screen. We'll fake it by essentially doing the same thing here. */
			if ((charcode == 0x11) && (sy == 0x07))
			{
				sy = 0xf1; /* When multiplexed, it's 0x10...why? */
				drawgfx(bitmap,machine->gfx[sprite_bank],
					charcode, 0,
					flipx,flipy,sx,sy,
					&bigfield_area,TRANSPARENCY_PEN,0);
			}
		}
	}

	return 0;
}
