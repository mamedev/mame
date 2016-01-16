// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    COM8116 Dual Baud Rate Generator (Programmable Divider) emulation

**********************************************************************/

#include "com8116.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
const device_type COM8116 = &device_creator<com8116_device>;


const int com8116_device::divisors_16X_5_0688MHz[] =
	{ 6336, 4224, 2880, 2355, 2112, 1056, 528, 264, 176, 158, 132, 88, 66, 44, 33, 16 };

const int com8116_device::divisors_16X_4_9152MHz[] =
	{ 6144, 4096, 2793, 2284, 2048, 1024, 512, 256, 171, 154, 128, 85, 64, 43, 32, 16 };

const int com8116_device::divisors_32X_5_0688MHz[] =
	{ 3168, 2112, 1440, 1177, 1056, 792, 528, 264, 132, 88, 66, 44, 33, 22, 16, 8 };



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  com8116_device - constructor
//-------------------------------------------------

com8116_device::com8116_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, COM8116, "COM8116", tag, owner, clock, "com8116", __FILE__),
	m_fx4_handler(*this),
	m_fr_handler(*this),
	m_ft_handler(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void com8116_device::device_start()
{
	// resolve callbacks
	m_fx4_handler.resolve_safe();
	m_fr_handler.resolve_safe();
	m_ft_handler.resolve_safe();

	// allocate timers
	m_fx4_timer = timer_alloc(TIMER_FX4);
	m_fx4_timer->adjust(attotime::from_hz((clock() / 4) * 2), 0, attotime::from_hz((clock() / 4)) * 2);
	m_fr_timer = timer_alloc(TIMER_FR);
	m_ft_timer = timer_alloc(TIMER_FT);

	m_fr_divisors = divisors_16X_5_0688MHz;
	m_ft_divisors = divisors_16X_5_0688MHz;

	// register for state saving
	save_item(NAME(m_fx4));
	save_item(NAME(m_fr));
	save_item(NAME(m_ft));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void com8116_device::device_reset()
{
	m_fx4 = 0;
	m_fr = 0;
	m_ft = 0;
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void com8116_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_FX4:
		m_fx4 = !m_fx4;
		m_fx4_handler(m_fx4);
		break;

	case TIMER_FR:
		m_fr = !m_fr;
		m_fr_handler(m_fr);
		break;

	case TIMER_FT:
		m_ft = !m_ft;
		m_ft_handler(m_ft);
		break;
	}
}


//-------------------------------------------------
//  str_w -
//-------------------------------------------------

void com8116_device::str_w(UINT8 data)
{
	int fr_divider = data & 0x0f;
	int fr_clock = clock() / m_fr_divisors[fr_divider];

	if (LOG) logerror("COM8116 '%s' Receiver Divisor Select %01x: %u (%u Hz)\n", tag().c_str(), data & 0x0f, m_fr_divisors[fr_divider], fr_clock);

	m_fr_timer->adjust(attotime::from_nsec(3500), 0, attotime::from_hz(fr_clock * 2));
}

WRITE8_MEMBER( com8116_device::str_w )
{
	str_w(data);
}


//-------------------------------------------------
//  stt_w -
//-------------------------------------------------

void com8116_device::stt_w(UINT8 data)
{
	int ft_divider = data & 0x0f;
	int ft_clock = clock() / m_ft_divisors[ft_divider];

	if (LOG) logerror("COM8116 '%s' Transmitter Divisor Select %01x: %u (%u Hz)\n", tag().c_str(), data & 0x0f, m_ft_divisors[ft_divider], ft_clock);

	m_ft_timer->adjust(attotime::from_nsec(3500), 0, attotime::from_hz(ft_clock * 2));
}

WRITE8_MEMBER( com8116_device::stt_w )
{
	stt_w(data);
}
