// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    Fujitsu MB3773

    Power Supply Monitor with Watch Dog Timer (i.e. Reset IC)


    Todo:
        Calculate the timeout from parameters.

***************************************************************************/

#include "emu.h"
#include "mb3773.h"


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type MB3773 = &device_creator<mb3773_device>;

//-------------------------------------------------
//  mb3773_device - constructor
//-------------------------------------------------

mb3773_device::mb3773_device( const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock )
	: device_t(mconfig, MB3773, "MB3773 Power Supply Monitor", tag, owner, clock, "mb3773", __FILE__), m_watchdog_timer(nullptr), m_ck(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mb3773_device::device_start()
{
	m_watchdog_timer = timer_alloc();
	reset_timer();

	save_item( NAME(m_ck) );
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mb3773_device::device_reset()
{
	m_ck = 0;
}

void mb3773_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	machine().schedule_soft_reset();
}

void mb3773_device::reset_timer()
{
	m_watchdog_timer->adjust( attotime::from_seconds( 5 ) );
}

WRITE_LINE_MEMBER( mb3773_device::write_line_ck )
{
	if( state == 0 && m_ck != 0 )
	{
		reset_timer();
	}

	m_ck = state;
}
