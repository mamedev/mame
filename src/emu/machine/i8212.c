/**********************************************************************

    Intel 8212 8-Bit Input/Output Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

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

i8212_device::i8212_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, I8212, "Intel 8212", tag, owner, clock),
	  m_md(I8212_MODE_INPUT),
	  m_stb(0)
{

}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void i8212_device::device_config_complete()
{
	// inherit a copy of the static data
	const i8212_interface *intf = reinterpret_cast<const i8212_interface *>(static_config());
	if (intf != NULL)
		*static_cast<i8212_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_int_cb, 0, sizeof(m_out_int_cb));
		memset(&m_in_di_cb, 0, sizeof(m_in_di_cb));
		memset(&m_out_do_cb, 0, sizeof(m_out_do_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8212_device::device_start()
{
	// resolve callbacks
	m_out_int_func.resolve(m_out_int_cb, *this);
	m_in_di_func.resolve(m_in_di_cb, *this);
	m_out_do_func.resolve(m_out_do_cb, *this);

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
		m_out_do_func(0, m_data);
	}
}


//-------------------------------------------------
//  data_r - data latch read
//-------------------------------------------------

READ8_MEMBER( i8212_device::data_r )
{
	// clear interrupt line
	m_out_int_func(CLEAR_LINE);

	if (LOG) logerror("I8212 '%s' INT: %u\n", tag(), CLEAR_LINE);

	return m_data;
}


//-------------------------------------------------
//  data_w - data latch write
//-------------------------------------------------

WRITE8_MEMBER( i8212_device::data_w )
{
	// latch data
	m_data = data;

	// output data
	m_out_do_func(0, m_data);
}


//-------------------------------------------------
//  md_w - mode write
//-------------------------------------------------

WRITE_LINE_MEMBER( i8212_device::md_w )
{
	if (LOG) logerror("I8212 '%s' Mode: %s\n", tag(), state ? "output" : "input");

	m_md = state;
}


//-------------------------------------------------
//  stb_w - data strobe write
//-------------------------------------------------

WRITE_LINE_MEMBER( i8212_device::stb_w )
{
	if (LOG) logerror("I8212 '%s' STB: %u\n", tag(), state);

	if (m_md == I8212_MODE_INPUT)
	{
		if (m_stb && !state)
		{
			// input data
			m_data = m_in_di_func(0);

			// assert interrupt line
			m_out_int_func(ASSERT_LINE);

			if (LOG) logerror("I8212 '%s' INT: %u\n", tag(), ASSERT_LINE);
		}
	}

	m_stb = state;
}
