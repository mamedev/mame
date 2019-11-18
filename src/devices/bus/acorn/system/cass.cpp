// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Cassette Interface

    Part No. 100,001

**********************************************************************/


#include "emu.h"
#include "cass.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ACORN_CASS, acorn_cass_device, "acorn_cass", "Acorn Cassette Interface")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void acorn_cass_device::device_add_mconfig(machine_config &config)
{
	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	WAVE(config, "wave", "cassette").add_route(ALL_OUTPUTS, "mono", 0.25);

	CASSETTE(config, "cassette", 0);
	TIMER(config, "cass_c").configure_periodic(FUNC(acorn_cass_device::cass_c), attotime::from_hz(4800));
	TIMER(config, "cass_p").configure_periodic(FUNC(acorn_cass_device::cass_p), attotime::from_hz(40000));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  acorn_cass_device - constructor
//-------------------------------------------------

acorn_cass_device::acorn_cass_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ACORN_CASS, tag, owner, clock)
	, device_acorn_bus_interface(mconfig, *this)
	, m_cass(*this, "cassette")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void acorn_cass_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

WRITE_LINE_MEMBER(acorn_cass_device::cass_w)
{
	m_cass_state = state;
}

TIMER_DEVICE_CALLBACK_MEMBER(acorn_cass_device::cass_c)
{
	m_cass_data[3]++;

	if (m_cass_state != m_cassold)
	{
		m_cass_data[3] = 0;
		m_cassold = m_cass_state;
	}

	if (m_cass_state)
		m_cass->output(BIT(m_cass_data[3], 0) ? -1.0 : +1.0); // 2400Hz
	else
		m_cass->output(BIT(m_cass_data[3], 1) ? -1.0 : +1.0); // 1200Hz
}

TIMER_DEVICE_CALLBACK_MEMBER(acorn_cass_device::cass_p)
{
	/* cassette - turn 1200/2400Hz to a bit */
	m_cass_data[1]++;
	uint8_t cass_ws = (m_cass->input() > +0.03) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		//m_bus->write_pb7(m_cass_data[1] < 12);
		m_cass_data[1] = 0;
	}
}
