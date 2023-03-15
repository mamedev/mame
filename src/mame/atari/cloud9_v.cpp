// license:BSD-3-Clause
// copyright-holders:Mike Balfour, Aaron Giles
/***************************************************************************

    Atari Cloud 9 (prototype) hardware

***************************************************************************/

#include "emu.h"
#include "cloud9.h"
#include "video/resnet.h"


/*************************************
 *
 *  Video startup
 *
 *************************************/

void cloud9_state::video_start()
{
	static const int resistances[3] = { 22000, 10000, 4700 };

	/* allocate second bank of videoram */
	m_videoram = std::make_unique<uint8_t[]>(0x8000);
	membank("bank1")->set_base(m_videoram.get());

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
	save_pointer(NAME(m_videoram), 0x8000);
	save_item(NAME(m_bitmode_addr));
}



/*************************************
 *
 *  Palette RAM accesses
 *
 *************************************/

void cloud9_state::cloud9_paletteram_w(offs_t offset, uint8_t data)
{
	int bit0, bit1, bit2;
	int r, g, b;

	/* extract the raw RGB bits */
	r = (data & 0xe0) >> 5;
	g = (data & 0x1c) >> 2;
	b = ((data & 0x03) << 1) | ((offset & 0x40) >> 6);

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

	m_palette->set_pen_color(offset & 0x3f, rgb_t(r, g, b));
}



/*************************************
 *
 *  Video RAM access via the write
 *  protect PROM
 *
 *************************************/

inline void cloud9_state::cloud9_write_vram( uint16_t addr, uint8_t data, uint8_t bitmd, uint8_t pixba )
{
	uint8_t *dest = &m_videoram[0x0000 | (addr & 0x3fff)];
	uint8_t *dest2 = &m_videoram[0x4000 | (addr & 0x3fff)];
	uint8_t promaddr = 0;
	uint8_t wpbits;

	/*
	    Inputs to the write-protect PROM:

	    Bit 7 = BITMD
	    Bit 6 = video_control[4]
	    Bit 5 = video_control[6]
	    Bit 4 = 1 if (A15-A12 != 4)
	    Bit 3 = !(A13 | A12 | A11)
	    Bit 2 = A9 & A10
	    Bit 1 = PIXB
	    Bit 0 = PIXA
	*/
	promaddr |= bitmd << 7;
	promaddr |= m_videolatch->q4_r() << 6;
	promaddr |= m_videolatch->q6_r() << 5;
	promaddr |= ((addr & 0xf000) != 0x4000) << 4;
	promaddr |= ((addr & 0x3800) == 0x0000) << 3;
	promaddr |= ((addr & 0x0600) == 0x0600) << 2;
	promaddr |= (pixba << 0);

	/* look up the PROM result */
	wpbits = m_wpprom[promaddr];

	/* write to the appropriate parts of VRAM depending on the result */
	if (!(wpbits & 1))
		dest2[0] = (dest2[0] & 0x0f) | (data & 0xf0);
	if (!(wpbits & 2))
		dest2[0] = (dest2[0] & 0xf0) | (data & 0x0f);
	if (!(wpbits & 4))
		dest[0] = (dest[0] & 0x0f) | (data & 0xf0);
	if (!(wpbits & 8))
		dest[0] = (dest[0] & 0xf0) | (data & 0x0f);
}



/*************************************
 *
 *  Autoincrement control for bit mode
 *
 *************************************/

inline void cloud9_state::bitmode_autoinc(  )
{
	/* auto increment in the x-direction if it's enabled */
	if (!m_videolatch->q0_r()) /* /AX */
		m_bitmode_addr[0]++;

	/* auto increment in the y-direction if it's enabled */
	if (!m_videolatch->q1_r()) /* /AY */
		m_bitmode_addr[1]++;
}



/*************************************
 *
 *  Standard video RAM access
 *
 *************************************/

void cloud9_state::cloud9_videoram_w(offs_t offset, uint8_t data)
{
	/* direct writes to VRAM go through the write protect PROM as well */
	cloud9_write_vram(offset, data, 0, 0);
}



/*************************************
 *
 *  Bit mode video RAM access
 *
 *************************************/

