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
	, m_slapstic_num(0)
	, m_slapstic(nullptr)
	, m_slapstic_bank(0)
	, m_slapstic_last_pc(0)
	, m_slapstic_last_address(0)
	, m_slapstic_base(0)
	, m_slapstic_mirror(0)
	, m_maincpu(*this, "maincpu")
	, m_gfxdecode(*this, "gfxdecode")
	, m_screen(*this, "screen")
	, m_slapstic_device(*this, ":slapstic")
{
}

void atarigen_state::machine_start()
{
	save_item(NAME(m_slapstic_num));
	save_item(NAME(m_slapstic_bank));
	save_item(NAME(m_slapstic_last_pc));
	save_item(NAME(m_slapstic_last_address));
}


void atarigen_state::machine_reset()
{
	// reset the slapstic
	if (m_slapstic_num != 0)
	{
		if (!m_slapstic_device.found())
			fatalerror("Slapstic device is missing?\n");

		m_slapstic_device->slapstic_reset();
		slapstic_update_bank(m_slapstic_device->slapstic_bank());
	}
}


void atarigen_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		// unhalt the CPU that was passed as a pointer
		case TID_UNHALT_CPU:
			reinterpret_cast<device_t *>(ptr)->execute().set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
			break;
	}
}


/***************************************************************************
    SLAPSTIC HANDLING
***************************************************************************/

inline void atarigen_state::slapstic_update_bank(int bank)
{
	// if the bank has changed, copy the memory; Pit Fighter needs this
	if (bank != m_slapstic_bank)
	{
		// bank 0 comes from the copy we made earlier
		if (bank == 0)
			memcpy(m_slapstic, &m_slapstic_bank0[0], 0x2000);
		else
			memcpy(m_slapstic, &m_slapstic[bank * 0x1000], 0x2000);

		// remember the current bank
		m_slapstic_bank = bank;
	}
}


void atarigen_state::device_post_load()
{
	if (m_slapstic_num != 0)
	{
		if (!m_slapstic_device.found())
			fatalerror("Slapstic device is missing?\n");

		slapstic_update_bank(m_slapstic_device->slapstic_bank());
	}
}


//-------------------------------------------------
//  slapstic_configure: Installs memory handlers for the
//  slapstic and sets the chip number.
//-------------------------------------------------

void atarigen_state::slapstic_configure(cpu_device &device, offs_t base, offs_t mirror, u8 *mem)
{
	if (!m_slapstic_device.found())
		fatalerror("Slapstic device is missing\n");

	// initialize the slapstic
	m_slapstic_num = m_slapstic_device->m_chipnum;
	m_slapstic_device->slapstic_init();

	// install the memory handlers
	address_space &program = device.space(AS_PROGRAM);
	program.install_readwrite_handler(base, base + 0x7fff, 0, mirror, 0, read16_delegate(*this, FUNC(atarigen_state::slapstic_r)), write16_delegate(*this, FUNC(atarigen_state::slapstic_w)));
	m_slapstic = (u16 *)mem;

	// allocate memory for a copy of bank 0
	m_slapstic_bank0.resize(0x2000);
	memcpy(&m_slapstic_bank0[0], m_slapstic, 0x2000);

	// ensure we recopy memory for the bank
	m_slapstic_bank = 0xff;

	// install an opcode base handler if we are a 68000 or variant
	m_slapstic_base = base;
	m_slapstic_mirror = mirror;
}


//-------------------------------------------------
//  slapstic_w: Assuming that the slapstic sits in
//  ROM memory space, we just simply tweak the slapstic at this
//  address and do nothing more.
//-------------------------------------------------

WRITE16_MEMBER(atarigen_state::slapstic_w)
{
	if (!m_slapstic_device.found())
		fatalerror("Slapstic device is missing?\n");

	slapstic_update_bank(m_slapstic_device->slapstic_tweak(space, offset));
}


//-------------------------------------------------
//  slapstic_r: Tweaks the slapstic at the appropriate
//  address and then reads a word from the underlying memory.
//-------------------------------------------------

READ16_MEMBER(atarigen_state::slapstic_r)
{
	if (!m_slapstic_device.found())
		fatalerror("Slapstic device is missing?\n");

	// fetch the result from the current bank first
	int result = m_slapstic[offset & 0xfff];

	if (!machine().side_effects_disabled())
	{
		// then determine the new one
		slapstic_update_bank(m_slapstic_device->slapstic_tweak(space, offset));
	}
	return result;
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
	device.execute().set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	timer_set(screen.scan_period() * (hblank - hpos) / width, TID_UNHALT_CPU, 0, (void *)&device);
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
	u8 *srcdata = auto_alloc_array(machine(), u8, gx0->elements() * gx0->width() * gx0->height());

	// loop over elements
	u8 *dest = srcdata;
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
	gx0->set_raw_layout(srcdata, gx0->width(), gx0->height(), gx0->elements(), 8 * gx0->width(), 8 * gx0->width() * gx0->height());
	gx0->set_granularity(granularity);

	// free the second graphics element
	m_gfxdecode->set_gfx(gfx1, nullptr);
}
