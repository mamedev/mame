// license:BSD-3-Clause
// copyright-holders:68bit
/***************************************************************************

SWTPC 8212 Video Terminal

Front end interfacing the terminal device to a MAME RS232 port.

****************************************************************************/

#include "emu.h"
#include "machine/swtpc8212.h"
#include "bus/rs232/rs232.h"

class swtpc8212_state : public driver_device
{
public:
	swtpc8212_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag)
		, m_swtpc8212(*this, "swtpc8212")
		, m_rs232(*this, "rs232")
	{ }

	void swtpc8212(machine_config &config);

private:
	required_device<swtpc8212_device> m_swtpc8212;
	required_device<rs232_port_device> m_rs232;
};


void swtpc8212_state::swtpc8212(machine_config &config)
{
	SWTPC8212(config, m_swtpc8212, 0);

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->dcd_handler().set(m_swtpc8212, FUNC(swtpc8212_device::rs232_conn_dcd_w));
	m_rs232->dsr_handler().set(m_swtpc8212, FUNC(swtpc8212_device::rs232_conn_dsr_w));
	m_rs232->ri_handler().set(m_swtpc8212, FUNC(swtpc8212_device::rs232_conn_ri_w));
	m_rs232->cts_handler().set(m_swtpc8212, FUNC(swtpc8212_device::rs232_conn_cts_w));
	m_rs232->rxd_handler().set(m_swtpc8212, FUNC(swtpc8212_device::rs232_conn_rxd_w));

	m_swtpc8212->rs232_conn_txd_handler().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_swtpc8212->rs232_conn_dtr_handler().set(m_rs232, FUNC(rs232_port_device::write_dtr));
	m_swtpc8212->rs232_conn_rts_handler().set(m_rs232, FUNC(rs232_port_device::write_rts));
}


ROM_START(swtpc8212)
ROM_END

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY  FULLNAME       FLAGS
COMP(1980, swtpc8212, 0, 0, swtpc8212, 0, swtpc8212_state, empty_init, "Southwest Technical Products", "SWTPC 8212 Video Terminal", 0 )
