// license:BSD-3-Clause
// copyright-holders:68bit

#include "emu.h"
#include "swtpc8212.h"

swtpc8212_terminal_device::swtpc8212_terminal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SERIAL_TERMINAL_SWTPC8212, tag, owner, clock)
	, device_rs232_port_interface(mconfig, *this)
	, m_swtpc8212(*this, "swtpc8212")
	, m_flow_control(*this, "FLOW_CONTROL")
{
}

void swtpc8212_terminal_device::device_add_mconfig(machine_config &config)
{
	SWTPC8212(config, m_swtpc8212, 0);
	m_swtpc8212->rs232_conn_txd_handler().set(FUNC(swtpc8212_terminal_device::output_rxd));
	m_swtpc8212->rs232_conn_rts_handler().set(FUNC(swtpc8212_terminal_device::route_term_rts));
	m_swtpc8212->rs232_conn_dtr_handler().set(FUNC(swtpc8212_terminal_device::route_term_dtr));
}

INPUT_PORTS_START(swtpc8212_terminal)

	PORT_START("FLOW_CONTROL")
	PORT_CONFNAME(0x1, 1, "Flow control") PORT_CHANGED_MEMBER(DEVICE_SELF, swtpc8212_terminal_device, flow_control, 0)
	PORT_CONFSETTING(0x00, "None")
	PORT_CONFSETTING(0x01, "Terminal DTR to remote CTS")

INPUT_PORTS_END

ioport_constructor swtpc8212_terminal_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(swtpc8212_terminal);
}

WRITE_LINE_MEMBER(swtpc8212_terminal_device::input_txd)
{
	m_swtpc8212->rs232_conn_rxd_w(state);
}

WRITE_LINE_MEMBER(swtpc8212_terminal_device::route_term_rts)
{
	// Loop the terminal RTS output to the terminal CTS input.
	m_swtpc8212->rs232_conn_cts_w(state);
}

// This terminal uses DTR for hardware flow control.
WRITE_LINE_MEMBER(swtpc8212_terminal_device::route_term_dtr)
{
	if (m_flow_control->read())
	{
		// Connect the terminal DTR output to CTS at the other end.
		output_cts(state);
	}

	// Cache the state, in case the ioport setting changes.
	m_dtr = state;
}

INPUT_CHANGED_MEMBER(swtpc8212_terminal_device::flow_control)
{
	if (newval)
		output_cts(m_dtr);
	else
		output_cts(0);
}

void swtpc8212_terminal_device::device_start()
{
	save_item(NAME(m_dtr));
}

void swtpc8212_terminal_device::device_reset()
{
	// To the terminal
	m_swtpc8212->rs232_conn_cts_w(0);

	// To the computer
	output_rxd(1);
	output_dcd(0);
	output_dsr(0);
	if (!m_flow_control->read())
		output_cts(0);
}

DEFINE_DEVICE_TYPE(SERIAL_TERMINAL_SWTPC8212, swtpc8212_terminal_device, "swtpc8212_terminal", "SWTPC8212 Terminal")
