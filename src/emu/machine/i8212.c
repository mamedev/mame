/**********************************************************************

    Intel 8212 8-Bit Input/Output Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "emu.h"
#include "machine/devhelpr.h"
#include "i8212.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
const device_type I8212 = i8212_device_config::static_alloc_device_config;



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

GENERIC_DEVICE_CONFIG_SETUP(i8212, "Intel 8212")


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void i8212_device_config::device_config_complete()
{
	// inherit a copy of the static data
	const i8212_interface *intf = reinterpret_cast<const i8212_interface *>(static_config());
	if (intf != NULL)
		*static_cast<i8212_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&out_int_func, 0, sizeof(out_int_func));
		memset(&in_di_func, 0, sizeof(in_di_func));
		memset(&out_do_func, 0, sizeof(out_do_func));
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  i8212_device - constructor
//-------------------------------------------------

i8212_device::i8212_device(running_machine &_machine, const i8212_device_config &config)
    : device_t(_machine, config),
	  m_md(I8212_MODE_INPUT),
	  m_stb(0),
      m_config(config)
{

}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8212_device::device_start()
{
	// resolve callbacks
	devcb_resolve_write_line(&m_out_int_func, &m_config.out_int_func, this);
	devcb_resolve_read8(&m_in_di_func, &m_config.in_di_func, this);
	devcb_resolve_write8(&m_out_do_func, &m_config.out_do_func, this);

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
		devcb_call_write8(&m_out_do_func, 0, m_data);
	}
}


//-------------------------------------------------
//  data_r - data latch read
//-------------------------------------------------

READ8_MEMBER( i8212_device::data_r )
{
	// clear interrupt line
	devcb_call_write_line(&m_out_int_func, CLEAR_LINE);

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
	devcb_call_write8(&m_out_do_func, 0, m_data);
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
			m_data = devcb_call_read8(&m_in_di_func, 0);

			// assert interrupt line
			devcb_call_write_line(&m_out_int_func, ASSERT_LINE);

			if (LOG) logerror("I8212 '%s' INT: %u\n", tag(), ASSERT_LINE);
		}
	}

	m_stb = state;
}
