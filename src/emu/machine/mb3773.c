/***************************************************************************

    Fujitsu MB3773

    Power Supply Monitor with Watch Dog Timer (i.e. Reset IC)


    Todo:
        Calculate the timeout from parameters.

***************************************************************************/

#include "emu.h"
#include "mb3773.h"



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  mb3773_device_config - constructor
//-------------------------------------------------

mb3773_device_config::mb3773_device_config( const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock ) :
	device_config( mconfig, static_alloc_device_config, "MB3773", tag, owner, clock)
{
}



//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *mb3773_device_config::static_alloc_device_config( const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock )
{
	return global_alloc( mb3773_device_config( mconfig, tag, owner, clock ) );
}



//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *mb3773_device_config::alloc_device( running_machine &machine ) const
{
	return auto_alloc( &machine, mb3773_device( machine, *this ) );
}



//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void mb3773_device_config::device_config_complete()
{
}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

bool mb3773_device_config::device_validity_check( const game_driver &driver ) const
{
	return false;
}




//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mb3773_device - constructor
//-------------------------------------------------

mb3773_device::mb3773_device( running_machine &_machine, const mb3773_device_config &config ) :
	device_t( _machine, config ),
	m_config( config )
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mb3773_device::device_start()
{
	m_watchdog_timer = timer_alloc( &m_machine, watchdog_timeout, this );
	reset_timer();

	state_save_register_device_item( this, 0, m_ck );
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
	timer_adjust_oneshot( m_watchdog_timer, ATTOTIME_IN_SEC( 5 ), 0 );
}

TIMER_CALLBACK( mb3773_device::watchdog_timeout )
{
	reinterpret_cast<mb3773_device *>(ptr)->m_machine.schedule_soft_reset();
}



const device_type MB3773 = mb3773_device_config::static_alloc_device_config;
