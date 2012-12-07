/**********************************************************************

    Fairchild DM9368 7-Segment Decoder/Driver/Latch emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "emu.h"
#include "dm9368.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 1

// device type definition
const device_type DM9368 = &device_creator<dm9368_device>;


static const UINT8 OUTPUT[16] =
{
	0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71
};



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  get_rbi -
//-------------------------------------------------

inline int dm9368_device::get_rbi()
{
	if (!m_in_rbi_func.isnull())
	{
		m_rbi = m_in_rbi_func();
	}

	return m_rbi;
}


//-------------------------------------------------
//  set_rbo -
//-------------------------------------------------

inline void dm9368_device::set_rbo(int state)
{
	m_rbo = state;

	m_out_rbo_func(m_rbo);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dm9368_device - constructor
//-------------------------------------------------

dm9368_device::dm9368_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, DM9368, "DM9368", tag, owner, clock),
	  m_rbi(1),
	  m_rbo(1)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void dm9368_device::device_config_complete()
{
	// inherit a copy of the static data
	const dm9368_interface *intf = reinterpret_cast<const dm9368_interface *>(static_config());
	if (intf != NULL)
		*static_cast<dm9368_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_in_rbi_cb, 0, sizeof(m_in_rbi_cb));
		memset(&m_out_rbo_cb, 0, sizeof(m_out_rbo_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dm9368_device::device_start()
{
	// resolve callbacks
	m_in_rbi_func.resolve(m_in_rbi_cb, *this);
	m_out_rbo_func.resolve(m_out_rbo_cb, *this);

	// register for state saving
	save_item(NAME(m_rbi));
}


//-------------------------------------------------
//  a_w -
//-------------------------------------------------

void dm9368_device::a_w(UINT8 data)
{
	int a = data & 0x0f;

	if (!get_rbi() && !a)
	{
		if (LOG) logerror("DM9368 '%s' Blanked Rippling Zero\n", tag());

		// blank rippling 0
		output_set_digit_value(m_digit, 0);

		set_rbo(0);
	}
	else
	{
		if (LOG) logerror("DM9368 '%s' Output Data: %u = %02x\n", tag(), a, OUTPUT[a]);

		output_set_digit_value(m_digit, OUTPUT[a]);

		set_rbo(1);
	}
}


//-------------------------------------------------
//  rbi_w - ripple blanking input
//-------------------------------------------------

WRITE_LINE_MEMBER( dm9368_device::rbi_w )
{
	if (LOG) logerror("DM9368 '%s' Ripple Blanking Input: %u\n", tag(), state);

	m_rbi = state;
}


//-------------------------------------------------
//  rbo_r - ripple blanking output
//-------------------------------------------------

READ_LINE_MEMBER( dm9368_device::rbo_r )
{
	return m_rbo;
}
