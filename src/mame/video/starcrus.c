
/* Ramtek - Star Cruiser */

#include "emu.h"
#include "sound/samples.h"
#include "includes/starcrus.h"

/* The collision detection techniques use in this driver
   are well explained in the comments in the sprint2 driver */

WRITE8_HANDLER( starcrus_s1_x_w ) { space->machine().driver_data<starcrus_state>()->m_s1_x = data^0xff; }
WRITE8_HANDLER( starcrus_s1_y_w ) { space->machine().driver_data<starcrus_state>()->m_s1_y = data^0xff; }
WRITE8_HANDLER( starcrus_s2_x_w ) { space->machine().driver_data<starcrus_state>()->m_s2_x = data^0xff; }
WRITE8_HANDLER( starcrus_s2_y_w ) { space->machine().driver_data<starcrus_state>()->m_s2_y = data^0xff; }
WRITE8_HANDLER( starcrus_p1_x_w ) { space->machine().driver_data<starcrus_state>()->m_p1_x = data^0xff; }
WRITE8_HANDLER( starcrus_p1_y_w ) { space->machine().driver_data<starcrus_state>()->m_p1_y = data^0xff; }
WRITE8_HANDLER( starcrus_p2_x_w ) { space->machine().driver_data<starcrus_state>()->m_p2_x = data^0xff; }
WRITE8_HANDLER( starcrus_p2_y_w ) { space->machine().driver_data<starcrus_state>()->m_p2_y = data^0xff; }

VIDEO_START( starcrus )
{
	starcrus_state *state = machine.driver_data<starcrus_state>();
	state->m_ship1_vid = auto_bitmap_ind16_alloc(machine, 16, 16);
	state->m_ship2_vid = auto_bitmap_ind16_alloc(machine, 16, 16);

	state->m_proj1_vid = auto_bitmap_ind16_alloc(machine, 16, 16);
	state->m_proj2_vid = auto_bitmap_ind16_alloc(machine, 16, 16);
}

WRITE8_HANDLER( starcrus_ship_parm_1_w )
{
	starcrus_state *state = space->machine().driver_data<starcrus_state>();
	samples_device *samples = space->machine().device<samples_device>("samples");

	state->m_s1_sprite = data&0x1f;
	state->m_engine1_on = ((data&0x20)>>5)^0x01;

	if (state->m_engine1_on || state->m_engine2_on)
	{
		if (state->m_engine_sound_playing == 0)
		{
			state->m_engine_sound_playing = 1;
			samples->start(0, 0, true);	/* engine sample */
		}
	}
	else
	{
		if (state->m_engine_sound_playing == 1)
		{
			state->m_engine_sound_playing = 0;
			samples->stop(0);
		}
	}
}

WRITE8_HANDLER( starcrus_ship_parm_2_w )
{
	starcrus_state *state = space->machine().driver_data<starcrus_state>();
	samples_device *samples = space->machine().device<samples_device>("samples");

	state->m_s2_sprite = data&0x1f;
	set_led_status(space->machine(), 2,~data & 0x80);			/* game over lamp */
	coin_counter_w(space->machine(), 0, ((data&0x40)>>6)^0x01);	/* coin counter */
	state->m_engine2_on = ((data&0x20)>>5)^0x01;

	if (state->m_engine1_on || state->m_engine2_on)
	{
		if (state->m_engine_sound_playing == 0)
		{
			state->m_engine_sound_playing = 1;
			samples->start(0, 0, true);	/* engine sample */
		}
	}
	else
	{
		if (state->m_engine_sound_playing == 1)
		{
			state->m_engine_sound_playing = 0;
			samples->stop(0);
		}
	}

}

WRITE8_HANDLER( starcrus_proj_parm_1_w )
{
	starcrus_state *state = space->machine().driver_data<starcrus_state>();
	samples_device *samples = space->machine().device<samples_device>("samples");

	state->m_p1_sprite = data&0x0f;
	state->m_launch1_on = ((data&0x20)>>5)^0x01;
	state->m_explode1_on = ((data&0x10)>>4)^0x01;

	if (state->m_explode1_on || state->m_explode2_on)
	{
		if (state->m_explode_sound_playing == 0)
		{
			state->m_explode_sound_playing = 1;
			samples->start(1,1, true);	/* explosion initial sample */
		}
	}
	else
	{
		if (state->m_explode_sound_playing == 1)
		{
			state->m_explode_sound_playing = 0;
			samples->start(1,2);	/* explosion ending sample */
		}
	}

	if (state->m_launch1_on)
	{
		if (state->m_launch1_sound_playing == 0)
		{
			state->m_launch1_sound_playing = 1;
			samples->start(2,3);	/* launch sample */
		}
	}
	else
	{
		state->m_launch1_sound_playing = 0;
	}
}

