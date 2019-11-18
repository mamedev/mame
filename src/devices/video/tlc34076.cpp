// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    tlc34076.c

    Basic implementation of the TLC34076 palette chip and similar
    compatible chips.

***************************************************************************/

#include "emu.h"
#include "tlc34076.h"


//**************************************************************************
//  MACROS
//**************************************************************************

#define PALETTE_WRITE_ADDR  0x00
#define PALETTE_DATA        0x01
#define PIXEL_READ_MASK     0x02
#define PALETTE_READ_ADDR   0x03
#define GENERAL_CONTROL     0x08
#define INPUT_CLOCK_SEL     0x09
#define OUTPUT_CLOCK_SEL    0x0a
#define MUX_CONTROL         0x0b
#define PALETTE_PAGE        0x0c
#define TEST_REGISTER       0x0e
#define RESET_STATE         0x0f


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(TLC34076, tlc34076_device, "tlc34076", "TI TLC34076 VIP")


//-------------------------------------------------
//  tlc34076_device - constructor
//-------------------------------------------------
tlc34076_device::tlc34076_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TLC34076, tag, owner, clock)
	, device_palette_interface(mconfig, *this)
	, m_dacbits(6)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void tlc34076_device::device_start()
{
	for (int i = 0; i < 3; i++)
	{
		m_local_paletteram[i] = std::make_unique<uint8_t[]>(0x100);
		save_pointer(NAME(m_local_paletteram[i]), 0x100, i);
	}

	save_item(NAME(m_regs));

	save_item(NAME(m_writeindex));
	save_item(NAME(m_readindex));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tlc34076_device::device_reset()
{
	m_regs[PIXEL_READ_MASK]    = 0xff;
	m_regs[GENERAL_CONTROL]    = 0x03;
	m_regs[INPUT_CLOCK_SEL]    = 0x00;
	m_regs[OUTPUT_CLOCK_SEL]   = 0x3f;
	m_regs[MUX_CONTROL]        = 0x2d;
	m_regs[PALETTE_PAGE]       = 0x00;
	m_regs[TEST_REGISTER]      = 0x00;
	m_regs[RESET_STATE]        = 0x00;

	for (int i = 0; i < 0x100; i++)
		update_pen(i);
}


//**************************************************************************
//  PUBLIC INTERFACE
//**************************************************************************

//-------------------------------------------------
//  update_pen - update color in current palette
//-------------------------------------------------

void tlc34076_device::update_pen(uint8_t i)
{
	int r, g, b;

	if ((i & m_regs[PIXEL_READ_MASK]) == i)
	{
		r = m_local_paletteram[0][i];
		g = m_local_paletteram[1][i];
		b = m_local_paletteram[2][i];

		if (m_dacbits == 6)
		{
			r = pal6bit(r);
			g = pal6bit(g);
			b = pal6bit(b);
		}
	}
	else
	{
		r = 0;
		g = 0;
		b = 0;
	}

	set_pen_color(i, rgb_t(r, g, b));
}


//-------------------------------------------------
//  read - read access
//-------------------------------------------------

u8 tlc34076_device::read(offs_t offset)
{
	uint8_t result;

	/* keep in range */
	offset &= 0x0f;
	result = m_regs[offset];

	/* switch off the offset */
	switch (offset)
	{
		case PALETTE_DATA:
			if (m_readindex == 0)
			{
				for (int i = 0; i < 3; i++)
					m_palettedata[i] = m_local_paletteram[i][m_regs[PALETTE_READ_ADDR]];
			}
			result = m_palettedata[m_readindex++];
			if (m_readindex == 3)
			{
				m_readindex = 0;
				m_regs[PALETTE_READ_ADDR]++;
			}
			break;
	}

	return result;
}


//-------------------------------------------------
//  write - write access
//-------------------------------------------------

void tlc34076_device::write(offs_t offset, u8 data)
{
//  uint8_t oldval;

	/* keep in range */
	offset &= 0x0f;
//  oldval = m_regs[offset];
	m_regs[offset] = data;

	/* switch off the offset */
	switch (offset)
	{
		case PALETTE_WRITE_ADDR:
			m_writeindex = 0;
			break;

		case PALETTE_DATA:
			m_palettedata[m_writeindex++] = data;
			if (m_writeindex == 3)
			{
				for (int i = 0; i < 3; i++)
					m_local_paletteram[i][m_regs[PALETTE_WRITE_ADDR]] = m_palettedata[i];

				update_pen(m_regs[PALETTE_WRITE_ADDR]);

				m_writeindex = 0;
				m_regs[PALETTE_WRITE_ADDR]++;
			}
			break;

		case PIXEL_READ_MASK:
			for (int i = 0; i < 0x100; i++)
				update_pen(i);
			break;

		case PALETTE_READ_ADDR:
			m_readindex = 0;
			break;

		case GENERAL_CONTROL:
			/*
			    7 6 5 4 3 2 1 0
			    X X X X X X X 0 HSYNCOUT is active-low
			    X X X X X X X 1 HSYNCOUT is active-high (default)
			    X X X X X X 0 X VSYNCOUT is active-low
			    X X X X X X 1 X VSYNCOUT is active-high (default)
			    X X X X X 0 X X Disable split shift register transfer (default)
			    X X X X 0 1 X X Enable split shift register transfer
			    X X X X 0 X X X Disable special nibble mode (default)
			    X X X X 1 0 X X Enable special nibble mode
			    X X X 0 X X X X 0-IRE pedestal (default)
			    X X X 1 X X X X 7.5-IRE pedestal
			    X X 0 X X X X X Disable sync (default)
			    X X 1 X X X X X Enable sync
			    X 0 X X X X X X Little-endian mode (default)
			    X 1 X X X X X X Big-endian mode
			    0 X X X X X X X MUXOUT is low (default)
			    1 X X X X X X X MUXOUT is high
			*/
			break;

		case INPUT_CLOCK_SEL:
			/*
			    3 2 1 0
			    0 0 0 0 Select CLK0 as clock source?
			    0 0 0 1 Select CLK1 as clock source
			    0 0 1 0 Select CLK2 as clock source
			    0 0 1 1 Select CLK3 as TTL clock source
			    0 1 0 0 Select CLK3 as TTL clock source
			    1 0 0 0 Select CLK3 and CLK3 as ECL clock sources
			*/
			break;

		case OUTPUT_CLOCK_SEL:
			/*
			    0 0 0 X X X VCLK frequency = DOTCLK frequency
			    0 0 1 X X X VCLK frequency = DOTCLK frequency/2
			    0 1 0 X X X VCLK frequency = DOTCLK frequency/4
			    0 1 1 X X X VCLK frequency = DOTCLK frequency/8
			    1 0 0 X X X VCLK frequency = DOTCLK frequency/16
			    1 0 1 X X X VCLK frequency = DOTCLK frequency/32
			    1 1 X X X X VCLK output held at logic high level (default condition)
			    X X X 0 0 0 SCLK frequency = DOTCLK frequency
			    X X X 0 0 1 SCLK frequency = DOTCLK frequency/2
			    X X X 0 1 0 SCLK frequency = DOTCLK frequency/4
			    X X X 0 1 1 SCLK frequency = DOTCLK frequency/8
			    X X X 1 0 0 SCLK frequency = DOTCLK frequency/16
			    X X X 1 0 1 SCLK frequency = DOTCLK frequency/32
			    X X X 1 1 X SCLK output held at logic level low (default condition)
			*/
			break;

		case RESET_STATE:
			device_reset();
			break;
	}
}
