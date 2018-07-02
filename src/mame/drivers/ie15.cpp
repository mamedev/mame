// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    15IE-00-013 Terminal

    A serial (RS232 or current loop) green-screen terminal, mostly VT52
    compatible (no Hold Screen mode and no graphics character set).

    Alternate character set (selected by SO/SI chars) is Cyrillic.

****************************************************************************/


#include "emu.h"
#include "machine/ie15_kbd.h"
#include "machine/ie15.h"


class ie15_state : public driver_device
{
public:
	ie15_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_ie15(*this, "ie15")
	{ }

	void ie15(machine_config &config);

private:
	required_device<ie15_device> m_ie15;
};


MACHINE_CONFIG_START(ie15_state::ie15)
	MCFG_DEVICE_ADD("ie15", IE15, 0)
MACHINE_CONFIG_END


ROM_START(ie15)
ROM_END

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY  FULLNAME       FLAGS
COMP( 1980, ie15, 0,      0,      ie15,    0,     ie15_state, empty_init, "USSR",  "15IE-00-013", 0)