WRITE8_HANDLER( starcrus_proj_parm_2_w )
{
	starcrus_state *state = space->machine().driver_data<starcrus_state>();
	samples_device *samples = space->machine().device<samples_device>("samples");

	state->m_p2_sprite = data&0x0f;
	state->m_launch2_on = ((data&0x20)>>5)^0x01;
	state->m_explode2_on = ((data&0x10)>>4)^0x01;

	if (state->m_explode1_on || state->m_explode2_on)
	{
		if (state->m_explode_sound_playing == 0)
		{
			state->m_explode_sound_playing = 1;
			samples->start(1,1, true);	/* explosion initial sample */
		}
	}
	else
	{
		if (state->m_explode_sound_playing == 1)
		{
			state->m_explode_sound_playing = 0;
			samples->start(1,2);	/* explosion ending sample */
		}
	}

	if (state->m_launch2_on)
	{
		if (state->m_launch2_sound_playing == 0)
		{
			state->m_launch2_sound_playing = 1;
			samples->start(3,3);	/* launch sample */
		}
	}
	else
	{
		state->m_launch2_sound_playing = 0;
	}
}

static int collision_check_s1s2(running_machine &machine)
{
	starcrus_state *state = machine.driver_data<starcrus_state>();
	int org_x, org_y;
	int sx, sy;
	rectangle clip(0, 15, 0, 15);

	state->m_ship1_vid->fill(0, clip);
	state->m_ship2_vid->fill(0, clip);

	/* origin is with respect to ship1 */

	org_x = state->m_s1_x;
	org_y = state->m_s1_y;

	/* Draw ship 1 */
	drawgfx_opaque(*state->m_ship1_vid,
			clip,
			machine.gfx[8+((state->m_s1_sprite&0x04)>>2)],
			(state->m_s1_sprite&0x03)^0x03,
			0,
			(state->m_s1_sprite&0x08)>>3, (state->m_s1_sprite&0x10)>>4,
			state->m_s1_x-org_x, state->m_s1_y-org_y);

	/* Draw ship 2 */
	drawgfx_opaque(*state->m_ship2_vid,
			clip,
			machine.gfx[10+((state->m_s2_sprite&0x04)>>2)],
			(state->m_s2_sprite&0x03)^0x03,
			0,
			(state->m_s2_sprite&0x08)>>3, (state->m_s2_sprite&0x10)>>4,
			state->m_s2_x-org_x, state->m_s2_y-org_y);

	/* Now check for collisions */
	for (sy=0;sy<16;sy++)
		for (sx=0;sx<16;sx++)
		/* Condition 1 - ship 1 = ship 2 */
		if ((state->m_ship1_vid->pix16(sy, sx) == 1) && (state->m_ship2_vid->pix16(sy, sx) == 1))
			return 1;

	return 0;
}

static int collision_check_p1p2(running_machine &machine)
{
	starcrus_state *state = machine.driver_data<starcrus_state>();
	int org_x, org_y;
	int sx, sy;
	rectangle clip(0, 15, 0, 15);

	/* if both are scores, return */
	if ( ((state->m_p1_sprite & 0x08) == 0) &&
		 ((state->m_p2_sprite & 0x08) == 0) )
	{
		return 0;
	}

	state->m_proj1_vid->fill(0, clip);
	state->m_proj2_vid->fill(0, clip);

	/* origin is with respect to proj1 */

	org_x = state->m_p1_x;
	org_y = state->m_p1_y;

	if (state->m_p1_sprite & 0x08)	/* if p1 is a projectile */
	{
		/* Draw score/projectile 1 */
		drawgfx_opaque(*state->m_proj1_vid,
				clip,
				machine.gfx[(state->m_p1_sprite&0x0c)>>2],
				(state->m_p1_sprite&0x03)^0x03,
				0,
				0,0,
				state->m_p1_x-org_x, state->m_p1_y-org_y);
	}

	if (state->m_p2_sprite & 0x08)	/* if p2 is a projectile */
	{
		/* Draw score/projectile 2 */
		drawgfx_opaque(*state->m_proj2_vid,
				clip,
				machine.gfx[4+((state->m_p2_sprite&0x0c)>>2)],
				(state->m_p2_sprite&0x03)^0x03,
				0,
				0,0,
				state->m_p2_x-org_x, state->m_p2_y-org_y);
	}

	/* Now check for collisions */
	for (sy=0;sy<16;sy++)
		for (sx=0;sx<16;sx++)
			/* Condition 1 - proj 1 = proj 2 */
			if ((state->m_proj1_vid->pix16(sy, sx) == 1) && (state->m_proj2_vid->pix16(sy, sx) == 1))
				return 1;

	return 0;
}

