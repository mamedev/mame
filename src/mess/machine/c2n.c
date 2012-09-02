/**********************************************************************

    Commodore C2N/1530/1531 Datassette emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "c2n.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define CASSETTE_TAG	"cassette"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C1530 = &device_creator<c1530_device>;
const device_type C1531 = &device_creator<c1531_device>;


//-------------------------------------------------
//  MACHINE_CONFIG( c2n )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c2n )
	MCFG_CASSETTE_ADD(CASSETTE_TAG, cbm_cassette_interface )
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c2n_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c2n );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c2n_device - constructor
//-------------------------------------------------

c2n_device::c2n_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, type, name, tag, owner, clock),
	  device_pet_datassette_port_interface(mconfig, *this),
	  m_cassette(*this, CASSETTE_TAG)
{
}


//-------------------------------------------------
//  c1530_device - constructor
//-------------------------------------------------

c1530_device::c1530_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: c2n_device(mconfig, C1530, "C1530", tag, owner, clock) { }


//-------------------------------------------------
//  c1531_device - constructor
//-------------------------------------------------

c1531_device::c1531_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: c2n_device(mconfig, C1531, "C1531", tag, owner, clock) { }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c2n_device::device_start()
{
	// allocate timers
	m_read_timer = timer_alloc();
	m_read_timer->adjust(attotime::from_hz(44100), 0, attotime::from_hz(44100));
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void c2n_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	int input = (m_cassette->input() > +0.0) ? 1 : 0;

	m_slot->read_w(input);
}


//-------------------------------------------------
//  datassette_read - read data
//-------------------------------------------------

int c2n_device::datassette_read()
{
	return (m_cassette->input() > +0.0) ? 1 : 0;
}


//-------------------------------------------------
//  datassette_write - write data
//-------------------------------------------------

void c2n_device::datassette_write(int state)
{
	m_cassette->output(state ? -(0x5a9e >> 1) : +(0x5a9e >> 1));
}


//-------------------------------------------------
//  datassette_sense - switch sense
//-------------------------------------------------

int c2n_device::datassette_sense()
{
	return (m_cassette->get_state() & CASSETTE_MASK_UISTATE) == CASSETTE_STOPPED;
}


//-------------------------------------------------
//  datassette_motor - motor
//-------------------------------------------------

void c2n_device::datassette_motor(int state)
{
	if (!state)
	{
		m_cassette->change_state(CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
		m_read_timer->enable(true);
	}
	else
	{
		m_cassette->change_state(CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
		m_read_timer->enable(false);
	}
}
