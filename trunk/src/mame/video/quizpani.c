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
	int code = state->m_bg_videoram[tile_index];

	SET_TILE_INFO(
			1,
			(code & 0xfff) + (0x1000 * state->m_bgbank),
			code >> 12,
			0);
}

static TILE_GET_INFO( txt_tile_info )
{
	quizpani_state *state = machine.driver_data<quizpani_state>();
	int code = state->m_txt_videoram[tile_index];

	SET_TILE_INFO(
			0,
			(code & 0xfff) + (0x1000 * state->m_txtbank),
			code >> 12,
			0);
}

WRITE16_HANDLER( quizpani_bg_videoram_w )
{
	quizpani_state *state = space->machine().driver_data<quizpani_state>();
	state->m_bg_videoram[offset] = data;
	state->m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE16_HANDLER( quizpani_txt_videoram_w )
{
	quizpani_state *state = space->machine().driver_data<quizpani_state>();
	state->m_txt_videoram[offset] = data;
	state->m_txt_tilemap->mark_tile_dirty(offset);
}

WRITE16_HANDLER( quizpani_tilesbank_w )
{
	quizpani_state *state = space->machine().driver_data<quizpani_state>();
	if (ACCESSING_BITS_0_7)
	{
		if(state->m_txtbank != (data & 0x30)>>4)
		{
			state->m_txtbank = (data & 0x30)>>4;
			state->m_txt_tilemap->mark_all_dirty();
		}

		if(state->m_bgbank != (data & 3))
		{
			state->m_bgbank = data & 3;
			state->m_bg_tilemap->mark_all_dirty();
		}
	}
}

VIDEO_START( quizpani )
{
	quizpani_state *state = machine.driver_data<quizpani_state>();
	state->m_bg_tilemap  = tilemap_create(machine, bg_tile_info, bg_scan,16,16,256,32);
	state->m_txt_tilemap = tilemap_create(machine, txt_tile_info,bg_scan,16,16,256,32);
	state->m_txt_tilemap->set_transparent_pen(15);
}

SCREEN_UPDATE_IND16( quizpani )
{
	quizpani_state *state = screen.machine().driver_data<quizpani_state>();
	state->m_bg_tilemap->set_scrollx(0, state->m_scrollreg[0] - 64);
	state->m_bg_tilemap->set_scrolly(0, state->m_scrollreg[1] + 16);
	state->m_txt_tilemap->set_scrollx(0, state->m_scrollreg[2] - 64);
	state->m_txt_tilemap->set_scrolly(0, state->m_scrollreg[3] + 16);

	state->m_bg_tilemap->draw(bitmap, cliprect, 0,0);
	state->m_txt_tilemap->draw(bitmap, cliprect, 0,0);
	return 0;
}
