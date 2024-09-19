// license:BSD-3-Clause
// copyright-holders:hap
/*

Fidelity Electronics 6502 dynamic CPU clock divider

Offset-dependent CPU clock divider base class. Used to compensate slow memory
chips in chess computer models: SC12, AS12, PC, EAS, EAG.

*/

#include "emu.h"
#include "clockdiv.h"


// input ports

INPUT_PORTS_START( fidel_clockdiv_2 )
	PORT_START("div_config") // hardwired, default to /2
	PORT_CONFNAME( 0x03, 0x02, "CPU Divider" ) PORT_CHANGED_MEMBER(DEVICE_SELF, fidel_clockdiv_state, div_changed, 0)
	PORT_CONFSETTING(    0x00, "Disabled" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x03, "4" )
INPUT_PORTS_END

INPUT_PORTS_START( fidel_clockdiv_4 )
	PORT_START("div_config") // hardwired, default to /4
	PORT_CONFNAME( 0x03, 0x03, "CPU Divider" ) PORT_CHANGED_MEMBER(DEVICE_SELF, fidel_clockdiv_state, div_changed, 0)
	PORT_CONFSETTING(    0x00, "Disabled" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x03, "4" )
INPUT_PORTS_END


// implementation

void fidel_clockdiv_state::machine_start()
{
	// set up trigger for breaking out of 6502 execution (not using synchronize())
	m_div_timer = timer_alloc(FUNC(fidel_clockdiv_state::div_set_cpu_freq), this);
}

TIMER_CALLBACK_MEMBER(fidel_clockdiv_state::div_set_cpu_freq)
{
	// when a13/a14 is high, XTAL goes through divider(s)
	// (depending on factory-set jumper, either one or two 7474)
	m_maincpu->set_clock_scale(param ? m_div_scale : 1.0);
	m_div_status = param;
}

void fidel_clockdiv_state::div_prep_cpu_freq(offs_t offset)
{
	if (offset != m_div_status)
		m_div_timer->adjust(attotime::zero, offset);
}

void fidel_clockdiv_state::div_refresh(ioport_value val)
{
	if (val == 0xff)
	{
		// bail out if there is no cpu divider
		ioport_port *inp = ioport("div_config");
		if (inp == nullptr)
			return;

		val = inp->read();
	}

	m_maincpu->set_clock_scale(1.0);
	m_div_status = ~0;
	m_div_scale = (val & 1) ? 0.25 : 0.5;
	m_div_timer->adjust(attotime::never);

	// set up memory passthroughs
	m_read_tap.remove();
	m_write_tap.remove();

	if (val)
	{
		address_space &program = m_maincpu->space(AS_PROGRAM);

		m_read_tap = program.install_read_tap(
				0x0000, 0xffff,
				"program_div_r",
				[this] (offs_t offset, u8 &data, u8 mem_mask)
				{
					if (!machine().side_effects_disabled())
						div_prep_cpu_freq(offset & 0x6000);
				},
				&m_read_tap);
		m_write_tap = program.install_write_tap(
				0x0000, 0xffff,
				"program_div_w",
				[this] (offs_t offset, u8 &data, u8 mem_mask)
				{
					div_prep_cpu_freq(offset & 0x6000);
				},
				&m_write_tap);
	}
}
