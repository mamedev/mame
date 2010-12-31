
/* Ramtek - Star Cruiser */

#include "emu.h"
#include "sound/samples.h"
#include "includes/starcrus.h"

/* The collision detection techniques use in this driver
   are well explained in the comments in the sprint2 driver */

WRITE8_HANDLER( starcrus_s1_x_w ) { space->machine->driver_data<starcrus_state>()->s1_x = data^0xff; }
WRITE8_HANDLER( starcrus_s1_y_w ) { space->machine->driver_data<starcrus_state>()->s1_y = data^0xff; }
WRITE8_HANDLER( starcrus_s2_x_w ) { space->machine->driver_data<starcrus_state>()->s2_x = data^0xff; }
WRITE8_HANDLER( starcrus_s2_y_w ) { space->machine->driver_data<starcrus_state>()->s2_y = data^0xff; }
WRITE8_HANDLER( starcrus_p1_x_w ) { space->machine->driver_data<starcrus_state>()->p1_x = data^0xff; }
WRITE8_HANDLER( starcrus_p1_y_w ) { space->machine->driver_data<starcrus_state>()->p1_y = data^0xff; }
WRITE8_HANDLER( starcrus_p2_x_w ) { space->machine->driver_data<starcrus_state>()->p2_x = data^0xff; }
WRITE8_HANDLER( starcrus_p2_y_w ) { space->machine->driver_data<starcrus_state>()->p2_y = data^0xff; }

VIDEO_START( starcrus )
{
	starcrus_state *state = machine->driver_data<starcrus_state>();
	state->ship1_vid = auto_bitmap_alloc(machine, 16, 16, machine->primary_screen->format());
	state->ship2_vid = auto_bitmap_alloc(machine, 16, 16, machine->primary_screen->format());

	state->proj1_vid = auto_bitmap_alloc(machine, 16, 16, machine->primary_screen->format());
	state->proj2_vid = auto_bitmap_alloc(machine, 16, 16, machine->primary_screen->format());
}

WRITE8_HANDLER( starcrus_ship_parm_1_w )
{
	starcrus_state *state = space->machine->driver_data<starcrus_state>();
	device_t *samples = space->machine->device("samples");

	state->s1_sprite = data&0x1f;
	state->engine1_on = ((data&0x20)>>5)^0x01;

	if (state->engine1_on || state->engine2_on)
	{
		if (state->engine_sound_playing == 0)
		{
			state->engine_sound_playing = 1;
			sample_start(samples, 0, 0, 1);	/* engine sample */
		}
	}
	else
	{
		if (state->engine_sound_playing == 1)
		{
			state->engine_sound_playing = 0;
			sample_stop(samples, 0);
		}
	}
}

WRITE8_HANDLER( starcrus_ship_parm_2_w )
{
	starcrus_state *state = space->machine->driver_data<starcrus_state>();
	device_t *samples = space->machine->device("samples");

	state->s2_sprite = data&0x1f;
	set_led_status(space->machine, 2,~data & 0x80);			/* game over lamp */
	coin_counter_w(space->machine, 0, ((data&0x40)>>6)^0x01);	/* coin counter */
	state->engine2_on = ((data&0x20)>>5)^0x01;

	if (state->engine1_on || state->engine2_on)
	{
		if (state->engine_sound_playing == 0)
		{
			state->engine_sound_playing = 1;
			sample_start(samples, 0, 0, 1);	/* engine sample */
		}
	}
	else
	{
		if (state->engine_sound_playing == 1)
		{
			state->engine_sound_playing = 0;
			sample_stop(samples, 0);
		}
	}

}

WRITE8_HANDLER( starcrus_proj_parm_1_w )
{
	starcrus_state *state = space->machine->driver_data<starcrus_state>();
	device_t *samples = space->machine->device("samples");

	state->p1_sprite = data&0x0f;
	state->launch1_on = ((data&0x20)>>5)^0x01;
	state->explode1_on = ((data&0x10)>>4)^0x01;

	if (state->explode1_on || state->explode2_on)
	{
		if (state->explode_sound_playing == 0)
		{
			state->explode_sound_playing = 1;
			sample_start(samples, 1,1,1);	/* explosion initial sample */
		}
	}
	else
	{
		if (state->explode_sound_playing == 1)
		{
			state->explode_sound_playing = 0;
			sample_start(samples, 1,2,0);	/* explosion ending sample */
		}
	}

	if (state->launch1_on)
	{
		if (state->launch1_sound_playing == 0)
		{
			state->launch1_sound_playing = 1;
			sample_start(samples, 2,3,0);	/* launch sample */
		}
	}
	else
	{
		state->launch1_sound_playing = 0;
	}
}

