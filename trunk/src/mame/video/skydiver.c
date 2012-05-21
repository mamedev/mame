/***************************************************************************

    Atari Sky Diver hardware

***************************************************************************/

#include "emu.h"
#include "includes/skydiver.h"
#include "sound/discrete.h"


MACHINE_RESET( skydiver )
{
	skydiver_state *state = machine.driver_data<skydiver_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	/* reset all latches */
	state->skydiver_start_lamp_1_w(*space, 0, 0);
	state->skydiver_start_lamp_2_w(*space, 0, 0);
	state->skydiver_lamp_s_w(*space, 0, 0);
	state->skydiver_lamp_k_w(*space, 0, 0);
	state->skydiver_lamp_y_w(*space, 0, 0);
	state->skydiver_lamp_d_w(*space, 0, 0);
	output_set_value("lampi", 0);
	output_set_value("lampv", 0);
	output_set_value("lampe", 0);
	output_set_value("lampr", 0);
	state->skydiver_width_w(*space, 0, 0);
	state->skydiver_coin_lockout_w(*space, 0, 0);
}


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	skydiver_state *state = machine.driver_data<skydiver_state>();
	UINT8 code = state->m_videoram[tile_index];
	SET_TILE_INFO(0, code & 0x3f, code >> 6, 0);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( skydiver )
{
	skydiver_state *state = machine.driver_data<skydiver_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_tile_info,tilemap_scan_rows,8,8,32,32);
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_MEMBER(skydiver_state::skydiver_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


READ8_MEMBER(skydiver_state::skydiver_wram_r)
{
	return m_videoram[offset | 0x380];
}

WRITE8_MEMBER(skydiver_state::skydiver_wram_w)
{
	m_videoram[offset | 0x0380] = data;
}


WRITE8_MEMBER(skydiver_state::skydiver_width_w)
{
	m_width = offset;
}


WRITE8_MEMBER(skydiver_state::skydiver_coin_lockout_w)
{
	coin_lockout_global_w(machine(), !offset);
}


WRITE8_MEMBER(skydiver_state::skydiver_start_lamp_1_w)
{
	set_led_status(machine(), 0, offset);
}

WRITE8_MEMBER(skydiver_state::skydiver_start_lamp_2_w)
{
	set_led_status(machine(), 1, offset);
}


WRITE8_MEMBER(skydiver_state::skydiver_lamp_s_w)
{
	output_set_value("lamps", offset);
}

WRITE8_MEMBER(skydiver_state::skydiver_lamp_k_w)
{
	output_set_value("lampk", offset);
}

WRITE8_MEMBER(skydiver_state::skydiver_lamp_y_w)
{
	output_set_value("lampy", offset);
}

WRITE8_MEMBER(skydiver_state::skydiver_lamp_d_w)
{
	output_set_value("lampd", offset);
}

WRITE8_MEMBER(skydiver_state::skydiver_2000_201F_w)
{
	device_t *discrete = machine().device("discrete");
	int bit = offset & 0x01;

	watchdog_reset_w(space,0,0);

	switch (offset & 0x0e)
	{
		case (0x02):
			output_set_value("lampi", bit);
			break;
		case (0x04):
			output_set_value("lampv", bit);
			break;
		case (0x06):
			output_set_value("lampe", bit);
			break;
		case (0x08):
			output_set_value("lampr", bit);
			break;
		case (0x0a):
			discrete_sound_w(discrete, SKYDIVER_OCT1_EN, bit);
			break;
		case (0x0c):
			discrete_sound_w(discrete, SKYDIVER_OCT2_EN, bit);
			break;
		case (0x0e):
			discrete_sound_w(discrete, SKYDIVER_NOISE_RST, bit);
			break;
	}
}


/*************************************
 *
 *  Video update
 *
 *************************************/

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	skydiver_state *state = machine.driver_data<skydiver_state>();
	int pic;


	/* draw each one of our four motion objects, the two PLANE sprites
       can be drawn double width */
	for (pic = 3; pic >= 0; pic--)
	{
		int sx,sy;
		int charcode;
		int xflip, yflip;
		int color;
		int wide;

		sx = 29*8 - state->m_videoram[pic + 0x0390];
		sy = 30*8 - state->m_videoram[pic*2 + 0x0398];
		charcode = state->m_videoram[pic*2 + 0x0399];
		xflip = charcode & 0x10;
		yflip = charcode & 0x08;
		wide = (~pic & 0x02) && state->m_width;
		charcode = (charcode & 0x07) | ((charcode & 0x60) >> 2);
		color = pic & 0x01;

		if (wide)
		{
			sx -= 8;
		}

		drawgfxzoom_transpen(bitmap,cliprect,machine.gfx[1],
			charcode, color,
			xflip,yflip,sx,sy,
			wide ? 0x20000 : 0x10000, 0x10000,0);
	}
}


SCREEN_UPDATE_IND16( skydiver )
{
	skydiver_state *state = screen.machine().driver_data<skydiver_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0,0);

	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
