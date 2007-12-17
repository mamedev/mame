#include "driver.h"
#include "deco16ic.h"

static void draw_sprites(running_machine* machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs;

	for (offs = 0;offs < 0x400;offs += 4)
	{
		int x,y,sprite,colour,multi,fx,fy,inc,flash,mult;

		sprite = spriteram16[offs+1];
		if (!sprite) continue;

		y = spriteram16[offs];
		flash=y&0x1000;
		if (flash && (cpu_getcurrentframe() & 1)) continue;

		x = spriteram16[offs+2];
		colour = (x >>9) & 0x1f;

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
			drawgfx(bitmap,machine->gfx[2],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,
					cliprect,TRANSPARENCY_PEN,0);

			multi--;
		}
	}
}

static int dietgo_bank_callback(const int bank)
{
	return ((bank>>4)&0x7) * 0x1000;
}

VIDEO_START(dietgo)
{
	deco16_1_video_init();

	deco16_set_tilemap_bank_callback(0, dietgo_bank_callback);
	deco16_set_tilemap_bank_callback(1, dietgo_bank_callback);
}

VIDEO_UPDATE(dietgo)
{
	flip_screen_set( deco16_pf12_control[0]&0x80 );
	deco16_pf12_update(deco16_pf1_rowscroll,deco16_pf2_rowscroll);

	fillbitmap(bitmap,machine->pens[256],cliprect); /* not verified */

	deco16_tilemap_2_draw(bitmap,cliprect,TILEMAP_DRAW_OPAQUE,0);
	deco16_tilemap_1_draw(bitmap,cliprect,0,0);

	draw_sprites(machine,bitmap,cliprect);
	return 0;
}
