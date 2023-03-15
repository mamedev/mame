// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari Crystal Castles hardware

***************************************************************************/

#include "emu.h"
#include "ccastles.h"
#include "video/resnet.h"


/*************************************
 *
 *  Video startup
 *
 *************************************/

void ccastles_state::video_start()
{
	static const int resistances[3] = { 22000, 10000, 4700 };

	/* get pointers to our PROMs */
	m_syncprom = memregion("proms")->base() + 0x000;
	m_wpprom = memregion("proms")->base() + 0x200;
	m_priprom = memregion("proms")->base() + 0x300;

	/* compute the color output resistor weights at startup */
	compute_resistor_weights(0, 255, -1.0,
			3,  resistances, m_rweights, 1000, 0,
			3,  resistances, m_gweights, 1000, 0,
			3,  resistances, m_bweights, 1000, 0);

	/* allocate a bitmap for drawing sprites */
	m_screen->register_screen_bitmap(m_spritebitmap);

	/* register for savestates */
	save_item(NAME(m_bitmode_addr));
	save_item(NAME(m_hscroll));
	save_item(NAME(m_vscroll));
}



/*************************************
 *
 *  Video control registers
 *
 *************************************/

void ccastles_state::ccastles_hscroll_w(uint8_t data)
{
	m_screen->update_partial(m_screen->vpos());
	m_hscroll = data;
}


void ccastles_state::ccastles_vscroll_w(uint8_t data)
{
	m_vscroll = data;
}


void ccastles_state::ccastles_video_control_w(offs_t offset, uint8_t data)
{
	/* only D3 matters */
	m_outlatch[1]->write_bit(offset, BIT(data, 3));
}



/*************************************
 *
 *  Palette RAM accesses
 *
 *************************************/

void ccastles_state::ccastles_paletteram_w(offs_t offset, uint8_t data)
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
	r = combine_weights(m_rweights, bit0, bit1, bit2);

	/* green component (inverted) */
	bit0 = (~g >> 0) & 0x01;
	bit1 = (~g >> 1) & 0x01;
	bit2 = (~g >> 2) & 0x01;
	g = combine_weights(m_gweights, bit0, bit1, bit2);

	/* blue component (inverted) */
	bit0 = (~b >> 0) & 0x01;
	bit1 = (~b >> 1) & 0x01;
	bit2 = (~b >> 2) & 0x01;
	b = combine_weights(m_bweights, bit0, bit1, bit2);

	m_palette->set_pen_color(offset & 0x1f, rgb_t(r, g, b));
}



/*************************************
 *
 *  Video RAM access via the write
 *  protect PROM
 *
 *************************************/

