// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intel 8212 8-Bit Input/Output Port emulation

**********************************************************************/

#include "emu.h"
#include "i8212.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type I8212 = &device_creator<i8212_device>;

//-------------------------------------------------
//  i8212_device - constructor
//-------------------------------------------------

i8212_device::i8212_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, I8212, "I8212", tag, owner, clock, "i8212", __FILE__),
	m_write_irq(*this),
	m_read_di(*this),
	m_write_do(*this),
	m_md(I8212_MODE_INPUT),
	m_stb(0), m_data(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8212_device::device_start()
{
	// resolve callbacks
	m_write_irq.resolve_safe();
	m_read_di.resolve_safe(0);
	m_write_do.resolve_safe();

	// register for state saving
	save_item(NAME(m_md));
	save_item(NAME(m_stb));
	save_item(NAME(m_data));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8212_device::device_reset()
{
	m_data = 0;

	if (m_md == I8212_MODE_OUTPUT)
	{
		// output data
		m_write_do((offs_t)0, m_data);
	}
}


//-------------------------------------------------
//  read - data latch read
//-------------------------------------------------

READ8_MEMBER( i8212_device::read )
{
	// clear interrupt line
	m_write_irq(CLEAR_LINE);

	if (LOG) logerror("I8212 '%s' INT: %u\n", tag().c_str(), CLEAR_LINE);

	return m_data;
}


//-------------------------------------------------
//  write - data latch write
//-------------------------------------------------

WRITE8_MEMBER( i8212_device::write )
{
	// latch data
	m_data = data;

	// output data
	m_write_do((offs_t)0, m_data);
}


//-------------------------------------------------
//  md_w - mode write
//-------------------------------------------------

WRITE_LINE_MEMBER( i8212_device::md_w )
{
	if (LOG) logerror("I8212 '%s' Mode: %s\n", tag().c_str(), state ? "output" : "input");

	m_md = state;
}


//-------------------------------------------------
//  stb_w - data strobe write
//-------------------------------------------------

WRITE_LINE_MEMBER( i8212_device::stb_w )
{
	if (LOG) logerror("I8212 '%s' STB: %u\n", tag().c_str(), state);

	if (m_md == I8212_MODE_INPUT)
	{
		if (m_stb && !state)
		{
			// input data
			m_data = m_read_di(0);

			// assert interrupt line
			m_write_irq(ASSERT_LINE);

			if (LOG) logerror("I8212 '%s' INT: %u\n", tag().c_str(), ASSERT_LINE);
		}
	}

	m_stb = state;
}