WRITE8_HANDLER( starcrus_proj_parm_2_w )
{
	starcrus_state *state = space->machine->driver_data<starcrus_state>();
	device_t *samples = space->machine->device("samples");

	state->p2_sprite = data&0x0f;
	state->launch2_on = ((data&0x20)>>5)^0x01;
	state->explode2_on = ((data&0x10)>>4)^0x01;

	if (state->explode1_on || state->explode2_on)
	{
		if (state->explode_sound_playing == 0)
		{
			state->explode_sound_playing = 1;
			sample_start(samples, 1,1,1);	/* explosion initial sample */
		}
	}
	else
	{
		if (state->explode_sound_playing == 1)
		{
			state->explode_sound_playing = 0;
			sample_start(samples, 1,2,0);	/* explosion ending sample */
		}
	}

	if (state->launch2_on)
	{
		if (state->launch2_sound_playing == 0)
		{
			state->launch2_sound_playing = 1;
			sample_start(samples, 3,3,0);	/* launch sample */
		}
	}
	else
	{
		state->launch2_sound_playing = 0;
	}
}

static int collision_check_s1s2(running_machine *machine)
{
	starcrus_state *state = machine->driver_data<starcrus_state>();
	int org_x, org_y;
	int sx, sy;
	rectangle clip;

	clip.min_x=0;
	clip.max_x=15;
	clip.min_y=0;
	clip.max_y=15;

	bitmap_fill(state->ship1_vid, &clip, 0);
	bitmap_fill(state->ship2_vid, &clip, 0);

	/* origin is with respect to ship1 */

	org_x = state->s1_x;
	org_y = state->s1_y;

	/* Draw ship 1 */
	drawgfx_opaque(state->ship1_vid,
			&clip,
			machine->gfx[8+((state->s1_sprite&0x04)>>2)],
			(state->s1_sprite&0x03)^0x03,
			0,
			(state->s1_sprite&0x08)>>3, (state->s1_sprite&0x10)>>4,
			state->s1_x-org_x, state->s1_y-org_y);

	/* Draw ship 2 */
	drawgfx_opaque(state->ship2_vid,
			&clip,
			machine->gfx[10+((state->s2_sprite&0x04)>>2)],
			(state->s2_sprite&0x03)^0x03,
			0,
			(state->s2_sprite&0x08)>>3, (state->s2_sprite&0x10)>>4,
			state->s2_x-org_x, state->s2_y-org_y);

	/* Now check for collisions */
	for (sy=0;sy<16;sy++)
		for (sx=0;sx<16;sx++)
		/* Condition 1 - ship 1 = ship 2 */
		if ((*BITMAP_ADDR16(state->ship1_vid, sy, sx) == 1) && (*BITMAP_ADDR16(state->ship2_vid, sy, sx) == 1))
			return 1;

	return 0;
}

static int collision_check_p1p2(running_machine *machine)
{
	starcrus_state *state = machine->driver_data<starcrus_state>();
	int org_x, org_y;
	int sx, sy;
	rectangle clip;

	/* if both are scores, return */
	if ( ((state->p1_sprite & 0x08) == 0) &&
		 ((state->p2_sprite & 0x08) == 0) )
	{
		return 0;
	}

	clip.min_x=0;
	clip.max_x=15;
	clip.min_y=0;
	clip.max_y=15;

	bitmap_fill(state->proj1_vid, &clip, 0);
	bitmap_fill(state->proj2_vid, &clip, 0);

	/* origin is with respect to proj1 */

	org_x = state->p1_x;
	org_y = state->p1_y;

	if (state->p1_sprite & 0x08)	/* if p1 is a projectile */
	{
		/* Draw score/projectile 1 */
		drawgfx_opaque(state->proj1_vid,
				&clip,
				machine->gfx[(state->p1_sprite&0x0c)>>2],
				(state->p1_sprite&0x03)^0x03,
				0,
				0,0,
				state->p1_x-org_x, state->p1_y-org_y);
	}

	if (state->p2_sprite & 0x08)	/* if p2 is a projectile */
	{
		/* Draw score/projectile 2 */
		drawgfx_opaque(state->proj2_vid,
				&clip,
				machine->gfx[4+((state->p2_sprite&0x0c)>>2)],
				(state->p2_sprite&0x03)^0x03,
				0,
				0,0,
				state->p2_x-org_x, state->p2_y-org_y);
	}

	/* Now check for collisions */
	for (sy=0;sy<16;sy++)
		for (sx=0;sx<16;sx++)
			/* Condition 1 - proj 1 = proj 2 */
			if ((*BITMAP_ADDR16(state->proj1_vid, sy, sx) == 1) && (*BITMAP_ADDR16(state->proj2_vid, sy, sx) == 1))
				return 1;

	return 0;
}

