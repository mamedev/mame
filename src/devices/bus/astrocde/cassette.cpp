// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//
// Decoding is done in hardware by an external box, and decoded bits are fed to bit 0 of the controller port,
// with sync bits being fed to bit 1. The current HLE is not remotely accurate to hardware, but works.

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

void astrocade_cassette_device::device_start()
{
	save_item(NAME(m_cass_wave));
	save_item(NAME(m_cass_delta));
	save_item(NAME(m_cass_wave_ticks));
	save_item(NAME(m_cass_cycles));
	save_item(NAME(m_cass_mark));
}

void astrocade_cassette_device::device_reset()
{
	m_cass_wave = 0.0;
	m_cass_delta = 0.0;
	m_cass_wave_ticks = 0;
	m_cass_cycles = 0;
	m_cass_mark = false;
}

uint8_t astrocade_cassette_device::read_handle()
{
	if (m_cass_data.size())
	{
		const uint8_t data = m_cass_data.front();
		m_cass_data.pop();
		return data;
	}
	return 0;
}

uint8_t astrocade_cassette_device::read_knob()
{
	return 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(astrocade_cassette_device::check_cassette_wave)
{
	if (m_cassette->get_state() & CASSETTE_MOTOR_DISABLED)
		return;

	double old_cass_wave = m_cass_wave;
	m_cass_wave = m_cassette->input();

	bool cycled = false;
	if (old_cass_wave != m_cass_wave)
	{
		double old_delta = m_cass_delta;
		m_cass_delta = m_cass_wave - old_cass_wave;
		if (old_delta < 0.0 && m_cass_delta > 0.0)
		{
			cycled = true;
		}
	}

	if (cycled)
	{
		m_cass_mark = m_cass_wave_ticks <= 30;

		m_cass_wave_ticks = 0;

		m_cass_cycles++;
		if (m_cass_mark)
		{
			if (m_cass_cycles >= 8)
			{
				m_cass_data.push(1);
				m_cass_cycles = 0;
			}
		}
		else
		{
			if (m_cass_cycles >= 4)
			{
				m_cass_data.push(0);
				m_cass_cycles = 0;
			}
		}
	}

	m_cass_wave_ticks++;
}

TIMER_DEVICE_CALLBACK_MEMBER(astrocade_cassette_device::pulse_cassette_clock)
{
	if (m_cass_data.size())
	{
		write_ltpen(1);
		write_ltpen(0);
	}
}

void astrocade_cassette_device::device_add_mconfig(machine_config &config)
{
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED);
	m_cassette->set_interface("astrocade_cass");

	TIMER(config, "kcs_hle").configure_periodic(FUNC(astrocade_cassette_device::check_cassette_wave), attotime::from_hz(48000));
	TIMER(config, "kcs_clk").configure_periodic(FUNC(astrocade_cassette_device::pulse_cassette_clock), attotime::from_hz(300));
}
