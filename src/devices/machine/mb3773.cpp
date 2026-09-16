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


#define WATCHDOG_DEBUG ( 0 )

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(MB3773, mb3773_device, "mb3773", "MB3773 Power Supply Monitor")

//-------------------------------------------------
//  mb3773_device - constructor
//-------------------------------------------------

mb3773_device::mb3773_device( const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock )
	: device_t(mconfig, MB3773, tag, owner, clock), m_watchdog_timer(nullptr), m_ck(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mb3773_device::device_start()
{
	m_watchdog_timer = timer_alloc(FUNC(mb3773_device::watchdog_expired), this);

	save_item( NAME(m_ck) );
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mb3773_device::device_reset()
{
	reset_timer();
}

TIMER_CALLBACK_MEMBER(mb3773_device::watchdog_expired)
{
	logerror("watchdog_expired: reset issued\n");

#if WATCHDOG_DEBUG
	machine().debug_break();
#else
	machine().schedule_soft_reset();
#endif
}

void mb3773_device::reset_timer()
{
	m_watchdog_timer->adjust( attotime::from_seconds( 5 ) );
}

void mb3773_device::write_line_ck(int state)
{
	if( state == 0 && m_ck != 0 )
	{
		reset_timer();
	}

	m_ck = state;
}