inline void ccastles_state::ccastles_write_vram( uint16_t addr, uint8_t data, uint8_t bitmd, uint8_t pixba )
{
	uint8_t *dest = &m_videoram[addr & 0x7ffe];
	uint8_t promaddr = 0;
	uint8_t wpbits;

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
	wpbits = m_wpprom[promaddr];

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

inline void ccastles_state::bitmode_autoinc(  )
{
	/* auto increment in the x-direction if it's enabled */
	if (!m_outlatch[1]->q0_r()) /* /AX */
	{
		if (!m_outlatch[1]->q2_r()) /* /XINC */
			m_bitmode_addr[0]++;
		else
			m_bitmode_addr[0]--;
	}

	/* auto increment in the y-direction if it's enabled */
	if (!m_outlatch[1]->q1_r()) /* /AY */
	{
		if (!m_outlatch[1]->q3_r()) /* /YINC */
			m_bitmode_addr[1]++;
		else
			m_bitmode_addr[1]--;
	}
}



/*************************************
 *
 *  Standard video RAM access
 *
 *************************************/

void ccastles_state::ccastles_videoram_w(offs_t offset, uint8_t data)
{
	/* direct writes to VRAM go through the write protect PROM as well */
	ccastles_write_vram(offset, data, 0, 0);
}



/*************************************
 *
 *  Bit mode video RAM access
 *
 *************************************/

uint8_t ccastles_state::ccastles_bitmode_r()
{
	/* in bitmode, the address comes from the autoincrement latches */
	uint16_t addr = (m_bitmode_addr[1] << 7) | (m_bitmode_addr[0] >> 1);

	/* the appropriate pixel is selected into the upper 4 bits */
	uint8_t result = m_videoram[addr] << ((~m_bitmode_addr[0] & 1) * 4);

	/* autoincrement because /BITMD was selected */
	bitmode_autoinc();

	/* the low 4 bits of the data lines are not driven so make them all 1's */
	return result | 0x0f;
}


void ccastles_state::ccastles_bitmode_w(uint8_t data)
{
	/* in bitmode, the address comes from the autoincrement latches */
	uint16_t addr = (m_bitmode_addr[1] << 7) | (m_bitmode_addr[0] >> 1);

	/* the upper 4 bits of data are replicated to the lower 4 bits */
	data = (data & 0xf0) | (data >> 4);

	/* write through the generic VRAM routine, passing the low 2 X bits as PIXB/PIXA */
	ccastles_write_vram(addr, data, 1, m_bitmode_addr[0] & 3);

	/* autoincrement because /BITMD was selected */
	bitmode_autoinc();
}


void ccastles_state::ccastles_bitmode_addr_w(offs_t offset, uint8_t data)
{
	/* write through to video RAM and also to the addressing latches */
	ccastles_write_vram(offset, data, 0, 0);
	m_bitmode_addr[offset] = data;
}



/*************************************
 *
 *  Video updating
 *
 *************************************/

uint32_t ccastles_state::screen_update_ccastles(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t const *const spriteaddr = &m_spriteram[m_outlatch[1]->q7_r() * 0x100];   /* BUF1/BUF2 */
	int const flip = m_outlatch[1]->q4_r() ? 0xff : 0x00;    /* PLAYER2 */
	pen_t const black = m_palette->black_pen();

	/* draw the sprites */
	m_spritebitmap.fill(0x0f, cliprect);
	for (int offs = 0; offs < 320/2; offs += 4)
	{
		int const x = spriteaddr[offs + 3];
		int const y = 256 - 16 - spriteaddr[offs + 1];
		int const which = spriteaddr[offs];
		int const color = spriteaddr[offs + 2] >> 7;

		m_gfxdecode->gfx(0)->transpen(m_spritebitmap,cliprect, which, color, flip, flip, x, y, 7);
	}

	/* draw the bitmap to the screen, looping over Y */
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		uint16_t *const dst = &bitmap.pix(y);

		/* if we're in the VBLANK region, just fill with black */
		if (m_syncprom[y] & 1)
		{
			for (int x = cliprect.left(); x <= cliprect.right(); x++)
				dst[x] = black;
		}

		/* non-VBLANK region: merge the sprites and the bitmap */
		else
		{
			uint16_t const *const mosrc = &m_spritebitmap.pix(y);

			/* the "POTATO" chip does some magic here; this is just a guess */
			int effy = (((y - m_vblank_end) + (flip ? 0 : m_vscroll)) ^ flip) & 0xff;
			if (effy < 24)
				effy = 24;
			uint8_t const *const src = &m_videoram[effy * 128];

			/* loop over X */
			for (int x = cliprect.left(); x <= cliprect.right(); x++)
			{
				/* if we're in the HBLANK region, just store black */
				if (x >= 256)
					dst[x] = black;

				/* otherwise, process normally */
				else
				{
					int const effx = (m_hscroll + (x ^ flip)) & 255;

					/* low 4 bits = left pixel, high 4 bits = right pixel */
					uint8_t pix = (src[effx / 2] >> ((effx & 1) * 4)) & 0x0f;
					uint8_t const mopix = mosrc[x];

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
					uint8_t prindex;
					prindex = 0x40;
					prindex |= (mopix & 7) << 2;
					prindex |= (mopix & 8) >> 2;
					prindex |= (pix & 8) >> 3;
					uint8_t const prvalue = m_priprom[prindex];

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