static int collision_check_s1p1p2(running_machine &machine)
{
	starcrus_state *state = machine.driver_data<starcrus_state>();
	int org_x, org_y;
	int sx, sy;
	rectangle clip(0, 15, 0, 15);

	/* if both are scores, return */
	if ( ((state->m_p1_sprite & 0x08) == 0) &&
		 ((state->m_p2_sprite & 0x08) == 0) )
	{
		return 0;
	}

	state->m_ship1_vid->fill(0, clip);
	state->m_proj1_vid->fill(0, clip);
	state->m_proj2_vid->fill(0, clip);

	/* origin is with respect to ship1 */

	org_x = state->m_s1_x;
	org_y = state->m_s1_y;

	/* Draw ship 1 */
	drawgfx_opaque(*state->m_ship1_vid,
			clip,
			machine.gfx[8+((state->m_s1_sprite&0x04)>>2)],
			(state->m_s1_sprite&0x03)^0x03,
			0,
			(state->m_s1_sprite&0x08)>>3, (state->m_s1_sprite&0x10)>>4,
			state->m_s1_x-org_x, state->m_s1_y-org_y);

	if (state->m_p1_sprite & 0x08)	/* if p1 is a projectile */
	{
		/* Draw projectile 1 */
		drawgfx_opaque(*state->m_proj1_vid,
				clip,
				machine.gfx[(state->m_p1_sprite&0x0c)>>2],
				(state->m_p1_sprite&0x03)^0x03,
				0,
				0,0,
				state->m_p1_x-org_x, state->m_p1_y-org_y);
	}

	if (state->m_p2_sprite & 0x08)	/* if p2 is a projectile */
	{
		/* Draw projectile 2 */
		drawgfx_opaque(*state->m_proj2_vid,
				clip,
				machine.gfx[4+((state->m_p2_sprite&0x0c)>>2)],
				(state->m_p2_sprite&0x03)^0x03,
				0,
				0,0,
				state->m_p2_x-org_x, state->m_p2_y-org_y);
	}

	/* Now check for collisions */
	for (sy=0;sy<16;sy++)
		for (sx=0;sx<16;sx++)
			if (state->m_ship1_vid->pix16(sy, sx) == 1)
			{
				/* Condition 1 - ship 1 = proj 1 */
				if (state->m_proj1_vid->pix16(sy, sx) == 1)
					return 1;
				/* Condition 2 - ship 1 = proj 2 */
				if (state->m_proj2_vid->pix16(sy, sx) == 1)
					return 1;
			}

	return 0;
}

