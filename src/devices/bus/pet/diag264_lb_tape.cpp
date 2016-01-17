// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Diag264 Cassette Loop Back Connector emulation

**********************************************************************/

#include "diag264_lb_tape.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type DIAG264_CASSETTE_LOOPBACK = &device_creator<diag264_cassette_loopback_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  diag264_cassette_loopback_device - constructor
//-------------------------------------------------

diag264_cassette_loopback_device::diag264_cassette_loopback_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DIAG264_CASSETTE_LOOPBACK, "Diag264 Cassette Loopback", tag, owner, clock, "diag264_loopback_cassette", __FILE__),
		device_pet_datassette_port_interface(mconfig, *this),
		m_read(1),
		m_sense(0)
{ }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void diag264_cassette_loopback_device::device_start()
{
}


//-------------------------------------------------
//  datassette_read - read data
//-------------------------------------------------

int diag264_cassette_loopback_device::datassette_read()
{
	return m_read;
}


//-------------------------------------------------
//  datassette_write - write data
//-------------------------------------------------

void diag264_cassette_loopback_device::datassette_write(int state)
{
	m_read = state;
}


//-------------------------------------------------
//  datassette_sense - switch sense
//-------------------------------------------------

int diag264_cassette_loopback_device::datassette_sense()
{
	return m_sense;
}


//-------------------------------------------------
//  datassette_motor - motor
//-------------------------------------------------

void diag264_cassette_loopback_device::datassette_motor(int state)
{
	m_sense = !state;
}
