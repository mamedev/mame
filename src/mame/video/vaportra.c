/***************************************************************************

   Vapour Trail Video emulation - Bryan McPhail, mish@tendril.co.uk

****************************************************************************

    2 Data East 55 chips for playfields (same as Dark Seal, etc)
    1 Data East MXC-06 chip for sprites (same as Bad Dudes, etc)

***************************************************************************/

#include "driver.h"
#include "deco16ic.h"

static UINT16 vaportra_priority[2];

/******************************************************************************/

static int vaportra_bank_callback(const int bank)
{
	return ((bank>>4)&0x7) * 0x1000;
}


VIDEO_START( vaportra )
{
	deco16_2_video_init(0);

	deco16_pf1_rowscroll = 0;
	deco16_pf2_rowscroll = 0;
	deco16_pf3_rowscroll = 0;
	deco16_pf4_rowscroll = 0;

	deco16_set_tilemap_bank_callback(0, vaportra_bank_callback);
	deco16_set_tilemap_bank_callback(1, vaportra_bank_callback);
	deco16_set_tilemap_bank_callback(2, vaportra_bank_callback);
	deco16_set_tilemap_bank_callback(3, vaportra_bank_callback);

	deco16_pf1_colour_bank=0x00;
	deco16_pf2_colour_bank=0x20;
	deco16_pf4_colour_bank=0x40;
	deco16_pf3_colour_bank=0x30;
}

/******************************************************************************/


WRITE16_HANDLER( vaportra_priority_w )
{
	COMBINE_DATA(&vaportra_priority[offset]);
}

/******************************************************************************/

static void update_24bitcol(int offset)
{
	UINT8 r,g,b;

	r = (paletteram16[offset] >> 0) & 0xff;
	g = (paletteram16[offset] >> 8) & 0xff;
	b = (paletteram16_2[offset] >> 0) & 0xff;

	palette_set_color(Machine,offset,MAKE_RGB(r,g,b));
}

WRITE16_HANDLER( vaportra_palette_24bit_rg_w )
{
	COMBINE_DATA(&paletteram16[offset]);
	update_24bitcol(offset);
}

WRITE16_HANDLER( vaportra_palette_24bit_b_w )
{
	COMBINE_DATA(&paletteram16_2[offset]);
	update_24bitcol(offset);
}

/******************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int pri)
{
	int offs,priority_value;

	priority_value=vaportra_priority[1];

	for (offs = 0;offs < 0x400;offs += 4)
	{
		int x,y,sprite,colour,multi,fx,fy,inc,flash,mult;

		y = buffered_spriteram16[offs];
		if ((y&0x8000) == 0) continue;

		sprite = buffered_spriteram16[offs+1] & 0x1fff;
		x = buffered_spriteram16[offs+2];
		colour = (x >> 12) &0xf;
		if (pri && (colour>=priority_value)) continue;
		if (!pri && !(colour>=priority_value)) continue;

		flash=x&0x800;
		if (flash && (cpu_getcurrentframe() & 1)) continue;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x1800) >> 11)) - 1;	/* 1x, 2x, 4x, 8x height */

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 256) x -= 512;
		if (y >= 256) y -= 512;
		x = 240 - x;
		y = 240 - y;

		if (x>256) continue; /* Speedup */

		sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (flip_screen)
		{
			y=240-y;
			x=240-x;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
			mult=16;
		}
		else mult=-16;

		while (multi >= 0)
		{
			drawgfx(bitmap,machine->gfx[3],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,
					cliprect,TRANSPARENCY_PEN,0);

			multi--;
		}
	}
}


VIDEO_UPDATE( vaportra )
{
	int pri=vaportra_priority[0] & 0x03;

	flip_screen_set( !(deco16_pf12_control[0]&0x80) );

	deco16_pf12_update(deco16_pf1_rowscroll,deco16_pf2_rowscroll);
	deco16_pf34_update(deco16_pf3_rowscroll,deco16_pf4_rowscroll);

	/* Draw playfields */
	if (pri==0) {
		deco16_tilemap_4_draw(bitmap,cliprect,TILEMAP_DRAW_OPAQUE,0);
		deco16_tilemap_3_draw(bitmap,cliprect,0,0);
		draw_sprites(machine, bitmap,cliprect,0);
		deco16_tilemap_2_draw(bitmap,cliprect,0,0);
	}
	else if (pri==1) {
		deco16_tilemap_3_draw(bitmap,cliprect,TILEMAP_DRAW_OPAQUE,0);
		deco16_tilemap_4_draw(bitmap,cliprect,0,0);
		draw_sprites(machine, bitmap,cliprect,0);
		deco16_tilemap_2_draw(bitmap,cliprect,0,0);
	}
	else if (pri==2) {
		deco16_tilemap_4_draw(bitmap,cliprect,TILEMAP_DRAW_OPAQUE,0);
		deco16_tilemap_2_draw(bitmap,cliprect,0,0);
		draw_sprites(machine, bitmap,cliprect,0);
		deco16_tilemap_3_draw(bitmap,cliprect,0,0);
	}
	else {
		deco16_tilemap_3_draw(bitmap,cliprect,TILEMAP_DRAW_OPAQUE,0);
		deco16_tilemap_2_draw(bitmap,cliprect,0,0);
		draw_sprites(machine, bitmap,cliprect,0);
		deco16_tilemap_4_draw(bitmap,cliprect,0,0);
	}

	draw_sprites(machine,bitmap,cliprect,1);
	deco16_tilemap_1_draw(bitmap,cliprect,0,0);
	return 0;
}
