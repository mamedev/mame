/***************************************************************************

    Coors Light Bowling/Bowl-O-Rama hardware

***************************************************************************/

#include "emu.h"
#include "video/tms34061.h"
#include "cpu/m6809/m6809.h"
#include "includes/capbowl.h"


/*************************************
 *
 *  TMS34061 interfacing
 *
 *************************************/

static void generate_interrupt( running_machine *machine, int state )
{
	capbowl_state *driver = machine->driver_data<capbowl_state>();
	cpu_set_input_line(driver->maincpu, M6809_FIRQ_LINE, state);
}

static const struct tms34061_interface tms34061intf =
{
	"screen",				/* the screen we are acting on */
	8,						/* VRAM address is (row << rowshift) | col */
	0x10000,				/* size of video RAM */
	generate_interrupt		/* interrupt gen callback */
};



/*************************************
 *
 *  Video start
 *
 *************************************/

VIDEO_START( capbowl )
{
	/* initialize TMS34061 emulation */
    tms34061_start(machine, &tms34061intf);
}



/*************************************
 *
 *  TMS34061 I/O
 *
 *************************************/

WRITE8_HANDLER( capbowl_tms34061_w )
{
	capbowl_state *state = space->machine->driver_data<capbowl_state>();
	int func = (offset >> 8) & 3;
	int col = offset & 0xff;

	/* Column address (CA0-CA8) is hooked up the A0-A7, with A1 being inverted
       during register access. CA8 is ignored */
	if (func == 0 || func == 2)
		col ^= 2;

	/* Row address (RA0-RA8) is not dependent on the offset */
	tms34061_w(space, col, *state->rowaddress, func, data);
}


READ8_HANDLER( capbowl_tms34061_r )
{
	capbowl_state *state = space->machine->driver_data<capbowl_state>();
	int func = (offset >> 8) & 3;
	int col = offset & 0xff;

	/* Column address (CA0-CA8) is hooked up the A0-A7, with A1 being inverted
       during register access. CA8 is ignored */
	if (func == 0 || func == 2)
		col ^= 2;

	/* Row address (RA0-RA8) is not dependent on the offset */
	return tms34061_r(space, col, *state->rowaddress, func);
}



/*************************************
 *
 *  Bowl-o-rama blitter
 *
 *************************************/

WRITE8_HANDLER( bowlrama_blitter_w )
{
	capbowl_state *state = space->machine->driver_data<capbowl_state>();

	switch (offset)
	{
		case 0x08:	  /* Write address high byte (only 2 bits used) */
			state->blitter_addr = (state->blitter_addr & ~0xff0000) | (data << 16);
			break;

		case 0x17:    /* Write address mid byte (8 bits)   */
			state->blitter_addr = (state->blitter_addr & ~0x00ff00) | (data << 8);
			break;

		case 0x18:	  /* Write Address low byte (8 bits)   */
			state->blitter_addr = (state->blitter_addr & ~0x0000ff) | (data << 0);
			break;

		default:
			logerror("PC=%04X Write to unsupported blitter address %02X Data=%02X\n", cpu_get_pc(space->cpu), offset, data);
			break;
	}
}


READ8_HANDLER( bowlrama_blitter_r )
{
	capbowl_state *state = space->machine->driver_data<capbowl_state>();
	UINT8 data = space->machine->region("gfx1")->base()[state->blitter_addr];
	UINT8 result = 0;

	switch (offset)
	{
		/* Read Mask: Graphics data are 4bpp (2 pixels per byte).
            This function returns 0's for new pixel data.
            This allows data to be read as a mask, AND the mask with
            the screen data, then OR new data read by read data command. */
		case 0:
			if (!(data & 0xf0))
				result |= 0xf0;		/* High nibble is transparent */
			if (!(data & 0x0f))
				result |= 0x0f;		/* Low nibble is transparent */
			break;

		/* Read data and increment address */
		case 4:
			result = data;
			state->blitter_addr = (state->blitter_addr + 1) & 0x3ffff;
			break;

		default:
			logerror("PC=%04X Read from unsupported blitter address %02X\n", cpu_get_pc(space->cpu), offset);
			break;
	}

	return result;
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

INLINE rgb_t pen_for_pixel( UINT8 *src, UINT8 pix )
{
	return MAKE_RGB(pal4bit(src[(pix << 1) + 0] >> 0),
					pal4bit(src[(pix << 1) + 1] >> 4),
					pal4bit(src[(pix << 1) + 1] >> 0));
}


VIDEO_UPDATE( capbowl )
{
	struct tms34061_display state;
	int x, y;

	/* first get the current display state */
	tms34061_get_display_state(&state);

	/* if we're blanked, just fill with black */
	if (state.blanked)
	{
		bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
		return 0;
	}

	/* now regenerate the bitmap */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		UINT8 *src = &state.vram[256 * y];
		UINT32 *dest = BITMAP_ADDR32(bitmap, y, 0);

		for (x = cliprect->min_x & ~1; x <= cliprect->max_x; x += 2)
		{
			UINT8 pix = src[32 + (x / 2)];
			*dest++ = pen_for_pixel(src, pix >> 4);
			*dest++ = pen_for_pixel(src, pix & 0x0f);
		}
	}
	return 0;
}
