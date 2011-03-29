/* Speed Spin video, see driver file for notes */

#include "emu.h"
#include "includes/speedspn.h"


static TILE_GET_INFO( get_speedspn_tile_info )
{
	speedspn_state *state = machine.driver_data<speedspn_state>();
	int code = state->vidram[tile_index*2+1] | (state->vidram[tile_index*2] << 8);
	int attr = state->attram[tile_index^0x400];

	SET_TILE_INFO(0,code,attr & 0x3f,(attr & 0x80) ? TILE_FLIPX : 0);
}

VIDEO_START(speedspn)
{
	speedspn_state *state = machine.driver_data<speedspn_state>();
	state->vidram = auto_alloc_array(machine, UINT8, 0x1000 * 2);
	state->tilemap = tilemap_create(machine, get_speedspn_tile_info,tilemap_scan_cols, 8, 8,64,32);
}

WRITE8_HANDLER( speedspn_vidram_w )
{
	speedspn_state *state = space->machine().driver_data<speedspn_state>();
	state->vidram[offset + state->bank_vidram] = data;

	if (state->bank_vidram == 0)
		tilemap_mark_tile_dirty(state->tilemap,offset/2);
}

WRITE8_HANDLER( speedspn_attram_w )
{
	speedspn_state *state = space->machine().driver_data<speedspn_state>();
	state->attram[offset] = data;

	tilemap_mark_tile_dirty(state->tilemap,offset^0x400);
}

READ8_HANDLER( speedspn_vidram_r )
{
	speedspn_state *state = space->machine().driver_data<speedspn_state>();
	return state->vidram[offset + state->bank_vidram];
}

WRITE8_HANDLER(speedspn_banked_vidram_change)
{
	speedspn_state *state = space->machine().driver_data<speedspn_state>();
//  logerror("VidRam Bank: %04x\n", data);
	state->bank_vidram = data & 1;
	state->bank_vidram *= 0x1000;
}

WRITE8_HANDLER(speedspn_global_display_w)
{
	speedspn_state *state = space->machine().driver_data<speedspn_state>();
//  logerror("Global display: %u\n", data);
	state->display_disable = data & 1;
}


static void draw_sprites(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	speedspn_state *state = machine.driver_data<speedspn_state>();
	const gfx_element *gfx = machine.gfx[1];
	UINT8 *source = state->vidram+ 0x1000;
	UINT8 *finish = source + 0x1000;

	while( source<finish )
	{
		int xpos = source[0];
		int tileno = source[1];
		int attr = source[2];
		int ypos = source[3];
		int color;

		if (attr&0x10) xpos +=0x100;

		xpos = 0x1f8-xpos;
		tileno += ((attr & 0xe0) >> 5) * 0x100;
		color = attr & 0x0f;

		drawgfx_transpen(bitmap,cliprect,gfx,
				tileno,
				color,
				0,0,
				xpos,ypos,15);

		source +=4;
	}
}


SCREEN_UPDATE(speedspn)
{
	speedspn_state *state = screen->machine().driver_data<speedspn_state>();
	if (state->display_disable)
	{
		bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine()));
		return 0;
	}

#if 0
	{
		FILE* f;
		f = fopen("vidram.bin","wb");
		fwrite(state->vidram, 1, 0x1000 * 2, f);
		fclose(f);
	}
#endif
	tilemap_set_scrollx(state->tilemap,0, 0x100); // verify
	tilemap_draw(bitmap,cliprect,state->tilemap,0,0);
	draw_sprites(screen->machine(), bitmap,cliprect);
	return 0;
}
