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

mb3773_device::mb3773_device( const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock )
	: device_t(mconfig, MB3773, "MB3773", tag, owner, clock)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void mb3773_device::device_config_complete()
{
}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

bool mb3773_device::device_validity_check( emu_options &options, const game_driver &driver ) const
{
	return false;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mb3773_device::device_start()
{
	m_watchdog_timer = machine().scheduler().timer_alloc( FUNC(watchdog_timeout), this );
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



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

WRITE_LINE_DEVICE_HANDLER( mb3773_set_ck )
{
	downcast<mb3773_device *>( device )->set_ck( state );
}

void mb3773_device::set_ck( int state )
{
	state &= 1;

	if( state == 0 && m_ck != 0 )
	{
		reset_timer();
	}

	m_ck = state;
}


//**************************************************************************
//  INTERNAL HELPERS
//**************************************************************************

void mb3773_device::reset_timer()
{
	m_watchdog_timer->adjust( attotime::from_seconds( 5 ) );
}

TIMER_CALLBACK( mb3773_device::watchdog_timeout )
{
	reinterpret_cast<mb3773_device *>(ptr)->machine().schedule_soft_reset();
}