uint8_t cloud9_state::cloud9_bitmode_r()
{
	/* in bitmode, the address comes from the autoincrement latches */
	uint16_t addr = (m_bitmode_addr[1] << 6) | (m_bitmode_addr[0] >> 2);

	/* the appropriate pixel is selected into the upper 4 bits */
	uint8_t result = m_videoram[((~m_bitmode_addr[0] & 2) << 13) | addr] << ((m_bitmode_addr[0] & 1) * 4);

	/* autoincrement because /BITMD was selected */
	bitmode_autoinc();

	/* the upper 4 bits of the data lines are not driven so make them all 1's */
	return (result >> 4) | 0xf0;
}


void cloud9_state::cloud9_bitmode_w(uint8_t data)
{
	/* in bitmode, the address comes from the autoincrement latches */
	uint16_t addr = (m_bitmode_addr[1] << 6) | (m_bitmode_addr[0] >> 2);

	/* the lower 4 bits of data are replicated to the upper 4 bits */
	data = (data & 0x0f) | (data << 4);

	/* write through the generic VRAM routine, passing the low 2 X bits as PIXB/PIXA */
	cloud9_write_vram(addr, data, 1, m_bitmode_addr[0] & 3);

	/* autoincrement because /BITMD was selected */
	bitmode_autoinc();
}


void cloud9_state::cloud9_bitmode_addr_w(offs_t offset, uint8_t data)
{
	/* write through to video RAM and also to the addressing latches */
	cloud9_write_vram(offset, data, 0, 0);
	m_bitmode_addr[offset] = data;
}



/*************************************
 *
 *  Video updating
 *
 *************************************/

uint32_t cloud9_state::screen_update_cloud9(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t const *const spriteaddr = m_spriteram;
	int const flip = m_videolatch->q5_r() ? 0xff : 0x00;    /* PLAYER2 */
	pen_t const black = m_palette->black_pen();

	/* draw the sprites */
	m_spritebitmap.fill(0x00, cliprect);
	for (int offs = 0; offs < 0x20; offs++)
		if (spriteaddr[offs + 0x00] != 0)
		{
			int const x = spriteaddr[offs + 0x60];
			int const y = 256 - 15 - spriteaddr[offs + 0x00];
			int const xflip = spriteaddr[offs + 0x40] & 0x80;
			int const yflip = spriteaddr[offs + 0x40] & 0x40;
			int const which = spriteaddr[offs + 0x20];
			int const color = 0;

			m_gfxdecode->gfx(0)->transpen(m_spritebitmap,cliprect, which, color, xflip, yflip, x, y, 0);
			if (x >= 256 - 16)
				m_gfxdecode->gfx(0)->transpen(m_spritebitmap,cliprect, which, color, xflip, yflip, x - 256, y, 0);
		}

	/* draw the bitmap to the screen, looping over Y */
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		uint16_t *const dst = &bitmap.pix(y);

		/* if we're in the VBLANK region, just fill with black */
		if (~m_syncprom[y] & 2)
		{
			for (int x = cliprect.left(); x <= cliprect.right(); x++)
				dst[x] = black;
		}

		/* non-VBLANK region: merge the sprites and the bitmap */
		else
		{
			uint16_t const *const mosrc = &m_spritebitmap.pix(y);
			int const effy = y ^ flip;

			/* two videoram arrays */
			uint8_t const *const src[2]{ &m_videoram[0x4000 | (effy * 64)], &m_videoram[0x0000 | (effy * 64)] };

			/* loop over X */
			for (int x = cliprect.left(); x <= cliprect.right(); x++)
			{
				/* if we're in the HBLANK region, just store black */
				if (x >= 256)
					dst[x] = black;

				/* otherwise, process normally */
				else
				{
					int const effx = x ^ flip;

					/* low 4 bits = left pixel, high 4 bits = right pixel */
					uint8_t pix = (src[(effx >> 1) & 1][effx / 4] >> ((~effx & 1) * 4)) & 0x0f;
					uint8_t const mopix = mosrc[x];

					/* sprites have priority if sprite pixel != 0 or some other condition */
					if (mopix != 0)
						pix = mopix | 0x10;

					/* the high bit is the bank select */
					pix |= m_videolatch->q7_r() << 5;

					/* store the pixel value and also a priority value based on the topmost bit */
					dst[x] = pix;
				}
			}
		}
	}
	return 0;
}
