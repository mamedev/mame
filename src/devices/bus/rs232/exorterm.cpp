// license:BSD-3-Clause
// copyright-holders:68bit

#include "emu.h"
#include "exorterm.h"

exorterm155_terminal_device::exorterm155_terminal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SERIAL_TERMINAL_EXORTERM155, tag, owner, clock)
	, device_rs232_port_interface(mconfig, *this)
	, m_exorterm155(*this, "exorterm155")
	, m_flow_control(*this, "FLOW_CONTROL")
{
}

void exorterm155_terminal_device::device_add_mconfig(machine_config &config)
{
	EXORTERM155(config, m_exorterm155, 0);
	m_exorterm155->rs232_conn_txd_handler().set(FUNC(exorterm155_terminal_device::output_rxd));
	m_exorterm155->rs232_conn_rts_handler().set(FUNC(exorterm155_terminal_device::route_term_rts));
	m_exorterm155->rs232_conn_dtr_handler().set(FUNC(exorterm155_terminal_device::route_term_dtr));
}

INPUT_PORTS_START(exorterm155_terminal)

	PORT_START("FLOW_CONTROL")
	PORT_CONFNAME(0x1, 1, "Flow Control") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(exorterm155_terminal_device::flow_control), 0)
	PORT_CONFSETTING(0x00, "None")
	PORT_CONFSETTING(0x01, "Terminal DTR to remote CTS")

INPUT_PORTS_END

ioport_constructor exorterm155_terminal_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(exorterm155_terminal);
}

void exorterm155_terminal_device::input_txd(int state)
{
	m_exorterm155->rs232_conn_rxd_w(state);
}

void exorterm155_terminal_device::route_term_rts(int state)
{
	// Loop the terminal RTS output to the terminal CTS input.
	m_exorterm155->rs232_conn_cts_w(state);
}

// This terminal uses DTR for hardware flow control.
void exorterm155_terminal_device::route_term_dtr(int state)
{
	if (m_flow_control->read())
	{
		// Connect the terminal DTR output to CTS at the other end.
		output_cts(state);
	}

	// Cache the state, in case the ioport setting changes.
	m_dtr = state;
}

INPUT_CHANGED_MEMBER(exorterm155_terminal_device::flow_control)
{
	if (newval)
		output_cts(m_dtr);
	else
		output_cts(0);
}

void exorterm155_terminal_device::device_start()
{
	save_item(NAME(m_dtr));
}

void exorterm155_terminal_device::device_reset()
{
	// To the terminal
	m_exorterm155->rs232_conn_cts_w(0);

	// To the computer
	output_dcd(0);
	output_dsr(0);
	if (!m_flow_control->read())
		output_cts(0);
}

DEFINE_DEVICE_TYPE(SERIAL_TERMINAL_EXORTERM155, exorterm155_terminal_device, "exorterm155_terminal", "EXORterm 155 Terminal")
