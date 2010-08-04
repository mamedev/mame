/* Simple 156 based board

*/

#include "emu.h"
#include "includes/simpl156.h"
#include "video/deco16ic.h"

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
static void draw_sprites( running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect )
{
	UINT32 *spriteram = machine->generic.spriteram.u32;
	int offs;

	//FIXME: flip_screen_x should not be written!
	flip_screen_set_no_update(machine, 1);

	for (offs = (0x1400 / 4) - 4; offs >= 0; offs -= 4) // 0x1400 for charlien
	{
		int x, y, sprite, colour, multi, fx, fy, inc, flash, mult, pri;

		sprite = spriteram[offs + 1] & 0xffff;

		y = spriteram[offs] & 0xffff;
		flash = y & 0x1000;
		if (flash && (machine->primary_screen->frame_number() & 1))
			continue;

		x = spriteram[offs + 2] & 0xffff;
		colour = (x >> 9) & 0x1f;

		pri = (x & 0xc000); // 2 bits or 1?

		switch (pri & 0xc000)
		{
		case 0x0000: pri = 0; break;
		case 0x4000: pri = 0xf0; break;
		case 0x8000: pri = 0xf0 | 0xcc; break;
		case 0xc000: pri = 0xf0 | 0xcc; break; /*  or 0xf0|0xcc|0xaa ? */
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

		if (x > 320)
			continue;

		sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (flip_screen_get(machine))
		{
			y = 240 - y;
			x = 304 - x;
			if (fx) fx = 0; else fx = 1;
			if (fy) fy = 0; else fy = 1;
			mult = 16;
		}
		else
			mult = -16;

		while (multi >= 0)
		{
			pdrawgfx_transpen(bitmap,cliprect,machine->gfx[2],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,
					machine->priority_bitmap,pri,0);

			multi--;
		}
	}
}


VIDEO_START( simpl156 )
{
	simpl156_state *state = machine->driver_data<simpl156_state>();

	/* allocate the ram as 16-bit (we do it here because the CPU is 32-bit) */
	state->pf1_rowscroll = auto_alloc_array(machine, UINT16, 0x800/2);
	state->pf2_rowscroll = auto_alloc_array(machine, UINT16, 0x800/2);
	machine->generic.paletteram.u16 =  auto_alloc_array(machine, UINT16, 0x1000/2);

	/* and register the allocated ram so that save states still work */
	state_save_register_global_pointer(machine, state->pf1_rowscroll, 0x800/2);
	state_save_register_global_pointer(machine, state->pf2_rowscroll, 0x800/2);
	state_save_register_global_pointer(machine, machine->generic.paletteram.u16, 0x1000/2);
}

VIDEO_UPDATE( simpl156 )
{
	simpl156_state *state = screen->machine->driver_data<simpl156_state>();

	bitmap_fill(screen->machine->priority_bitmap, NULL, 0);

	deco16ic_pf12_update(state->deco16ic, state->pf1_rowscroll, state->pf2_rowscroll);

	bitmap_fill(bitmap, cliprect, 256);

	deco16ic_tilemap_2_draw(state->deco16ic, bitmap, cliprect, 0, 2);
	deco16ic_tilemap_1_draw(state->deco16ic, bitmap, cliprect, 0, 4);

	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
