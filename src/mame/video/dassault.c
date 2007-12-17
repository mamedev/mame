/***************************************************************************

   Desert Assault Video emulation - Bryan McPhail, mish@tendril.co.uk

****************************************************************************/

#include "driver.h"
#include "deco16ic.h"

/******************************************************************************/

static void draw_sprites(running_machine* machine, mame_bitmap *bitmap, const rectangle *cliprect, int pf_priority)
{
	int x,y,sprite,colour,multi,fx,fy,inc,flash,mult;
	int offs, bank, gfxbank;
	const UINT16 *spritebase;

	/* Have to loop over the two sprite sources */
	for (bank=0; bank<2; bank++)
	{
		for (offs = 0x800-4;offs >= 0; offs -= 4)
		{
			int trans=TRANSPARENCY_PEN, pmask=0;

			/* Draw the main spritebank after the other one */
			if (bank==0)
			{
				spritebase=buffered_spriteram16;
				gfxbank=3;
			}
			else
			{
				spritebase=buffered_spriteram16_2;
				gfxbank=4;
			}

			sprite = spritebase[offs+1] & 0x7fff;
			if (!sprite) continue;

			x = spritebase[offs+2];

			/* Alpha on chip 2 only */
			if (bank==1 && x&0xc000)
				trans=TRANSPARENCY_ALPHA;

			y = spritebase[offs];
			flash=y&0x1000;
			if (flash && (cpu_getcurrentframe() & 1)) continue;
			colour = (x >> 9) &0x1f;
			if (y&0x8000) colour+=32;

			fx = y & 0x2000;
			fy = y & 0x4000;
			multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

			x = x & 0x01ff;
			y = y & 0x01ff;
			if (x >= 320) x -= 512;
			if (y >= 256) y -= 512;
			x = 304 - x;
			y = 240 - y;

			if (x>320) continue; /* Speedup */

			sprite &= ~multi;
			if (fy)
				inc = -1;
			else
			{
				sprite += multi;
				inc = 1;
			}

			if (flip_screen) {
				y=240-y;
				x=304-x;
				if (fx) fx=0; else fx=1;
				if (fy) fy=0; else fy=1;
				mult=16;
			}
			else mult=-16;

			/* Priority */
			switch (pf_priority&3) {
			case 0:
				if (bank==0) {
					switch (spritebase[offs+2]&0xc000) {
					case 0xc000: pmask=1; break;
					case 0x8000: pmask=8; break;
					case 0x4000: pmask=32; break;
					case 0x0000: pmask=128; break;
					}
				} else {
					if (spritebase[offs+2]&0x8000) pmask=64; /* Check */
					else pmask=64;
				}
				break;

			case 1:
				if (bank==0) {
					switch (spritebase[offs+2]&0xc000) {
					case 0xc000: pmask=1; break;
					case 0x8000: pmask=8; break;
					case 0x4000: pmask=32; break;
					case 0x0000: pmask=128; break;
					}
				} else {
					if (spritebase[offs+2]&0x8000) pmask=16; /* Check */
					else pmask=16;
				}
				break;

			case 2: /* Unused */
			case 3:
				if (bank==0) {
					switch (spritebase[offs+2]&0xc000) {
					case 0xc000: pmask=1; break;
					case 0x8000: pmask=8; break;
					case 0x4000: pmask=32; break;
					case 0x0000: pmask=128; break;
					}
				} else {
					if (spritebase[offs+2]&0x8000) pmask=64; /* Check */
					else pmask=64;
				}
				break;
			}

			while (multi >= 0)
			{
				deco16_pdrawgfx(bitmap,machine->gfx[gfxbank],
						sprite - multi * inc,
						colour,
						fx,fy,
						x,y + mult * multi,
						cliprect,trans,0,pmask,1<<bank, 1);

				multi--;
			}
		}
	}
}

/******************************************************************************/

static int dassault_bank_callback(const int bank)
{
	return ((bank>>4)&0xf)<<12;
}

VIDEO_START( dassault )
{
	deco16_2_video_init(0);

	deco16_set_tilemap_bank_callback(0,dassault_bank_callback);
	deco16_set_tilemap_bank_callback(1,dassault_bank_callback);
	deco16_set_tilemap_bank_callback(2,dassault_bank_callback);
	deco16_set_tilemap_bank_callback(3,dassault_bank_callback);

	alpha_set_level(0x80);
}

/******************************************************************************/

VIDEO_UPDATE( dassault )
{
	/* Update tilemaps */
	flip_screen_set( deco16_pf12_control[0]&0x80 );
	deco16_pf12_update(deco16_pf1_rowscroll,deco16_pf2_rowscroll);
	deco16_pf34_update(deco16_pf3_rowscroll,deco16_pf4_rowscroll);

	/* Draw playfields/update priority bitmap */
	deco16_clear_sprite_priority_bitmap();
	fillbitmap(priority_bitmap,0,cliprect);
	fillbitmap(bitmap,machine->pens[3072],cliprect);
	deco16_tilemap_4_draw(bitmap,cliprect,TILEMAP_DRAW_OPAQUE,0);

	/* The middle playfields can be swapped priority-wise */
	if ((deco16_priority&3)==0) {
		deco16_tilemap_2_draw(bitmap,cliprect,0,2);
		deco16_tilemap_3_draw(bitmap,cliprect,0,16);
	} else if ((deco16_priority&3)==1) {
		deco16_tilemap_3_draw(bitmap,cliprect,0,2);
		deco16_tilemap_2_draw(bitmap,cliprect,0,64);
	} else if ((deco16_priority&3)==3) {
		deco16_tilemap_3_draw(bitmap,cliprect,0,2);
		deco16_tilemap_2_draw(bitmap,cliprect,0,16);
	} else {
		/* Unused */
	}

	/* Draw sprites - two sprite generators, with selectable priority */
	draw_sprites(machine,bitmap,cliprect,deco16_priority);
	deco16_tilemap_1_draw(bitmap,cliprect,0,0);
	return 0;
}
