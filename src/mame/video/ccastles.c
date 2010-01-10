/***************************************************************************

    Atari Crystal Castles hardware

***************************************************************************/

#include "emu.h"
#include "includes/ccastles.h"
#include "video/resnet.h"


/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START( ccastles )
{
	ccastles_state *state = (ccastles_state *)machine->driver_data;
	static const int resistances[3] = { 22000, 10000, 4700 };

	/* get pointers to our PROMs */
	state->syncprom = memory_region(machine, "proms") + 0x000;
	state->wpprom = memory_region(machine, "proms") + 0x200;
	state->priprom = memory_region(machine, "proms") + 0x300;

	/* compute the color output resistor weights at startup */
	compute_resistor_weights(0,	255, -1.0,
			3,	resistances, state->rweights, 1000, 0,
			3,	resistances, state->gweights, 1000, 0,
			3,	resistances, state->bweights, 1000, 0);

	/* allocate a bitmap for drawing sprites */
	state->spritebitmap = video_screen_auto_bitmap_alloc(machine->primary_screen);

	/* register for savestates */
	state_save_register_global_array(machine, state->video_control);
	state_save_register_global_array(machine, state->bitmode_addr);
	state_save_register_global(machine, state->hscroll);
	state_save_register_global(machine, state->vscroll);
}



/*************************************
 *
 *  Video control registers
 *
 *************************************/

WRITE8_HANDLER( ccastles_hscroll_w )
{
	ccastles_state *state = (ccastles_state *)space->machine->driver_data;
	video_screen_update_partial(space->machine->primary_screen, video_screen_get_vpos(space->machine->primary_screen));
	state->hscroll = data;
}


WRITE8_HANDLER( ccastles_vscroll_w )
{
	ccastles_state *state = (ccastles_state *)space->machine->driver_data;
	state->vscroll = data;
}


WRITE8_HANDLER( ccastles_video_control_w )
{
	ccastles_state *state = (ccastles_state *)space->machine->driver_data;
	/* only D3 matters */
	state->video_control[offset] = (data >> 3) & 1;
}



/*************************************
 *
 *  Palette RAM accesses
 *
 *************************************/

WRITE8_HANDLER( ccastles_paletteram_w )
{
	ccastles_state *state = (ccastles_state *)space->machine->driver_data;
	int bit0, bit1, bit2;
	int r, g, b;

	/* extract the raw RGB bits */
	r = ((data & 0xc0) >> 6) | ((offset & 0x20) >> 3);
	b = (data & 0x38) >> 3;
	g = (data & 0x07);

	/* red component (inverted) */
	bit0 = (~r >> 0) & 0x01;
	bit1 = (~r >> 1) & 0x01;
	bit2 = (~r >> 2) & 0x01;
	r = combine_3_weights(state->rweights, bit0, bit1, bit2);

	/* green component (inverted) */
	bit0 = (~g >> 0) & 0x01;
	bit1 = (~g >> 1) & 0x01;
	bit2 = (~g >> 2) & 0x01;
	g = combine_3_weights(state->gweights, bit0, bit1, bit2);

	/* blue component (inverted) */
	bit0 = (~b >> 0) & 0x01;
	bit1 = (~b >> 1) & 0x01;
	bit2 = (~b >> 2) & 0x01;
	b = combine_3_weights(state->bweights, bit0, bit1, bit2);

	palette_set_color(space->machine, offset & 0x1f, MAKE_RGB(r, g, b));
}



/*************************************
 *
 *  Video RAM access via the write
 *  protect PROM
 *
 *************************************/

INLINE void ccastles_write_vram( running_machine *machine, UINT16 addr, UINT8 data, UINT8 bitmd, UINT8 pixba )
{
	ccastles_state *state = (ccastles_state *)machine->driver_data;
	UINT8 *dest = &state->videoram[addr & 0x7ffe];
	UINT8 promaddr = 0;
	UINT8 wpbits;

	/*
        Inputs to the write-protect PROM:

        Bit 7 = BA1520 = 0 if (BA15-BA12 != 0), or 1 otherwise
        Bit 6 = DRBA11
        Bit 5 = DRBA10
        Bit 4 = /BITMD
        Bit 3 = GND
        Bit 2 = BA0
        Bit 1 = PIXB
        Bit 0 = PIXA
    */
	promaddr |= ((addr & 0xf000) == 0) << 7;
	promaddr |= (addr & 0x0c00) >> 5;
	promaddr |= (!bitmd) << 4;
	promaddr |= (addr & 0x0001) << 2;
	promaddr |= (pixba << 0);

	/* look up the PROM result */
	wpbits = state->wpprom[promaddr];

	/* write to the appropriate parts of VRAM depending on the result */
	if (!(wpbits & 1))
		dest[0] = (dest[0] & 0xf0) | (data & 0x0f);
	if (!(wpbits & 2))
		dest[0] = (dest[0] & 0x0f) | (data & 0xf0);
	if (!(wpbits & 4))
		dest[1] = (dest[1] & 0xf0) | (data & 0x0f);
	if (!(wpbits & 8))
		dest[1] = (dest[1] & 0x0f) | (data & 0xf0);
}



/*************************************
 *
 *  Autoincrement control for bit mode
 *
 *************************************/

INLINE void bitmode_autoinc( running_machine *machine )
{
	ccastles_state *state = (ccastles_state *)machine->driver_data;

	/* auto increment in the x-direction if it's enabled */
	if (!state->video_control[0])	/* /AX */
	{
		if (!state->video_control[2])	/* /XINC */
			state->bitmode_addr[0]++;
		else
			state->bitmode_addr[0]--;
	}

	/* auto increment in the y-direction if it's enabled */
	if (!state->video_control[1])	/* /AY */
	{
		if (!state->video_control[3])	/* /YINC */
			state->bitmode_addr[1]++;
		else
			state->bitmode_addr[1]--;
	}
}