static int collision_check_s1p1p2(running_machine *machine)
{
	starcrus_state *state = machine->driver_data<starcrus_state>();
	int org_x, org_y;
	int sx, sy;
	rectangle clip;

	/* if both are scores, return */
	if ( ((state->p1_sprite & 0x08) == 0) &&
		 ((state->p2_sprite & 0x08) == 0) )
	{
		return 0;
	}

	clip.min_x=0;
	clip.max_x=15;
	clip.min_y=0;
	clip.max_y=15;

	bitmap_fill(state->ship1_vid, &clip, 0);
	bitmap_fill(state->proj1_vid, &clip, 0);
	bitmap_fill(state->proj2_vid, &clip, 0);

	/* origin is with respect to ship1 */

	org_x = state->s1_x;
	org_y = state->s1_y;

	/* Draw ship 1 */
	drawgfx_opaque(state->ship1_vid,
			&clip,
			machine->gfx[8+((state->s1_sprite&0x04)>>2)],
			(state->s1_sprite&0x03)^0x03,
			0,
			(state->s1_sprite&0x08)>>3, (state->s1_sprite&0x10)>>4,
			state->s1_x-org_x, state->s1_y-org_y);

	if (state->p1_sprite & 0x08)	/* if p1 is a projectile */
	{
		/* Draw projectile 1 */
		drawgfx_opaque(state->proj1_vid,
				&clip,
				machine->gfx[(state->p1_sprite&0x0c)>>2],
				(state->p1_sprite&0x03)^0x03,
				0,
				0,0,
				state->p1_x-org_x, state->p1_y-org_y);
	}

	if (state->p2_sprite & 0x08)	/* if p2 is a projectile */
	{
		/* Draw projectile 2 */
		drawgfx_opaque(state->proj2_vid,
				&clip,
				machine->gfx[4+((state->p2_sprite&0x0c)>>2)],
				(state->p2_sprite&0x03)^0x03,
				0,
				0,0,
				state->p2_x-org_x, state->p2_y-org_y);
	}

	/* Now check for collisions */
	for (sy=0;sy<16;sy++)
		for (sx=0;sx<16;sx++)
			if (*BITMAP_ADDR16(state->ship1_vid, sy, sx) == 1)
			{
				/* Condition 1 - ship 1 = proj 1 */
				if (*BITMAP_ADDR16(state->proj1_vid, sy, sx) == 1)
					return 1;
				/* Condition 2 - ship 1 = proj 2 */
				if (*BITMAP_ADDR16(state->proj2_vid, sy, sx) == 1)
					return 1;
			}

	return 0;
}

