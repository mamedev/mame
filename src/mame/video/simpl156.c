/* Simple 156 based board

*/

#include "driver.h"
#include "deco16ic.h"

/*

offs +0
-------- --------
 fFb ssy yyyyyyyy

s = size (multipart)
f = flipy
b = flash
F = flipx
y = ypos

offs +1
-------- --------
tttttttt tttttttt

t = sprite tile

offs +2
-------- --------
ppcccccx xxxxxxxx

c = colour palette
p = priority
x = xpos

*/

/* spriteram is really 16-bit.. this can be changed to use 16-bit ram like the tilemaps
 its the same sprite chip Data East used on many, many 16-bit era titles */
static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs;

	flip_screen = 1;

	for (offs = (0x1400/4)-4;offs >= 0;offs -= 4) // 0x1400 for charlien
	{
		int x,y,sprite,colour,multi,fx,fy,inc,flash,mult, pri;

		sprite = spriteram32[offs+1]&0xffff;

		y = spriteram32[offs]&0xffff;
		flash=y&0x1000;
		if (flash && (cpu_getcurrentframe() & 1)) continue;

		x = spriteram32[offs+2]&0xffff;
		colour = (x >>9) & 0x1f;

		pri = (x&0xc000); // 2 bits or 1?

		switch (pri&0xc000) {
		case 0x0000: pri=0; break;
		case 0x4000: pri=0xf0; break;
		case 0x8000: pri=0xf0|0xcc; break;
		case 0xc000: pri=0xf0|0xcc; break; /*  or 0xf0|0xcc|0xaa ? */
		}

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;
		y = 240 - y;
        x = 304 - x;

		if (x>320) continue;

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
			x=304-x;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
			mult=16;
		}
		else mult=-16;

		while (multi >= 0)
		{
			pdrawgfx(bitmap,machine->gfx[2],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,
					cliprect,TRANSPARENCY_PEN,0,pri);

			multi--;
		}
	}
}


VIDEO_UPDATE( simpl156 )
{
	fillbitmap(priority_bitmap,0,NULL);

	deco16_pf12_update(deco16_pf1_rowscroll,deco16_pf2_rowscroll);

	fillbitmap(bitmap,machine->pens[256],cliprect);

	deco16_tilemap_2_draw(bitmap,cliprect,0,2);
	deco16_tilemap_1_draw(bitmap,cliprect,0,4);

	draw_sprites(machine, bitmap,cliprect);
	return 0;
}

static int simpl156_bank_callback(const int bank)
{
	return ((bank>>4)&0x7) * 0x1000;
}


VIDEO_START( simpl156 )
{
	/* allocate the ram as 16-bit (we do it here because the CPU is 32-bit) */
	deco16_pf1_data = auto_malloc(0x2000);
	deco16_pf2_data = auto_malloc(0x2000);
	deco16_pf1_rowscroll = auto_malloc(0x800);
	deco16_pf2_rowscroll = auto_malloc(0x800);
	deco16_pf12_control = auto_malloc(0x10);
	paletteram16 =  auto_malloc(0x1000);

	/* and register the allocated ram so that save states still work */
	state_save_register_global_pointer(deco16_pf1_data, 0x2000/2);
	state_save_register_global_pointer(deco16_pf2_data, 0x2000/2);
	state_save_register_global_pointer(deco16_pf1_rowscroll, 0x800/2);
	state_save_register_global_pointer(deco16_pf2_rowscroll, 0x800/2);
	state_save_register_global_pointer(deco16_pf12_control, 0x10/2);
	state_save_register_global_pointer(paletteram16, 0x1000/2);

	deco16_1_video_init();

	deco16_set_tilemap_bank_callback(0, simpl156_bank_callback);
	deco16_set_tilemap_bank_callback(1, simpl156_bank_callback);
}
