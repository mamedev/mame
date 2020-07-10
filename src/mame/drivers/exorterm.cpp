// license:BSD-3-Clause
// copyright-holders: 68bit
/****************************************************************************

    Stand alone front end for the Motorola EXORterm 155 (M68SDS)

****************************************************************************/

#include "emu.h"
#include "machine/exorterm.h"
#include "bus/rs232/rs232.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class exorterm155_state : public driver_device
{
public:
	exorterm155_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_exorterm155(*this, "exorterm155")
		, m_host(*this, "host")
	{ }

	void exorterm155(machine_config &config);

private:
	required_device<exorterm155_device> m_exorterm155;
	required_device<rs232_port_device> m_host;
};

void exorterm155_state::exorterm155(machine_config &config)
{
	EXORTERM155(config, m_exorterm155, 0);

	RS232_PORT(config, m_host, default_rs232_devices, nullptr);
	m_host->dcd_handler().set(m_exorterm155, FUNC(exorterm155_device::rs232_conn_dcd_w));
	m_host->dsr_handler().set(m_exorterm155, FUNC(exorterm155_device::rs232_conn_dsr_w));
	m_host->ri_handler().set(m_exorterm155, FUNC(exorterm155_device::rs232_conn_ri_w));
	m_host->cts_handler().set(m_exorterm155, FUNC(exorterm155_device::rs232_conn_cts_w));
	m_host->rxd_handler().set(m_exorterm155, FUNC(exorterm155_device::rs232_conn_rxd_w));

	m_exorterm155->rs232_conn_txd_handler().set(m_host, FUNC(rs232_port_device::write_txd));
	m_exorterm155->rs232_conn_dtr_handler().set(m_host, FUNC(rs232_port_device::write_dtr));
	m_exorterm155->rs232_conn_rts_handler().set(m_host, FUNC(rs232_port_device::write_rts));
}


ROM_START(exorterm155)
ROM_END

//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS         INIT        COMPANY  FULLNAME    FLAGS
COMP( 1979, exorterm155,  0,      0,      exorterm155,  0,  exorterm155_state,  empty_init, "Motorola",  "EXORterm 155",  MACHINE_SUPPORTS_SAVE )