static int collision_check_s2p1p2(running_machine &machine)
{
	starcrus_state *state = machine.driver_data<starcrus_state>();
	int org_x, org_y;
	int sx, sy;
	rectangle clip(0, 15, 0, 15);

	/* if both are scores, return */
	if ( ((state->m_p1_sprite & 0x08) == 0) &&
		 ((state->m_p2_sprite & 0x08) == 0) )
	{
		return 0;
	}

	state->m_ship2_vid->fill(0, clip);
	state->m_proj1_vid->fill(0, clip);
	state->m_proj2_vid->fill(0, clip);

	/* origin is with respect to ship2 */

	org_x = state->m_s2_x;
	org_y = state->m_s2_y;

	/* Draw ship 2 */
	drawgfx_opaque(*state->m_ship2_vid,
			clip,
			machine.gfx[10+((state->m_s2_sprite&0x04)>>2)],
			(state->m_s2_sprite&0x03)^0x03,
			0,
			(state->m_s2_sprite&0x08)>>3, (state->m_s2_sprite&0x10)>>4,
			state->m_s2_x-org_x, state->m_s2_y-org_y);

	if (state->m_p1_sprite & 0x08)	/* if p1 is a projectile */
	{
		/* Draw projectile 1 */
		drawgfx_opaque(*state->m_proj1_vid,
				clip,
				machine.gfx[(state->m_p1_sprite&0x0c)>>2],
				(state->m_p1_sprite&0x03)^0x03,
				0,
				0,0,
				state->m_p1_x-org_x, state->m_p1_y-org_y);
	}

	if (state->m_p2_sprite & 0x08)	/* if p2 is a projectile */
	{
		/* Draw projectile 2 */
		drawgfx_opaque(*state->m_proj2_vid,
				clip,
				machine.gfx[4+((state->m_p2_sprite&0x0c)>>2)],
				(state->m_p2_sprite&0x03)^0x03,
				0,
				0,0,
				state->m_p2_x-org_x, state->m_p2_y-org_y);
	}

	/* Now check for collisions */
	for (sy=0;sy<16;sy++)
		for (sx=0;sx<16;sx++)
			if (state->m_ship2_vid->pix16(sy, sx) == 1)
			{
				/* Condition 1 - ship 2 = proj 1 */
				if (state->m_proj1_vid->pix16(sy, sx) == 1)
					return 1;
				/* Condition 2 - ship 2 = proj 2 */
				if (state->m_proj2_vid->pix16(sy, sx) == 1)
					return 1;
			}

	return 0;
}

SCREEN_UPDATE_IND16( starcrus )
{
	starcrus_state *state = screen.machine().driver_data<starcrus_state>();

	bitmap.fill(0, cliprect);

	/* Draw ship 1 */
	drawgfx_transpen(bitmap,
			cliprect,
			screen.machine().gfx[8+((state->m_s1_sprite&0x04)>>2)],
			(state->m_s1_sprite&0x03)^0x03,
			0,
			(state->m_s1_sprite&0x08)>>3, (state->m_s1_sprite&0x10)>>4,
			state->m_s1_x, state->m_s1_y,
			0);

	/* Draw ship 2 */
	drawgfx_transpen(bitmap,
			cliprect,
			screen.machine().gfx[10+((state->m_s2_sprite&0x04)>>2)],
			(state->m_s2_sprite&0x03)^0x03,
			0,
			(state->m_s2_sprite&0x08)>>3, (state->m_s2_sprite&0x10)>>4,
			state->m_s2_x, state->m_s2_y,
			0);

	/* Draw score/projectile 1 */
	drawgfx_transpen(bitmap,
			cliprect,
			screen.machine().gfx[(state->m_p1_sprite&0x0c)>>2],
			(state->m_p1_sprite&0x03)^0x03,
			0,
			0,0,
			state->m_p1_x, state->m_p1_y,
			0);

	/* Draw score/projectile 2 */
	drawgfx_transpen(bitmap,
			cliprect,
			screen.machine().gfx[4+((state->m_p2_sprite&0x0c)>>2)],
			(state->m_p2_sprite&0x03)^0x03,
			0,
			0,0,
			state->m_p2_x, state->m_p2_y,
			0);

	/* Collision detection */
	if (cliprect.max_y == screen.visible_area().max_y)
	{
		state->m_collision_reg = 0x00;

		/* Check for collisions between ship1 and ship2 */
		if (collision_check_s1s2(screen.machine()))
		{
			state->m_collision_reg |= 0x08;
		}
		/* Check for collisions between ship1 and projectiles */
		if (collision_check_s1p1p2(screen.machine()))
		{
			state->m_collision_reg |= 0x02;
		}
		/* Check for collisions between ship1 and projectiles */
		if (collision_check_s2p1p2(screen.machine()))
		{
			state->m_collision_reg |= 0x01;
		}
		/* Check for collisions between ship1 and projectiles */
		/* Note: I don't think this is used by the game */
		if (collision_check_p1p2(screen.machine()))
		{
			state->m_collision_reg |= 0x04;
		}
	}

	return 0;
}

READ8_HANDLER( starcrus_coll_det_r )
{
	starcrus_state *state = space->machine().driver_data<starcrus_state>();

	return state->m_collision_reg ^ 0xff;
}
