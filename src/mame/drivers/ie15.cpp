// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    15IE-00-013 Terminal

    A serial (RS232 or current loop) green-screen terminal, mostly VT52
    compatible (no Hold Screen mode and no graphics character set).

    Alternate character set (selected by SO/SI chars) is Cyrillic.

****************************************************************************/


#include "emu.h"

#include "bus/rs232/rs232.h"
#include "machine/ie15.h"
#include "machine/ie15_kbd.h"


class ie15_state : public driver_device
{
public:
	ie15_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_ie15(*this, "ie15")
		, m_rs232(*this, "rs232")
	{ }

	void ie15(machine_config &config);

private:
	required_device<ie15_device> m_ie15;
	required_device<rs232_port_device> m_rs232;
};


void ie15_state::ie15(machine_config &config)
{
	IE15(config, m_ie15, 0);

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	//rs232.dcd_handler().set("ie15", FUNC(ie15_device::rs232_conn_dcd_w));
	//rs232.dsr_handler().set("ie15", FUNC(ie15_device::rs232_conn_dsr_w));
	//rs232.ri_handler().set("ie15", FUNC(ie15_device::rs232_conn_ri_w));
	//rs232.cts_handler().set("ie15", FUNC(ie15_device::rs232_conn_cts_w));
	rs232.rxd_handler().set("ie15", FUNC(ie15_device::rs232_conn_rxd_w));

	m_ie15->rs232_conn_txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_ie15->rs232_conn_dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_ie15->rs232_conn_rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
}


ROM_START(ie15)
ROM_END

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY  FULLNAME       FLAGS
COMP( 1980, ie15, 0,      0,      ie15,    0,     ie15_state, empty_init, "USSR",  "15IE-00-013", 0)
