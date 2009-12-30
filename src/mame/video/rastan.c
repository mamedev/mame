/***************************************************************************
  Functions to emulate similar video hardware on these Taito games:

  - rastan
  - operation wolf
  - rainbow islands
  - jumping (bootleg)

***************************************************************************/

#include "driver.h"
#include "video/taitoic.h"


static UINT16 sprite_ctrl = 0;
static UINT16 sprites_flipscreen = 0;

/***************************************************************************/

VIDEO_START( jumping )
{
	const device_config *pc080sn = devtag_get_device(machine, "pc080sn");

	pc080sn_set_trans_pen(pc080sn, 1, 15);

	/* not 100% sure Jumping needs to save both... */
	state_save_register_global(machine, sprite_ctrl);
	state_save_register_global(machine, sprites_flipscreen);
}


WRITE16_HANDLER( rastan_spritectrl_w )
{
	const device_config *pc090oj = devtag_get_device(space->machine, "pc090oj");

	/* bits 5-7 are the sprite palette bank */
	pc090oj_set_sprite_ctrl(pc090oj, (data & 0xe0) >> 5);

	/* bit 4 unused */

	/* bits 0 and 1 are coin lockout */
	coin_lockout_w(space->machine, 1,~data & 0x01);
	coin_lockout_w(space->machine, 0,~data & 0x02);

	/* bits 2 and 3 are the coin counters */
	coin_counter_w(space->machine, 1,data & 0x04);
	coin_counter_w(space->machine, 0,data & 0x08);
}

WRITE16_HANDLER( rainbow_spritectrl_w )
{
	const device_config *pc090oj = devtag_get_device(space->machine, "pc090oj");
	if (offset == 0)
	{
		/* bits 0 and 1 always set */
		/* bits 5-7 are the sprite palette bank */
		/* other bits unknown */

		pc090oj_set_sprite_ctrl(pc090oj, (data & 0xe0) >> 5);
	}
}

WRITE16_HANDLER( jumping_spritectrl_w )
{
	if (offset == 0)
	{
		/* bits 0 and 1 are set after 15 seconds */
		/* bits 5-7 are the sprite palette bank */
		/* other bits unknown */

		sprite_ctrl = data;
	}
}



/***************************************************************************/

VIDEO_UPDATE( rastan )
{
	const device_config *pc080sn = devtag_get_device(screen->machine, "pc080sn");
	const device_config *pc090oj = devtag_get_device(screen->machine, "pc090oj");
	int layer[2];

	pc080sn_tilemap_update(pc080sn);

	layer[0] = 0;
	layer[1] = 1;

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	pc080sn_tilemap_draw(pc080sn, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);
	pc080sn_tilemap_draw(pc080sn, bitmap, cliprect, layer[1], 0, 2);

	pc090oj_draw_sprites(pc090oj, bitmap, cliprect, 0);
	return 0;
}

/***************************************************************************/

VIDEO_UPDATE( opwolf )
{
	const device_config *pc080sn = devtag_get_device(screen->machine, "pc080sn");
	const device_config *pc090oj = devtag_get_device(screen->machine, "pc090oj");
	int layer[2];

	pc080sn_tilemap_update(pc080sn);

	layer[0] = 0;
	layer[1] = 1;

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	pc080sn_tilemap_draw(pc080sn, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);
	pc080sn_tilemap_draw(pc080sn, bitmap, cliprect, layer[1], 0, 2);

	pc090oj_draw_sprites(pc090oj, bitmap, cliprect, 1);

//  if (input_port_read(machine, "P1X"))
//  popmessage("%d %d", input_port_read(machine, "P1X"), input_port_read(machine, "P1Y"));

	return 0;
}

/***************************************************************************/

VIDEO_UPDATE( rainbow )
{
	const device_config *pc080sn = devtag_get_device(screen->machine, "pc080sn");
	const device_config *pc090oj = devtag_get_device(screen->machine, "pc090oj");
	int layer[2];

	pc080sn_tilemap_update(pc080sn);

	layer[0] = 0;
	layer[1] = 1;

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	pc080sn_tilemap_draw(pc080sn, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);
	pc080sn_tilemap_draw(pc080sn, bitmap, cliprect, layer[1], 0, 2);

	pc090oj_draw_sprites(pc090oj, bitmap, cliprect, 1);
	return 0;
}

/***************************************************************************

Jumping uses different sprite controller
than rainbow island. - values are remapped
at address 0x2EA in the code. Apart from
physical layout, the main change is that
the Y settings are active low.

*/

VIDEO_UPDATE( jumping )
{
	const device_config *pc080sn = devtag_get_device(screen->machine, "pc080sn");
	UINT16 *spriteram16 = screen->machine->generic.spriteram.u16;
	int offs, layer[2];
	int sprite_colbank = (sprite_ctrl & 0xe0) >> 1;

	pc080sn_tilemap_update(pc080sn);

	/* Override values, or foreground layer is in wrong position */
	pc080sn_set_scroll(pc080sn, 1, 16, 0);

	layer[0] = 0;
	layer[1] = 1;

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);

	pc080sn_tilemap_draw(pc080sn, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 0);

	/* Draw the sprites. 128 sprites in total */
	for (offs = screen->machine->generic.spriteram_size / 2 - 8; offs >= 0; offs -= 8)
	{
		int tile = spriteram16[offs];
		if (tile < screen->machine->gfx[1]->total_elements)
		{
			int sx,sy,color,data1;

			sy = ((spriteram16[offs+1] - 0xfff1) ^ 0xffff) & 0x1ff;
			if (sy > 400) sy = sy - 512;
			sx = (spriteram16[offs+2] - 0x38) & 0x1ff;
			if (sx > 400) sx = sx - 512;

			data1 = spriteram16[offs+3];
			color = (spriteram16[offs+4] & 0x0f) | sprite_colbank;

			drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[0],
					tile,
					color,
					data1 & 0x40, data1 & 0x80,
					sx,sy+1,15);
		}
	}

	pc080sn_tilemap_draw(pc080sn, bitmap, cliprect, layer[1], 0, 0);

#if 0
	{
		char buf[80];
		sprintf(buf,"sprite_ctrl: %04x",sprite_ctrl);
		popmessage(buf);
	}
#endif
	return 0;
}

