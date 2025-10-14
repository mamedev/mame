// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Cassette Interface

    Part No. 100,001

**********************************************************************/

#include "emu.h"
#include "cass.h"

#include "imagedev/cassette.h"
#include "machine/timer.h"

#include "speaker.h"


namespace {

class acorn_cass_device : public device_t, public device_acorn_bus_interface
{
public:
	acorn_cass_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, ACORN_CASS, tag, owner, clock)
		, device_acorn_bus_interface(mconfig, *this)
		, m_cass(*this, "cassette")
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	//void cass_w(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(cass_c);
	TIMER_DEVICE_CALLBACK_MEMBER(cass_p);

	required_device<cassette_image_device> m_cass;

	uint8_t m_cass_data[4] = { 0 };
	bool m_cass_state = false;
	bool m_cassold = false;
};


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void acorn_cass_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	CASSETTE(config, "cassette", 0).add_route(ALL_OUTPUTS, "mono", 0.05);
	TIMER(config, "cass_c").configure_periodic(FUNC(acorn_cass_device::cass_c), attotime::from_hz(4800));
	TIMER(config, "cass_p").configure_periodic(FUNC(acorn_cass_device::cass_p), attotime::from_hz(40000));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void acorn_cass_device::device_start()
{
	save_item(NAME(m_cass_data));
	save_item(NAME(m_cass_state));
	save_item(NAME(m_cassold));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

//void acorn_cass_device::cass_w(int state)
//{
//  m_cass_state = state;
//}

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

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ACORN_CASS, device_acorn_bus_interface, acorn_cass_device, "acorn_cass", "Acorn Cassette Interface")
