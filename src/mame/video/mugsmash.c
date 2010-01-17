/* video/mugsmash.c - see drivers/mugsmash.c for more info */

#include "emu.h"
#include "includes/mugsmash.h"

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{

	/* Each Sprite takes 16 bytes, 5 used? */

	/* ---- ----  xxxx xxxx  ---- ----  aaaa aaaa  ---- ----  NNNN NNNN  ---- ----  nnnn nnnn  ---- ----  yyyy yyyy (rest unused?) */

	/* x = xpos LSB
       y = ypos LSB
       N = tile number MSB
       n = tile number LSB
       a = attribute / extra
            f?XY cccc

        f = x-flip
        ? = unknown, probably y-flip
        X = xpos MSB
        y = ypos MSB
        c = colour

    */

	mugsmash_state *state = (mugsmash_state *)machine->driver_data;
	const UINT16 *source = state->spriteram;
	const UINT16 *finish = source+0x2000;
	const gfx_element *gfx = machine->gfx[0];

	while( source<finish )
	{
		int xpos = source[0] & 0x00ff;
		int ypos = source[4] & 0x00ff;
		int num = (source[3] & 0x00ff) | ((source[2] & 0x00ff) << 8);
		int attr = source[1];
		int flipx = (attr & 0x0080)>>7;
		int colour = (attr & 0x000f);

		xpos += ((attr & 0x0020) >> 5) * 0x100;
		ypos += ((attr & 0x0010) >> 4) * 0x100;

		xpos -= 28;
		ypos -= 16;

		drawgfx_transpen(
				bitmap,
				cliprect,
				gfx,
				num,
				colour,
				flipx,0,
				xpos,ypos,0
				);

		source += 0x8;
	}
}

static TILE_GET_INFO( get_mugsmash_tile_info1 )
{

	/* fF-- cccc  nnnn nnnn */

	/* c = colour?
       n = number?
       F = flip-X
       f = flip-Y
    */

	mugsmash_state *state = (mugsmash_state *)machine->driver_data;
	int tileno,colour,fx;

	tileno = state->videoram1[tile_index *2 +1];
	colour = state->videoram1[tile_index *2] & 0x000f;
	fx = (state->videoram1[tile_index *2] & 0xc0) >>6;

	SET_TILE_INFO(1,tileno,colour,TILE_FLIPYX(fx));
}

WRITE16_HANDLER( mugsmash_videoram1_w )
{
	mugsmash_state *state = (mugsmash_state *)space->machine->driver_data;

	state->videoram1[offset] = data;
	tilemap_mark_tile_dirty(state->tilemap1,offset/2);
}

static TILE_GET_INFO( get_mugsmash_tile_info2 )
{

	/* fF-- cccc  nnnn nnnn */

	/* c = colour?
       n = number?
       F = flip-X
       f = flip-Y
    */

	mugsmash_state *state = (mugsmash_state *)machine->driver_data;
	int tileno,colour,fx;

	tileno = state->videoram2[tile_index *2 +1];
	colour = state->videoram2[tile_index *2] & 0x000f;
	fx = (state->videoram2[tile_index *2] & 0xc0) >>6;

	SET_TILE_INFO(1,tileno,16+colour,TILE_FLIPYX(fx));
}

WRITE16_HANDLER( mugsmash_videoram2_w )
{
	mugsmash_state *state = (mugsmash_state *)space->machine->driver_data;

	state->videoram2[offset] = data;
	tilemap_mark_tile_dirty(state->tilemap2,offset/2);
}

WRITE16_HANDLER (mugsmash_reg_w)
{
	mugsmash_state *state = (mugsmash_state *)space->machine->driver_data;

	state->regs1[offset] = data;
//  popmessage ("Regs %04x, %04x, %04x, %04x", mugsmash_regs1[0], mugsmash_regs1[1],mugsmash_regs1[2], mugsmash_regs1[3]);

	switch (offset)
	{
	case 0:
		tilemap_set_scrollx(state->tilemap2,0, state->regs1[2]+7);
		break;
	case 1:
		tilemap_set_scrolly(state->tilemap2,0, state->regs1[3]+4);
		break;
	case 2:
		tilemap_set_scrollx(state->tilemap1,0, state->regs1[0]+3);
		break;
	case 3:
		tilemap_set_scrolly(state->tilemap1,0, state->regs1[1]+4);
		break;
	}
}

VIDEO_START( mugsmash )
{
	mugsmash_state *state = (mugsmash_state *)machine->driver_data;

	state->tilemap1 = tilemap_create(machine, get_mugsmash_tile_info1,tilemap_scan_rows, 16, 16,32,32);
	tilemap_set_transparent_pen(state->tilemap1,0);

	state->tilemap2 = tilemap_create(machine, get_mugsmash_tile_info2,tilemap_scan_rows, 16, 16,32,32);
}

VIDEO_UPDATE( mugsmash )
{
	mugsmash_state *state = (mugsmash_state *)screen->machine->driver_data;

	tilemap_draw(bitmap,cliprect,state->tilemap2,0,0);
	tilemap_draw(bitmap,cliprect,state->tilemap1,0,0);
	draw_sprites(screen->machine,bitmap,cliprect);
	return 0;
}