static int collision_check_s2p1p2(running_machine *machine)
{
	starcrus_state *state = machine->driver_data<starcrus_state>();
	int org_x, org_y;
	int sx, sy;
	rectangle clip;

	/* if both are scores, return */
	if ( ((state->p1_sprite & 0x08) == 0) &&
		 ((state->p2_sprite & 0x08) == 0) )
	{
		return 0;
	}

	clip.min_x=0;
	clip.max_x=15;
	clip.min_y=0;
	clip.max_y=15;

	bitmap_fill(state->ship2_vid, &clip, 0);
	bitmap_fill(state->proj1_vid, &clip, 0);
	bitmap_fill(state->proj2_vid, &clip, 0);

	/* origin is with respect to ship2 */

	org_x = state->s2_x;
	org_y = state->s2_y;

	/* Draw ship 2 */
	drawgfx_opaque(state->ship2_vid,
			&clip,
			machine->gfx[10+((state->s2_sprite&0x04)>>2)],
			(state->s2_sprite&0x03)^0x03,
			0,
			(state->s2_sprite&0x08)>>3, (state->s2_sprite&0x10)>>4,
			state->s2_x-org_x, state->s2_y-org_y);

	if (state->p1_sprite & 0x08)	/* if p1 is a projectile */
	{
		/* Draw projectile 1 */
		drawgfx_opaque(state->proj1_vid,
				&clip,
				machine->gfx[(state->p1_sprite&0x0c)>>2],
				(state->p1_sprite&0x03)^0x03,
				0,
				0,0,
				state->p1_x-org_x, state->p1_y-org_y);
	}

	if (state->p2_sprite & 0x08)	/* if p2 is a projectile */
	{
		/* Draw projectile 2 */
		drawgfx_opaque(state->proj2_vid,
				&clip,
				machine->gfx[4+((state->p2_sprite&0x0c)>>2)],
				(state->p2_sprite&0x03)^0x03,
				0,
				0,0,
				state->p2_x-org_x, state->p2_y-org_y);
	}

	/* Now check for collisions */
	for (sy=0;sy<16;sy++)
		for (sx=0;sx<16;sx++)
			if (*BITMAP_ADDR16(state->ship2_vid, sy, sx) == 1)
			{
				/* Condition 1 - ship 2 = proj 1 */
				if (*BITMAP_ADDR16(state->proj1_vid, sy, sx) == 1)
					return 1;
				/* Condition 2 - ship 2 = proj 2 */
				if (*BITMAP_ADDR16(state->proj2_vid, sy, sx) == 1)
					return 1;
			}

	return 0;
}

VIDEO_UPDATE( starcrus )
{
	starcrus_state *state = screen->machine->driver_data<starcrus_state>();

	bitmap_fill(bitmap,cliprect,0);

	/* Draw ship 1 */
	drawgfx_transpen(bitmap,
			cliprect,
			screen->machine->gfx[8+((state->s1_sprite&0x04)>>2)],
			(state->s1_sprite&0x03)^0x03,
			0,
			(state->s1_sprite&0x08)>>3, (state->s1_sprite&0x10)>>4,
			state->s1_x, state->s1_y,
			0);

	/* Draw ship 2 */
	drawgfx_transpen(bitmap,
			cliprect,
			screen->machine->gfx[10+((state->s2_sprite&0x04)>>2)],
			(state->s2_sprite&0x03)^0x03,
			0,
			(state->s2_sprite&0x08)>>3, (state->s2_sprite&0x10)>>4,
			state->s2_x, state->s2_y,
			0);

	/* Draw score/projectile 1 */
	drawgfx_transpen(bitmap,
			cliprect,
			screen->machine->gfx[(state->p1_sprite&0x0c)>>2],
			(state->p1_sprite&0x03)^0x03,
			0,
			0,0,
			state->p1_x, state->p1_y,
			0);

	/* Draw score/projectile 2 */
	drawgfx_transpen(bitmap,
			cliprect,
			screen->machine->gfx[4+((state->p2_sprite&0x0c)>>2)],
			(state->p2_sprite&0x03)^0x03,
			0,
			0,0,
			state->p2_x, state->p2_y,
			0);

	/* Collision detection */
	if (cliprect->max_y == screen->visible_area().max_y)
	{
		state->collision_reg = 0x00;

		/* Check for collisions between ship1 and ship2 */
		if (collision_check_s1s2(screen->machine))
		{
			state->collision_reg |= 0x08;
		}
		/* Check for collisions between ship1 and projectiles */
		if (collision_check_s1p1p2(screen->machine))
		{
			state->collision_reg |= 0x02;
		}
		/* Check for collisions between ship1 and projectiles */
		if (collision_check_s2p1p2(screen->machine))
		{
			state->collision_reg |= 0x01;
		}
		/* Check for collisions between ship1 and projectiles */
		/* Note: I don't think this is used by the game */
		if (collision_check_p1p2(screen->machine))
		{
			state->collision_reg |= 0x04;
		}
	}

	return 0;
}

READ8_HANDLER( starcrus_coll_det_r )
{
	starcrus_state *state = space->machine->driver_data<starcrus_state>();

	return state->collision_reg ^ 0xff;
}
