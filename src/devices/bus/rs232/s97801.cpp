// license:BSD-3-Clause
// copyright-holders:Dave Rand
// Siemens 97801 terminal as an RS-232 peripheral (plugs into any host serial port, e.g. the
// PC-MX2 SERAD port).  Note: the firmware drives the link at a fixed 38400 7O1 XON/XOFF.

#include "emu.h"
#include "s97801.h"

#include "machine/s97801.h"

namespace {

class s97801_terminal_device : public device_t, public device_rs232_port_interface
{
public:
	s97801_terminal_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, SERIAL_TERMINAL_S97801, tag, owner, clock)
		, device_rs232_port_interface(mconfig, *this)
		, m_term(*this, "term")
	{
	}

	virtual void input_txd(int state) override { m_term->rxd_w(state); } // host -> terminal

protected:
	virtual void device_start() override ATTR_COLD
	{
		// idle the data line at MARK at start (not reset, to avoid glitching the host line on a
		// soft reset) until the terminal's own UART drives it - otherwise the host sees a
		// continuous BREAK during the terminal's boot (the SERAD firmware treats that as a line
		// fault / flow stop on its XON/XOFF console link)
		output_rxd(1);
		// a powered 97801 holds its modem outputs asserted
		output_dcd(0);
		output_dsr(0);
		output_cts(0);
	}

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD
	{
		SIEMENS_97801(config, m_term);
		m_term->txd_handler().set(FUNC(s97801_terminal_device::term_txd_w)); // terminal -> host
	}

private:
	void term_txd_w(int state);

	required_device<s97801_device> m_term;
};

void s97801_terminal_device::term_txd_w(int state)
{
	output_rxd(state);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(SERIAL_TERMINAL_S97801, device_rs232_port_interface, s97801_terminal_device, "s97801_terminal", "Siemens 97801 Terminal (RS-232)")
