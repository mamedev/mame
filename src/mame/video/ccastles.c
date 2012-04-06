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
	ccastles_state *state = machine.driver_data<ccastles_state>();
	static const int resistances[3] = { 22000, 10000, 4700 };

	/* get pointers to our PROMs */
	state->m_syncprom = machine.region("proms")->base() + 0x000;
	state->m_wpprom = machine.region("proms")->base() + 0x200;
	state->m_priprom = machine.region("proms")->base() + 0x300;

	/* compute the color output resistor weights at startup */
	compute_resistor_weights(0,	255, -1.0,
			3,	resistances, state->m_rweights, 1000, 0,
			3,	resistances, state->m_gweights, 1000, 0,
			3,	resistances, state->m_bweights, 1000, 0);

	/* allocate a bitmap for drawing sprites */
	machine.primary_screen->register_screen_bitmap(state->m_spritebitmap);

	/* register for savestates */
	state->save_item(NAME(state->m_video_control));
	state->save_item(NAME(state->m_bitmode_addr));
	state->save_item(NAME(state->m_hscroll));
	state->save_item(NAME(state->m_vscroll));
}



/*************************************
 *
 *  Video control registers
 *
 *************************************/

WRITE8_MEMBER(ccastles_state::ccastles_hscroll_w)
{
	machine().primary_screen->update_partial(machine().primary_screen->vpos());
	m_hscroll = data;
}


WRITE8_MEMBER(ccastles_state::ccastles_vscroll_w)
{
	m_vscroll = data;
}


WRITE8_MEMBER(ccastles_state::ccastles_video_control_w)
{
	/* only D3 matters */
	m_video_control[offset] = (data >> 3) & 1;
}



/*************************************
 *
 *  Palette RAM accesses
 *
 *************************************/

WRITE8_MEMBER(ccastles_state::ccastles_paletteram_w)
{
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
	r = combine_3_weights(m_rweights, bit0, bit1, bit2);

	/* green component (inverted) */
	bit0 = (~g >> 0) & 0x01;
	bit1 = (~g >> 1) & 0x01;
	bit2 = (~g >> 2) & 0x01;
	g = combine_3_weights(m_gweights, bit0, bit1, bit2);

	/* blue component (inverted) */
	bit0 = (~b >> 0) & 0x01;
	bit1 = (~b >> 1) & 0x01;
	bit2 = (~b >> 2) & 0x01;
	b = combine_3_weights(m_bweights, bit0, bit1, bit2);

	palette_set_color(machine(), offset & 0x1f, MAKE_RGB(r, g, b));
}



/*************************************
 *
 *  Video RAM access via the write
 *  protect PROM
 *
 *************************************/

