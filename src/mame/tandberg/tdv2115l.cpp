// license:BSD-3-Clause
// copyright-holders:Frode van der Meeren
/***************************************************************************

    Tandberg TDV-2115L Terminal

    This driver uses the TDV-2100 series Display Logic module as a regular
    dumb-terminal, being the simplest configuration in the TDV-2100 series.

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
};

void tdv2115l_state::tdv2115l(machine_config& config)
{
	TANDBERG_TDV2100_DISPLAY_LOGIC(config, m_terminal);
	m_terminal->write_waitl_callback().set(m_keyboard, FUNC(tandberg_tdv2100_keyboard_device::waitl_w));
	m_terminal->write_onlil_callback().set(m_keyboard, FUNC(tandberg_tdv2100_keyboard_device::onlil_w));
	m_terminal->write_carl_callback().set(m_keyboard, FUNC(tandberg_tdv2100_keyboard_device::carl_w));
	m_terminal->write_errorl_callback().set(m_keyboard, FUNC(tandberg_tdv2100_keyboard_device::errorl_w));
	m_terminal->write_enql_callback().set(m_keyboard, FUNC(tandberg_tdv2100_keyboard_device::enql_w));
	m_terminal->write_ackl_callback().set(m_keyboard, FUNC(tandberg_tdv2100_keyboard_device::ackl_w));
	m_terminal->write_nakl_callback().set(m_keyboard, FUNC(tandberg_tdv2100_keyboard_device::nakl_w));

	TANDBERG_TDV2100_KEYBOARD(config, m_keyboard);
	m_keyboard->write_kstr_callback().set(m_terminal, FUNC(tandberg_tdv2100_disp_logic_device::process_keyboard_char));
	m_keyboard->write_cleark_callback().set(m_terminal, FUNC(tandberg_tdv2100_disp_logic_device::cleark_w));
	m_keyboard->write_linek_callback().set(m_terminal, FUNC(tandberg_tdv2100_disp_logic_device::linek_w));
	m_keyboard->write_transk_callback().set(m_terminal, FUNC(tandberg_tdv2100_disp_logic_device::transk_w));
	m_keyboard->write_break_callback().set(m_terminal, FUNC(tandberg_tdv2100_disp_logic_device::break_w));
}

static INPUT_PORTS_START( tdv2115l )
INPUT_PORTS_END

// ROM definition
ROM_START( tdv2115l )
ROM_END

} // anonymous namespace


//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY     FULLNAME     FLAGS
COMP( 1976, tdv2115l, 0,      0,      tdv2115l, tdv2115l, tdv2115l_state, empty_init, "Tandberg", "TDV-2115L", MACHINE_SUPPORTS_SAVE )
