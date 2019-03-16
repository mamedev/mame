// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#include "emu.h"
#include "cassette.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ASTROCADE_CASSETTE, astrocade_cassette_device, "astrocade_cass", "Bally Astrocade Cassette")


//**************************************************************************
//    Bally Astrocade cassette input
//**************************************************************************

astrocade_cassette_device::astrocade_cassette_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ASTROCADE_CASSETTE, tag, owner, clock)
	, device_astrocade_ctrl_interface(mconfig, *this)
	, m_cassette(*this, "cassette")
{
}

astrocade_cassette_device::~astrocade_cassette_device()
{
}

uint8_t astrocade_cassette_device::read_handle()
{
	uint8_t data = m_cassette->input() > 0.0 ? 0 : 1;
	logerror("%s: Cassette Handle Read: %d\n", machine().describe_context(), data);
	return data;
}

uint8_t astrocade_cassette_device::read_knob()
{
	logerror("%s: Cassette Knob Read\n", machine().describe_context());
	return 0;
}

void astrocade_cassette_device::device_add_mconfig(machine_config &config)
{
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED);
	m_cassette->set_interface("astrocade_cass");
}