INLINE void ccastles_write_vram( running_machine &machine, UINT16 addr, UINT8 data, UINT8 bitmd, UINT8 pixba )
{
	ccastles_state *state = machine.driver_data<ccastles_state>();
	UINT8 *dest = &state->m_videoram[addr & 0x7ffe];
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
	wpbits = state->m_wpprom[promaddr];

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

INLINE void bitmode_autoinc( running_machine &machine )
{
	ccastles_state *state = machine.driver_data<ccastles_state>();

	/* auto increment in the x-direction if it's enabled */
	if (!state->m_video_control[0])	/* /AX */
	{
		if (!state->m_video_control[2])	/* /XINC */
			state->m_bitmode_addr[0]++;
		else
			state->m_bitmode_addr[0]--;
	}

	/* auto increment in the y-direction if it's enabled */
	if (!state->m_video_control[1])	/* /AY */
	{
		if (!state->m_video_control[3])	/* /YINC */
			state->m_bitmode_addr[1]++;
		else
			state->m_bitmode_addr[1]--;
	}
}



/*************************************
 *
 *  Standard video RAM access
 *
 *************************************/

WRITE8_MEMBER(ccastles_state::ccastles_videoram_w)
{
	/* direct writes to VRAM go through the write protect PROM as well */
	ccastles_write_vram(machine(), offset, data, 0, 0);
}



/*************************************
 *
 *  Bit mode video RAM access
 *
 *************************************/

READ8_MEMBER(ccastles_state::ccastles_bitmode_r)
{

	/* in bitmode, the address comes from the autoincrement latches */
	UINT16 addr = (m_bitmode_addr[1] << 7) | (m_bitmode_addr[0] >> 1);

	/* the appropriate pixel is selected into the upper 4 bits */
	UINT8 result = m_videoram[addr] << ((~m_bitmode_addr[0] & 1) * 4);

	/* autoincrement because /BITMD was selected */
	bitmode_autoinc(machine());

	/* the low 4 bits of the data lines are not driven so make them all 1's */
	return result | 0x0f;
}


WRITE8_MEMBER(ccastles_state::ccastles_bitmode_w)
{

	/* in bitmode, the address comes from the autoincrement latches */
	UINT16 addr = (m_bitmode_addr[1] << 7) | (m_bitmode_addr[0] >> 1);

	/* the upper 4 bits of data are replicated to the lower 4 bits */
	data = (data & 0xf0) | (data >> 4);

	/* write through the generic VRAM routine, passing the low 2 X bits as PIXB/PIXA */
	ccastles_write_vram(machine(), addr, data, 1, m_bitmode_addr[0] & 3);

	/* autoincrement because /BITMD was selected */
	bitmode_autoinc(machine());
}


WRITE8_MEMBER(ccastles_state::ccastles_bitmode_addr_w)
{

	/* write through to video RAM and also to the addressing latches */
	ccastles_write_vram(machine(), offset, data, 0, 0);
	m_bitmode_addr[offset] = data;
}



/*************************************
 *
 *  Video updating
 *
 *************************************/

SCREEN_UPDATE_IND16( ccastles )
{
	ccastles_state *state = screen.machine().driver_data<ccastles_state>();
	UINT8 *spriteaddr = &state->m_spriteram[state->m_video_control[7] * 0x100];	/* BUF1/BUF2 */
	int flip = state->m_video_control[4] ? 0xff : 0x00;	/* PLAYER2 */
	pen_t black = get_black_pen(screen.machine());
	int x, y, offs;

	/* draw the sprites */
	state->m_spritebitmap.fill(0x0f, cliprect);
	for (offs = 0; offs < 320/2; offs += 4)
	{
		int x = spriteaddr[offs + 3];
		int y = 256 - 16 - spriteaddr[offs + 1];
		int which = spriteaddr[offs];
		int color = spriteaddr[offs + 2] >> 7;

		drawgfx_transpen(state->m_spritebitmap, cliprect, screen.machine().gfx[0], which, color, flip, flip, x, y, 7);
	}

	/* draw the bitmap to the screen, looping over Y */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT16 *dst = &bitmap.pix16(y);

		/* if we're in the VBLANK region, just fill with black */
		if (state->m_syncprom[y] & 1)
		{
			for (x = cliprect.min_x; x <= cliprect.max_x; x++)
				dst[x] = black;
		}

		/* non-VBLANK region: merge the sprites and the bitmap */
		else
		{
			UINT16 *mosrc = &state->m_spritebitmap.pix16(y);
			int effy = (((y - state->m_vblank_end) + (flip ? 0 : state->m_vscroll)) ^ flip) & 0xff;
			UINT8 *src;

			/* the "POTATO" chip does some magic here; this is just a guess */
			if (effy < 24)
				effy = 24;
			src = &state->m_videoram[effy * 128];

			/* loop over X */
			for (x = cliprect.min_x; x <= cliprect.max_x; x++)
			{
				/* if we're in the HBLANK region, just store black */
				if (x >= 256)
					dst[x] = black;

				/* otherwise, process normally */
				else
				{
					int effx = (state->m_hscroll + (x ^ flip)) & 255;

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
					prvalue = state->m_priprom[prindex];

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
