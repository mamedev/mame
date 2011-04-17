/**********************************************************************

    OKI MSM58321RS Real Time Clock/Calendar emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

/*

    TODO:

    - count
    - stop
    - reset
    - reference registers

*/

#include "msm58321.h"
#include "machine/devhelpr.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 1


// registers
enum
{
	REGISTER_S1 = 0,
	REGISTER_S10,
	REGISTER_MI1,
	REGISTER_MI10,
	REGISTER_H1,
	REGISTER_H10,
	REGISTER_W,
	REGISTER_D1,
	REGISTER_D10,
	REGISTER_MO1,
	REGISTER_MO10,
	REGISTER_Y1,
	REGISTER_Y10,
	REGISTER_RESET,
	REGISTER_REF0,
	REGISTER_REF1
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
const device_type MSM58321 = msm58321_device_config::static_alloc_device_config;



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

GENERIC_DEVICE_CONFIG_SETUP(msm58321, "MSM58321")


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void msm58321_device_config::device_config_complete()
{
	// inherit a copy of the static data
	const msm58321_interface *intf = reinterpret_cast<const msm58321_interface *>(static_config());
	if (intf != NULL)
		*static_cast<msm58321_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_busy_func, 0, sizeof(m_out_busy_func));
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  msm58321_device - constructor
//-------------------------------------------------

msm58321_device::msm58321_device(running_machine &_machine, const msm58321_device_config &config)
    : device_t(_machine, config),
      m_config(config)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void msm58321_device::device_start()
{
	// resolve callbacks
	devcb_resolve_write_line(&m_out_busy_func, &m_config.m_out_busy_func, this);

	// allocate timers
	m_busy_timer = timer_alloc();
	m_busy_timer->adjust(attotime::from_hz(2), 0, attotime::from_hz(2));

	// state saving
	save_item(NAME(m_cs1));
	save_item(NAME(m_cs2));
	save_item(NAME(m_busy));
	save_item(NAME(m_read));
	save_item(NAME(m_write));
	save_item(NAME(m_address_write));
	save_item(NAME(m_reg));
	save_item(NAME(m_latch));
	save_item(NAME(m_address));
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void msm58321_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	devcb_call_write_line(&m_out_busy_func, m_busy);

	m_busy = !m_busy;
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( msm58321_device::read )
{
	UINT8 data = 0;

	if (m_cs1 && m_cs2)
	{
		if (m_read)
		{
			system_time systime;

			m_machine.current_datetime(systime);

			switch (m_address)
			{
			case REGISTER_S1:	data = systime.local_time.second % 10; break;
			case REGISTER_S10:	data = systime.local_time.second / 10; break;
			case REGISTER_MI1:	data = systime.local_time.minute % 10; break;
			case REGISTER_MI10: data = systime.local_time.minute / 10; break;
			case REGISTER_H1:	data = systime.local_time.hour % 10; break;
			case REGISTER_H10:	data = (systime.local_time.hour / 10) | 0x08; break;
			case REGISTER_W:	data = systime.local_time.weekday; break;
			case REGISTER_D1:	data = systime.local_time.mday % 10; break;
			case REGISTER_D10:	data = (systime.local_time.mday / 10) | ((systime.local_time.year % 4) ? 0 : 0x04); break;
			case REGISTER_MO1:	data = (systime.local_time.month + 1) % 10; break;
			case REGISTER_MO10: data = (systime.local_time.month + 1) / 10; break;
			case REGISTER_Y1:	data = systime.local_time.year % 10; break;
			case REGISTER_Y10:	data = (systime.local_time.year / 10) % 10;	break;

			case REGISTER_RESET:
				break;

			case REGISTER_REF0:
			case REGISTER_REF1:
				break;

			default:
				data = m_reg[offset];
				break;
			}
		}

		if (m_write)
		{
			if (m_address >= REGISTER_REF0)
			{
				// TODO: output reference values
			}
		}
	}

	if (LOG) logerror("MSM58321 '%s' Register Read %01x: %01x\n", tag(), m_address, data & 0x0f);

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( msm58321_device::write )
{
	// latch data for future use
	m_latch = data & 0x0f;

	if (!m_cs1 || !m_cs2) return;

	if (m_address_write)
	{
		if (LOG) logerror("MSM58321 '%s' Latch Address %01x\n", tag(), m_latch);

		// latch address
		m_address = m_latch;
	}

	if (m_write)
	{
		switch(m_address)
		{
		case REGISTER_RESET:
			if (LOG) logerror("MSM58321 '%s' Reset\n", tag());
			break;

		case REGISTER_REF0:
		case REGISTER_REF1:
			if (LOG) logerror("MSM58321 '%s' Reference Signal\n", tag());
			break;

		default:
			if (LOG) logerror("MSM58321 '%s' Register Write %01x: %01x\n", tag(), m_address, data & 0x0f);
			m_reg[offset] = m_latch & 0x0f;
			break;
		}
	}
}


//-------------------------------------------------
//  cs1_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm58321_device::cs1_w )
{
	if (LOG) logerror("MSM58321 '%s' CS1: %u\n", tag(), state);

	m_cs1 = state;
}


//-------------------------------------------------
//  cs2_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm58321_device::cs2_w )
{
	if (LOG) logerror("MSM58321 '%s' CS2: %u\n", tag(), state);

	m_cs2 = state;
}


//-------------------------------------------------
//  read_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm58321_device::read_w )
{
	if (LOG) logerror("MSM58321 '%s' READ: %u\n", tag(), state);

	m_read = state;
}


//-------------------------------------------------
//  write_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm58321_device::write_w )
{
	if (LOG) logerror("MSM58321 '%s' WRITE: %u\n", tag(), state);

	m_write = state;
}


//-------------------------------------------------
//  address_write_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm58321_device::address_write_w )
{
	if (LOG) logerror("MSM58321 '%s' ADDRESS WRITE: %u\n", tag(), state);

	m_address_write = state;

	if (m_address_write)
	{
		if (LOG) logerror("MSM58321 '%s' Latch Address %01x\n", tag(), m_latch);

		// latch address
		m_address = m_latch;
	}
}


//-------------------------------------------------
//  stop_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm58321_device::stop_w )
{
	if (LOG) logerror("MSM58321 '%s' STOP: %u\n", tag(), state);
}


//-------------------------------------------------
//  test_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( msm58321_device::test_w )
{
	if (LOG) logerror("MSM58321 '%s' TEST: %u\n", tag(), state);
}


//-------------------------------------------------
//  busy_r -
//-------------------------------------------------

READ_LINE_MEMBER( msm58321_device::busy_r )
{
	return m_busy;
}
