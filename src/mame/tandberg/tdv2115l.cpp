// license:BSD-3-Clause
// copyright-holders:Frode van der Meeren
/***************************************************************************

	Tandberg TDV-2115L Terminal
	
	This driver uses the TDV-21xx series Display Logic module as a regular
	dumb-terminal, being the simplest configuration in the TDV-21xx series.

****************************************************************************/

#include "emu.h"

#include "disp_logic.h"

namespace {

class tdv2115l_state : public driver_device
{
public:
	tdv2115l_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_terminal(*this, "terminal")
	{}

	void tdv2115l(machine_config &config);

private:
	required_device<tandberg_disp_logic_device> m_terminal;

	virtual void machine_start() override;
	virtual void machine_reset() override;

};

void tdv2115l_state::tdv2115l(machine_config& config)
{
	TANDBERG_DISPLAY_LOGIC(config, m_terminal);
}

void tdv2115l_state::machine_start()
{
}

void tdv2115l_state::machine_reset()
{
}

static INPUT_PORTS_START( tdv2115l )

PORT_START("X1")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/")          PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("k_X2")
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP-0")       PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP-.")       PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP-Enter")   PORT_CODE(KEYCODE_ENTER_PAD)  PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_UNUSED)

INPUT_PORTS_END

// ROM definition
ROM_START( tdv2115l )
ROM_END

} // anonymous namespace


//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY     FULLNAME     FLAGS
COMP( 1976, tdv2115l, 0,      0,      tdv2115l, tdv2115l, tdv2115l_state, empty_init, "Tandberg", "TDV-2115L", 0 )
