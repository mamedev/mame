// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore C2N/1530/1531 Datassette emulation

**********************************************************************/

#include "emu.h"
#include "c2n.h"

#include "formats/cbm_tap.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C2N,   c2n_device,   "c2n",   "Commodore C2N Datassette")
DEFINE_DEVICE_TYPE(C1530, c1530_device, "c1530", "Commodore 1530 Datassette")
DEFINE_DEVICE_TYPE(C1531, c1531_device, "c1531", "Commodore 1531 Datassette")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c2n_device::device_add_mconfig(machine_config &config)
{
	CASSETTE(config, m_cassette);
	m_cassette->set_formats(cbm_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_MUTED);
	m_cassette->set_interface("cbm_cass");
	m_cassette->change_handler().set(FUNC(c2n_device::cassette_changed));
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c2n_device - constructor
//-------------------------------------------------

c2n_device::c2n_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_pet_datassette_port_interface(mconfig, *this)
	, m_cassette(*this, "cassette")
	, m_read_timer(nullptr)
{
}

c2n_device::c2n_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: c2n_device(mconfig, C2N, tag, owner, clock)
{
}


//-------------------------------------------------
//  c1530_device - constructor
//-------------------------------------------------

c1530_device::c1530_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: c2n_device(mconfig, C1530, tag, owner, clock)
{
}


//-------------------------------------------------
//  c1531_device - constructor
//-------------------------------------------------

c1531_device::c1531_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: c2n_device(mconfig, C1531, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c2n_device::device_start()
{
	// allocate timers
	m_read_timer = timer_alloc(FUNC(c2n_device::read_tick), this);
}


//-------------------------------------------------
//  read_tick - update the datassette read line
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(c2n_device::read_tick)
{
	cassette_changed(0);
}


void c2n_device::cassette_changed(int state)
{
	m_slot->read_w(datassette_read());
	m_read_timer->adjust(m_cassette->time_to_next_transition());
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
	m_cassette->change_state(state ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
}
