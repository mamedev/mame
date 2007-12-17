/***************************************************************************

    Raster Elite Tickee Tickats hardware

***************************************************************************/

#include "driver.h"
#include "cpu/tms34010/tms34010.h"
#include "tickee.h"


UINT16 *tickee_vram;


/* local variables */
static emu_timer *setup_gun_timer;



/*************************************
 *
 *  Compute X/Y coordinates
 *
 *************************************/

INLINE void get_crosshair_xy(int player, int *x, int *y)
{
	*x = (((readinputport(4 + player * 2) & 0xff) * (Machine->screen[0].visarea.max_x - Machine->screen[0].visarea.min_x)) >> 8) + Machine->screen[0].visarea.min_x;
	*y = (((readinputport(5 + player * 2) & 0xff) * (Machine->screen[0].visarea.max_y - Machine->screen[0].visarea.min_y)) >> 8) + Machine->screen[0].visarea.min_y;
}



/*************************************
 *
 *  Light gun interrupts
 *
 *************************************/

static TIMER_CALLBACK( trigger_gun_interrupt )
{
	/* fire the IRQ at the correct moment */
	cpunum_set_input_line(0, param, ASSERT_LINE);
}


static TIMER_CALLBACK( clear_gun_interrupt )
{
	/* clear the IRQ on the next scanline? */
	cpunum_set_input_line(0, param, CLEAR_LINE);
}


static TIMER_CALLBACK( setup_gun_interrupts )
{
	int beamx, beamy;

	/* set a timer to do this again next frame */
	timer_adjust(setup_gun_timer, video_screen_get_time_until_pos(0, 0, 0), 0, attotime_zero);

	/* only do work if the palette is flashed */
	if (!tickee_control[2])
		return;

	/* generate interrupts for player 1's gun */
	get_crosshair_xy(0, &beamx, &beamy);
	timer_set(video_screen_get_time_until_pos(0, beamy,     beamx + 50), 0, trigger_gun_interrupt);
	timer_set(video_screen_get_time_until_pos(0, beamy + 1, beamx + 50), 0, clear_gun_interrupt);

	/* generate interrupts for player 2's gun */
	get_crosshair_xy(1, &beamx, &beamy);
	timer_set(video_screen_get_time_until_pos(0, beamy,     beamx + 50), 1, trigger_gun_interrupt);
	timer_set(video_screen_get_time_until_pos(0, beamy + 1, beamx + 50), 1, clear_gun_interrupt);
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START( tickee )
{
	/* start a timer going on the first scanline of every frame */
	setup_gun_timer = timer_alloc(setup_gun_interrupts);
	timer_adjust(setup_gun_timer, video_screen_get_time_until_pos(0, 0, 0), 0, attotime_zero);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

void tickee_scanline_update(running_machine *machine, int screen, mame_bitmap *bitmap, int scanline, const tms34010_display_params *params)
{
	UINT16 *src = &tickee_vram[(params->rowaddr << 8) & 0x3ff00];
	UINT16 *dest = BITMAP_ADDR16(bitmap, scanline, 0);
	int coladdr = params->coladdr << 1;
	int x;

	/* blank palette: fill with pen 255 */
	if (tickee_control[2])
	{
		for (x = params->heblnk; x < params->hsblnk; x++)
			dest[x] = 0xff;
		return;
	}

	/* copy the non-blanked portions of this scanline */
	for (x = params->heblnk; x < params->hsblnk; x += 2)
	{
		UINT16 pixels = src[coladdr++ & 0xff];
		dest[x + 0] = pixels & 0xff;
		dest[x + 1] = pixels >> 8;
	}
}
