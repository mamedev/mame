// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//
// TODO: Emulation of the circuitry to convert 300-baud Kansas City Standard data into bits.
//
// Decoding is done in hardware by an external box, and decoded bits are fed to bit 0 of the controller port,
// with sync bits being fed to bit 1.

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
	return data;
}

uint8_t astrocade_cassette_device::read_knob()
{
	return 0;
}

void astrocade_cassette_device::device_add_mconfig(machine_config &config)
{
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED);
	m_cassette->set_interface("astrocade_cass");
}
