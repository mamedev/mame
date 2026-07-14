// license:BSD-3-Clause
// copyright-holders:Dave Rand

#include "emu.h"
#include "s97801.h"

DEFINE_DEVICE_TYPE(SERIAL_TERMINAL_S97801, s97801_terminal_device, "s97801_terminal", "Siemens 97801 Terminal (RS-232)")

s97801_terminal_device::s97801_terminal_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SERIAL_TERMINAL_S97801, tag, owner, clock)
	, device_rs232_port_interface(mconfig, *this)
	, m_term(*this, "term")
{
}

void s97801_terminal_device::device_add_mconfig(machine_config &config)
{
	SIEMENS_97801(config, m_term, 0);
	m_term->txd_handler().set(FUNC(s97801_terminal_device::term_txd_w)); // terminal -> host
}

void s97801_terminal_device::term_txd_w(int state)
{
	output_rxd(state);
}

void s97801_terminal_device::device_start()
{
}

void s97801_terminal_device::device_reset()
{
	// idle the data line at MARK until the terminal's own UART drives it - otherwise the host
	// sees a continuous BREAK during the terminal's boot (the SERAD firmware treats that as a
	// line fault / flow stop on its XON/XOFF console link)
	output_rxd(1);
	// a powered 97801 holds its modem outputs asserted
	output_dcd(0);
	output_dsr(0);
	output_cts(0);
}

void s97801_terminal_device::input_txd(int state) // host -> terminal
{
	m_term->rxd_w(state);
}
