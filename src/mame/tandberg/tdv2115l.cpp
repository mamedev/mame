// license:BSD-3-Clause
// copyright-holders:Frode van der Meeren
/***************************************************************************

	Tandberg TDV-2115L Terminal

	This driver uses the TDV-21xx series Display Logic module as a regular
	dumb-terminal, being the simplest configuration in the TDV-21xx series.

****************************************************************************/

#include "emu.h"

#include "tdv2100_disp_logic.h"
#include "tdv2100_kbd.h"

namespace {

class tdv2115l_state : public driver_device
{
public:
	tdv2115l_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_terminal(*this, "terminal"),
		m_keyboard(*this, "keyboard")
	{}

	void tdv2115l(machine_config &config);

private:
	required_device<tandberg_tdv2100_disp_logic_device> m_terminal;
	required_device<tandberg_tdv2100_keyboard_device> m_keyboard;

	virtual void machine_start() override;
	virtual void machine_reset() override;

};

void tdv2115l_state::tdv2115l(machine_config& config)
{
	TANDBERG_TDV2100_DISPLAY_LOGIC(config, m_terminal);
	TANDBERG_TDV2100_KEYBOARD(config, m_keyboard);
	m_keyboard->write_kstr_callback().set(m_terminal, FUNC(tandberg_tdv2100_disp_logic_device::process_keyboard_char));
	m_keyboard->write_cleark_callback().set(m_terminal, FUNC(tandberg_tdv2100_disp_logic_device::w_cleark));
	m_keyboard->write_linek_callback().set(m_terminal, FUNC(tandberg_tdv2100_disp_logic_device::w_linek));
	m_keyboard->write_transk_callback().set(m_terminal, FUNC(tandberg_tdv2100_disp_logic_device::w_transk));
	m_keyboard->write_break_callback().set(m_terminal, FUNC(tandberg_tdv2100_disp_logic_device::w_break));
	m_terminal->write_waitl_callback().set(m_keyboard, FUNC(tandberg_tdv2100_keyboard_device::w_waitl));
	m_terminal->write_onlil_callback().set(m_keyboard, FUNC(tandberg_tdv2100_keyboard_device::w_onlil));
	m_terminal->write_carl_callback().set(m_keyboard, FUNC(tandberg_tdv2100_keyboard_device::w_carl));
	m_terminal->write_errorl_callback().set(m_keyboard, FUNC(tandberg_tdv2100_keyboard_device::w_errorl));
	m_terminal->write_enql_callback().set(m_keyboard, FUNC(tandberg_tdv2100_keyboard_device::w_enql));
	m_terminal->write_ackl_callback().set(m_keyboard, FUNC(tandberg_tdv2100_keyboard_device::w_ackl));
	m_terminal->write_nakl_callback().set(m_keyboard, FUNC(tandberg_tdv2100_keyboard_device::w_nakl));
}

void tdv2115l_state::machine_start()
{}

void tdv2115l_state::machine_reset()
{}

static INPUT_PORTS_START( tdv2115l )
INPUT_PORTS_END

// ROM definition
ROM_START( tdv2115l )
ROM_END

} // anonymous namespace


//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY     FULLNAME     FLAGS
COMP( 1976, tdv2115l, 0,      0,      tdv2115l, tdv2115l, tdv2115l_state, empty_init, "Tandberg", "TDV-2115L", 0 )
