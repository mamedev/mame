/**********************************************************************

    COM8116 Dual Baud Rate Generator (Programmable Divider) emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "emu.h"
#include "com8116.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type COM8116 = &device_creator<com8116_device>;

//-------------------------------------------------
//  com8116_device - constructor
//-------------------------------------------------

com8116_device::com8116_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, COM8116, "COM8116", tag, owner, clock)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void com8116_device::device_config_complete()
{
	// inherit a copy of the static data
	const com8116_interface *intf = reinterpret_cast<const com8116_interface *>(static_config());
	if (intf != NULL)
		*static_cast<com8116_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_fx4_cb, 0, sizeof(m_out_fx4_cb));
		memset(&m_out_fr_cb, 0, sizeof(m_out_fr_cb));
		memset(&m_out_ft_cb, 0, sizeof(m_out_ft_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void com8116_device::device_start()
{
	// resolve callbacks
	m_out_fx4_func.resolve(m_out_fx4_cb, *this);
	m_out_fr_func.resolve(m_out_fr_cb, *this);
	m_out_ft_func.resolve(m_out_ft_cb, *this);

	// allocate timers
	m_fx4_timer = timer_alloc(TIMER_FX4);
	m_fx4_timer->adjust(attotime::zero, 0, attotime::from_hz(clock() / 4));
	m_fr_timer = timer_alloc(TIMER_FR);
	m_ft_timer = timer_alloc(TIMER_FT);

	// register for state saving
	save_item(NAME(m_fr));
	save_item(NAME(m_ft));
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void com8116_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_FX4:
		m_out_fx4_func(1);
		break;

	case TIMER_FR:
		m_out_fr_func(1);
		break;

	case TIMER_FT:
		m_out_ft_func(1);
		break;
	}
}


//-------------------------------------------------
//  str_w -
//-------------------------------------------------

WRITE8_MEMBER( com8116_device::str_w )
{
	if (LOG) logerror("COM8116 '%s' Receiver Divider %01x\n", tag(), data & 0x0f);

	m_fr = data & 0x0f;

	m_fr_timer->adjust(attotime::zero, 0, attotime::from_hz(clock() / m_fr_divisors[m_fr] / 2));
}


//-------------------------------------------------
//  stt_w -
//-------------------------------------------------

WRITE8_MEMBER( com8116_device::stt_w )
{
	if (LOG) logerror("COM8116 '%s' Transmitter Divider %01x\n", tag(), data & 0x0f);

	m_ft = data & 0x0f;

	m_ft_timer->adjust(attotime::zero, 0, attotime::from_hz(clock() / m_ft_divisors[m_ft] / 2));
}
