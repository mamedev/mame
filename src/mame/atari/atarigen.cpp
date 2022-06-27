// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    atarigen.cpp

    General functions for Atari games.

***************************************************************************/

#include "emu.h"
#include "atarigen.h"



/***************************************************************************
    OVERALL INIT
***************************************************************************/

atarigen_state::atarigen_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag)
	, m_maincpu(*this, "maincpu")
	, m_gfxdecode(*this, "gfxdecode")
	, m_screen(*this, "screen")
	, m_unhalt_cpu_timer(nullptr)
{
}

void atarigen_state::machine_start()
{
	m_unhalt_cpu_timer = timer_alloc(FUNC(atarigen_state::unhalt_cpu), this);
}


void atarigen_state::machine_reset()
{
}


TIMER_CALLBACK_MEMBER(atarigen_state::unhalt_cpu)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
}



/***************************************************************************
    VIDEO HELPERS
***************************************************************************/

//-------------------------------------------------
//  halt_until_hblank_0: Halts CPU 0 until the
//  next HBLANK.
//-------------------------------------------------

void atarigen_state::halt_until_hblank_0(device_t &device, screen_device &screen)
{
	// halt the CPU until the next HBLANK
	int hpos = screen.hpos();
	int width = screen.width();
	int hblank = width * 9 / 10;

	// if we're in hblank, set up for the next one
	if (hpos >= hblank)
		hblank += width;

	// halt and set a timer to wake up
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_unhalt_cpu_timer->adjust(screen.scan_period() * (hblank - hpos) / width);
}


/***************************************************************************
    MISC HELPERS
***************************************************************************/

//-------------------------------------------------
//  blend_gfx: Takes two GFXElements and blends their
//  data together to form one. Then frees the second.
//-------------------------------------------------

void atarigen_state::blend_gfx(int gfx0, int gfx1, int mask0, int mask1)
{
	gfx_element *gx0 = m_gfxdecode->gfx(gfx0);
	gfx_element *gx1 = m_gfxdecode->gfx(gfx1);

	// allocate memory for the assembled data
	m_blended_data = std::make_unique<u8[]>(gx0->elements() * gx0->width() * gx0->height());

	// loop over elements
	u8 *dest = m_blended_data.get();
	for (int c = 0; c < gx0->elements(); c++)
	{
		const u8 *c0base = gx0->get_data(c);
		const u8 *c1base = gx1->get_data(c);

		// loop over height
		for (int y = 0; y < gx0->height(); y++)
		{
			const u8 *c0 = c0base;
			const u8 *c1 = c1base;

			for (int x = 0; x < gx0->width(); x++)
				*dest++ = (*c0++ & mask0) | (*c1++ & mask1);
			c0base += gx0->rowbytes();
			c1base += gx1->rowbytes();
		}
	}

//  int newdepth = gx0->depth() * gx1->depth();
	int granularity = gx0->granularity();
	gx0->set_raw_layout(m_blended_data.get(), gx0->width(), gx0->height(), gx0->elements(), 8 * gx0->width(), 8 * gx0->width() * gx0->height());
	gx0->set_granularity(granularity);

	// free the second graphics element
	m_gfxdecode->set_gfx(gfx1, nullptr);
}