/*************************************
 *
 *  Standard video RAM access
 *
 *************************************/

WRITE8_HANDLER( ccastles_videoram_w )
{
	/* direct writes to VRAM go through the write protect PROM as well */
	ccastles_write_vram(space->machine, offset, data, 0, 0);
}



/*************************************
 *
 *  Bit mode video RAM access
 *
 *************************************/

READ8_HANDLER( ccastles_bitmode_r )
{
	ccastles_state *state = (ccastles_state *)space->machine->driver_data;

	/* in bitmode, the address comes from the autoincrement latches */
	UINT16 addr = (state->bitmode_addr[1] << 7) | (state->bitmode_addr[0] >> 1);

	/* the appropriate pixel is selected into the upper 4 bits */
	UINT8 result = state->videoram[addr] << ((~state->bitmode_addr[0] & 1) * 4);

	/* autoincrement because /BITMD was selected */
	bitmode_autoinc(space->machine);

	/* the low 4 bits of the data lines are not driven so make them all 1's */
	return result | 0x0f;
}


WRITE8_HANDLER( ccastles_bitmode_w )
{
	ccastles_state *state = (ccastles_state *)space->machine->driver_data;

	/* in bitmode, the address comes from the autoincrement latches */
	UINT16 addr = (state->bitmode_addr[1] << 7) | (state->bitmode_addr[0] >> 1);

	/* the upper 4 bits of data are replicated to the lower 4 bits */
	data = (data & 0xf0) | (data >> 4);

	/* write through the generic VRAM routine, passing the low 2 X bits as PIXB/PIXA */
	ccastles_write_vram(space->machine, addr, data, 1, state->bitmode_addr[0] & 3);

	/* autoincrement because /BITMD was selected */
	bitmode_autoinc(space->machine);
}


WRITE8_HANDLER( ccastles_bitmode_addr_w )
{
	ccastles_state *state = (ccastles_state *)space->machine->driver_data;

	/* write through to video RAM and also to the addressing latches */
	ccastles_write_vram(space->machine, offset, data, 0, 0);
	state->bitmode_addr[offset] = data;
}



/*************************************
 *
 *  Video updating
 *
 *************************************/

VIDEO_UPDATE( ccastles )
{
	ccastles_state *state = (ccastles_state *)screen->machine->driver_data;
	UINT8 *spriteaddr = &state->spriteram[state->video_control[7] * 0x100];	/* BUF1/BUF2 */
	int flip = state->video_control[4] ? 0xff : 0x00;	/* PLAYER2 */
	pen_t black = get_black_pen(screen->machine);
	int x, y, offs;

	/* draw the sprites */
	bitmap_fill(state->spritebitmap, cliprect, 0x0f);
	for (offs = 0; offs < 320/2; offs += 4)
	{
		int x = spriteaddr[offs + 3];
		int y = 256 - 16 - spriteaddr[offs + 1];
		int which = spriteaddr[offs];
		int color = spriteaddr[offs + 2] >> 7;

		drawgfx_transpen(state->spritebitmap, cliprect, screen->machine->gfx[0], which, color, flip, flip, x, y, 7);
	}

	/* draw the bitmap to the screen, looping over Y */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		UINT16 *dst = (UINT16 *)bitmap->base + y * bitmap->rowpixels;

		/* if we're in the VBLANK region, just fill with black */
		if (state->syncprom[y] & 1)
		{
			for (x = cliprect->min_x; x <= cliprect->max_x; x++)
				dst[x] = black;
		}

		/* non-VBLANK region: merge the sprites and the bitmap */
		else
		{
			UINT16 *mosrc = (UINT16 *)state->spritebitmap->base + y * state->spritebitmap->rowpixels;
			int effy = (((y - state->vblank_end) + (flip ? 0 : state->vscroll)) ^ flip) & 0xff;
			UINT8 *src;

			/* the "POTATO" chip does some magic here; this is just a guess */
			if (effy < 24)
				effy = 24;
			src = &state->videoram[effy * 128];

			/* loop over X */
			for (x = cliprect->min_x; x <= cliprect->max_x; x++)
			{
				/* if we're in the HBLANK region, just store black */
				if (x >= 256)
					dst[x] = black;

				/* otherwise, process normally */
				else
				{
					int effx = (state->hscroll + (x ^ flip)) & 255;

					/* low 4 bits = left pixel, high 4 bits = right pixel */
					UINT8 pix = (src[effx / 2] >> ((effx & 1) * 4)) & 0x0f;
					UINT8 mopix = mosrc[x];
					UINT8 prindex, prvalue;

					/* Inputs to the priority PROM:

                        Bit 7 = GND
                        Bit 6 = /CRAM
                        Bit 5 = BA4
                        Bit 4 = MV2
                        Bit 3 = MV1
                        Bit 2 = MV0
                        Bit 1 = MPI
                        Bit 0 = BIT3
                    */
					prindex = 0x40;
					prindex |= (mopix & 7) << 2;
					prindex |= (mopix & 8) >> 2;
					prindex |= (pix & 8) >> 3;
					prvalue = state->priprom[prindex];

					/* Bit 1 of prvalue selects the low 4 bits of the final pixel */
					if (prvalue & 2)
						pix = mopix;

					/* Bit 0 of prvalue selects bit 4 of the final color */
					pix |= (prvalue & 1) << 4;

					/* store the pixel value and also a priority value based on the topmost bit */
					dst[x] = pix;
				}
			}
		}
	}
	return 0;
}
