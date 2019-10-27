// license:BSD-3-Clause
// copyright-holders:68bit
/****************************************************************************

    Stand alone front end for the Qume QVT-102/QVT-102A video terminals

****************************************************************************/

#include "emu.h"
#include "machine/qvt102.h"
#include "bus/rs232/rs232.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class qvt102_state : public driver_device
{
public:
	qvt102_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_qvt102(*this, "qvt102")
		, m_host(*this, "host")
	{ }

	void qvt102(machine_config &config);

private:
	required_device<qvt102_device> m_qvt102;
	required_device<rs232_port_device> m_host;
};

void qvt102_state::qvt102(machine_config &config)
{
	QVT102(config, m_qvt102, 0);

	RS232_PORT(config, m_host, default_rs232_devices, nullptr);
	m_host->dcd_handler().set(m_qvt102, FUNC(qvt102_device::rs232_conn_dcd_w));
	m_host->dsr_handler().set(m_qvt102, FUNC(qvt102_device::rs232_conn_dsr_w));
	m_host->ri_handler().set(m_qvt102, FUNC(qvt102_device::rs232_conn_ri_w));
	m_host->cts_handler().set(m_qvt102, FUNC(qvt102_device::rs232_conn_cts_w));
	m_host->rxd_handler().set(m_qvt102, FUNC(qvt102_device::rs232_conn_rxd_w));

	m_qvt102->rs232_conn_txd_handler().set(m_host, FUNC(rs232_port_device::write_txd));
	m_qvt102->rs232_conn_dtr_handler().set(m_host, FUNC(rs232_port_device::write_dtr));
	m_qvt102->rs232_conn_rts_handler().set(m_host, FUNC(rs232_port_device::write_rts));
}


ROM_START(qvt102)
ROM_END


class qvt102a_state : public qvt102_state
{
public:
	qvt102a_state(const machine_config &mconfig, device_type type, const char *tag)
		: qvt102_state(mconfig, type, tag)
		, m_qvt102(*this, "qvt102")
		, m_host(*this, "host")
	{ }

	void qvt102a(machine_config &config);

private:
	required_device<qvt102_device> m_qvt102;
	required_device<rs232_port_device> m_host;
};

void qvt102a_state::qvt102a(machine_config &config)
{
	QVT102A(config, m_qvt102, 0);

	RS232_PORT(config, m_host, default_rs232_devices, nullptr);
	m_host->dcd_handler().set(m_qvt102, FUNC(qvt102_device::rs232_conn_dcd_w));
	m_host->dsr_handler().set(m_qvt102, FUNC(qvt102_device::rs232_conn_dsr_w));
	m_host->ri_handler().set(m_qvt102, FUNC(qvt102_device::rs232_conn_ri_w));
	m_host->cts_handler().set(m_qvt102, FUNC(qvt102_device::rs232_conn_cts_w));
	m_host->rxd_handler().set(m_qvt102, FUNC(qvt102_device::rs232_conn_rxd_w));

	m_qvt102->rs232_conn_txd_handler().set(m_host, FUNC(rs232_port_device::write_txd));
	m_qvt102->rs232_conn_dtr_handler().set(m_host, FUNC(rs232_port_device::write_dtr));
	m_qvt102->rs232_conn_rts_handler().set(m_host, FUNC(rs232_port_device::write_rts));
}

ROM_START(qvt102a)
ROM_END


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS         INIT        COMPANY  FULLNAME    FLAGS
COMP( 1983, qvt102,  0,      0,      qvt102,  0,  qvt102_state,  empty_init, "Qume",  "QVT-102",  MACHINE_SUPPORTS_SAVE )
COMP( 1983, qvt102a, qvt102, 0,      qvt102a, 0,  qvt102a_state, empty_init, "Qume",  "QVT-102A", MACHINE_SUPPORTS_SAVE )
