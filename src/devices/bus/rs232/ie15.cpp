// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev

#include "emu.h"
#include "ie15.h"

ie15_terminal_device::ie15_terminal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SERIAL_TERMINAL_IE15, tag, owner, clock)
	, device_rs232_port_interface(mconfig, *this)
	, m_ie15(*this, "ie15")
{
}

void ie15_terminal_device::device_add_mconfig(machine_config &config)
{
	IE15(config, m_ie15, 0);

	m_ie15->rs232_conn_txd_handler().set(FUNC(ie15_terminal_device::output_rxd));
	//m_ie15->rs232_conn_rts_handler().set(FUNC(ie15_terminal_device::route_term_rts));
	//m_ie15->rs232_conn_dtr_handler().set(FUNC(ie15_terminal_device::route_term_dtr));
}

INPUT_PORTS_START(ie15_terminal)
INPUT_PORTS_END

ioport_constructor ie15_terminal_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ie15_terminal);
}

WRITE_LINE_MEMBER(ie15_terminal_device::input_txd)
{
	m_ie15->rs232_conn_rxd_w(state);
}

void ie15_terminal_device::device_start()
{
}

void ie15_terminal_device::device_reset()
{
	output_rxd(1);

	// TODO: make this configurable
	output_dcd(0);
	output_dsr(0);
	output_cts(0);
}

DEFINE_DEVICE_TYPE(SERIAL_TERMINAL_IE15, ie15_terminal_device, "ie15_terminal", "IE15 Terminal")
