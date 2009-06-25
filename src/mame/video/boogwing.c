#include "driver.h"
#include "deco16ic.h"

static void draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect, UINT16* spriteram_base, int gfx_region)
{
	int offs;
	int flipscreen=!flip_screen_get(machine);

	for (offs = 0x400-4;offs >= 0;offs -= 4)
	{
		int x,y,sprite,colour,multi,fx,fy,inc,flash,mult,pri=0,spri=0;
		int alpha = 0xff;

		sprite = spriteram_base[offs+1];
		if (!sprite) continue;

		y = spriteram_base[offs];
		flash=y&0x1000;
		if (flash && (video_screen_get_frame_number(machine->primary_screen) & 1)) continue;

		x = spriteram_base[offs+2];
		colour = (x >>9) & 0x1f;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

		// Todo:  This should be verified from the prom
		if (gfx_region==4)
		{
			// Sprite 2 priority vs sprite 1
			if ((spriteram_base[offs+2]&0xc000)==0xc000)
				spri=4;
			else if ((spriteram_base[offs+2]&0xc000))
				spri=16;
			else
				spri=64;

			// Transparency
			if (spriteram_base[offs+2]&0x2000)
				alpha = 0x80;

			if (deco16_priority==0x2)
			{
				// Additional sprite alpha in this mode
				if (spriteram_base[offs+2]&0x8000)
					alpha = 0x80;

				// Sprite vs playfield
				if ((spriteram_base[offs+2]&0xc000)==0xc000)
					pri=4;
				else if ((spriteram_base[offs+2]&0xc000)==0x8000)
					pri=16;
				else
					pri=64;
			}
			else
			{
				if ((spriteram_base[offs+2]&0x8000)==0x8000)
					pri=16;
				else
					pri=64;
			}
		}
		else
		{
			// Sprite 1 priority vs sprite 2
			if (spriteram_base[offs+2]&0x8000)		// todo - check only in pri mode 2??
				spri=8;
			else
				spri=32;

			// Sprite vs playfield
			if (deco16_priority==0x1)
			{
				if ((spriteram_base[offs+2]&0xc000))
					pri=16;
				else
					pri=64;
			}
			else
			{
				if ((spriteram_base[offs+2]&0xc000)==0xc000)
					pri=4;
				else if ((spriteram_base[offs+2]&0xc000)==0x8000)
					pri=16;
				else
					pri=64;
			}
		}

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;
		y = 240 - y;
        x = 304 - x;

		sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (flipscreen)
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
			deco16_pdrawgfx(
					bitmap, cliprect, machine->gfx[gfx_region],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,
					0, pri, spri, 0, alpha);

			multi--;
		}
	}
}

static int boogwing_bank_callback(const int bank)
{
	return ((bank>>4) & 0x7) * 0x1000;
}

static int boogwing_bank_callback2(const int bank)
{
	int offset=((bank>>4) & 0x7) * 0x1000;
	if ((bank&0xf)==0xa)
		offset+=0x800; // strange - transporter level
	return offset;
}

VIDEO_START(boogwing)
{
	deco16_2_video_init(machine, 0);

	deco16_set_tilemap_bank_callback(1,boogwing_bank_callback);
	deco16_set_tilemap_bank_callback(2,boogwing_bank_callback2);
	deco16_set_tilemap_bank_callback(3,boogwing_bank_callback2);
	deco16_set_tilemap_colour_base(1,0);
	deco16_set_tilemap_transparency_mask(1, 0x1f); // 5bpp graphics
}

VIDEO_UPDATE(boogwing)
{
	flip_screen_set(screen->machine,  deco16_pf12_control[0]&0x80 );
	deco16_pf12_update(deco16_pf1_rowscroll,deco16_pf2_rowscroll);
	deco16_pf34_update(deco16_pf3_rowscroll,deco16_pf4_rowscroll);

	/* Draw playfields */
	deco16_clear_sprite_priority_bitmap();
	bitmap_fill(bitmap,cliprect,screen->machine->pens[0x400]); /* pen not confirmed */
	bitmap_fill(priority_bitmap,NULL,0);

	// bit&0x8 is definitely some kind of palette effect
	// bit&0x4 combines playfields
	if ((deco16_priority&0x7)==0x5)
	{
		deco16_tilemap_2_draw(screen,bitmap,cliprect,TILEMAP_DRAW_OPAQUE,0);
		deco16_tilemap_34_combine_draw(screen,bitmap,cliprect,0,32);
	}
	else if ((deco16_priority&0x7)==0x1 || (deco16_priority&0x7)==0x2)
	{
		deco16_tilemap_4_draw(screen,bitmap,cliprect,TILEMAP_DRAW_OPAQUE,0);
		deco16_tilemap_2_draw(screen,bitmap,cliprect,0,8);
		deco16_tilemap_3_draw(screen,bitmap,cliprect,0,32);
	}
	else if ((deco16_priority&0x7)==0x3)
	{
		deco16_tilemap_4_draw(screen,bitmap,cliprect,TILEMAP_DRAW_OPAQUE,0);
		deco16_tilemap_2_draw(screen,bitmap,cliprect,0,8);

		// This mode uses playfield 3 to shadow sprites & playfield 2 (instead of
		// regular alpha-blending, the destination is inverted).  Not yet implemented.
//      deco16_tilemap_3_draw(screen,bitmap,cliprect,TILEMAP_DRAW_ALPHA(0x80),32);
	}
	else
	{
		deco16_tilemap_4_draw(screen,bitmap,cliprect,TILEMAP_DRAW_OPAQUE,0);
		deco16_tilemap_3_draw(screen,bitmap,cliprect,0,8);
		deco16_tilemap_2_draw(screen,bitmap,cliprect,0,32);
	}

	draw_sprites(screen->machine, bitmap, cliprect, buffered_spriteram16, 3);
	draw_sprites(screen->machine, bitmap, cliprect, buffered_spriteram16_2, 4);

	deco16_tilemap_1_draw(screen,bitmap,cliprect,0,0);
	return 0;
}
