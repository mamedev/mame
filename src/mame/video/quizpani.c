/************************************************************************

    Quiz Panicuru Fantasy video hardware

************************************************************************/

#include "emu.h"
#include "includes/quizpani.h"


static TILEMAP_MAPPER( bg_scan )
{
	/* logical (col,row) -> memory offset */
	return (row & 0x0f) + ((col & 0xff) << 4) + ((row & 0x70) << 8);
}

static TILE_GET_INFO( bg_tile_info )
{
	quizpani_state *state = machine.driver_data<quizpani_state>();
	int code = state->bg_videoram[tile_index];

	SET_TILE_INFO(
			1,
			(code & 0xfff) + (0x1000 * state->bgbank),
			code >> 12,
			0);
}

static TILE_GET_INFO( txt_tile_info )
{
	quizpani_state *state = machine.driver_data<quizpani_state>();
	int code = state->txt_videoram[tile_index];

	SET_TILE_INFO(
			0,
			(code & 0xfff) + (0x1000 * state->txtbank),
			code >> 12,
			0);
}

WRITE16_HANDLER( quizpani_bg_videoram_w )
{
	quizpani_state *state = space->machine().driver_data<quizpani_state>();
	state->bg_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE16_HANDLER( quizpani_txt_videoram_w )
{
	quizpani_state *state = space->machine().driver_data<quizpani_state>();
	state->txt_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->txt_tilemap, offset);
}

WRITE16_HANDLER( quizpani_tilesbank_w )
{
	quizpani_state *state = space->machine().driver_data<quizpani_state>();
	if (ACCESSING_BITS_0_7)
	{
		if(state->txtbank != (data & 0x30)>>4)
		{
			state->txtbank = (data & 0x30)>>4;
			tilemap_mark_all_tiles_dirty(state->txt_tilemap);
		}

		if(state->bgbank != (data & 3))
		{
			state->bgbank = data & 3;
			tilemap_mark_all_tiles_dirty(state->bg_tilemap);
		}
	}
}

VIDEO_START( quizpani )
{
	quizpani_state *state = machine.driver_data<quizpani_state>();
	state->bg_tilemap  = tilemap_create(machine, bg_tile_info, bg_scan,16,16,256,32);
	state->txt_tilemap = tilemap_create(machine, txt_tile_info,bg_scan,16,16,256,32);
	tilemap_set_transparent_pen(state->txt_tilemap,15);
}

SCREEN_UPDATE( quizpani )
{
	quizpani_state *state = screen->machine().driver_data<quizpani_state>();
	tilemap_set_scrollx(state->bg_tilemap, 0, state->scrollreg[0] - 64);
	tilemap_set_scrolly(state->bg_tilemap, 0, state->scrollreg[1] + 16);
	tilemap_set_scrollx(state->txt_tilemap, 0, state->scrollreg[2] - 64);
	tilemap_set_scrolly(state->txt_tilemap, 0, state->scrollreg[3] + 16);

	tilemap_draw(bitmap,cliprect,state->bg_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,state->txt_tilemap,0,0);
	return 0;
}
