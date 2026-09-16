// license:BSD-3-Clause
// copyright-holders:Dave Rand
/***************************************************************************

    Siemens 97801 terminal -- standalone driver

    Thin wrapper around the s97801 core device (src/devices/machine/s97801.cpp): the terminal
    plus a host serial port so it can be exercised on its own (attach null_modem/pty/loopback to
    -host_port).  The terminal's real use is as an RS-232 peripheral -- see the SERIAL_TERMINAL_S97801
    device (src/devices/bus/rs232/s97801.cpp), e.g. on the PC-MX2 SERAD port.
    SS97 host line = 38400 7O1 XON/XOFF.

***************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "machine/s97801.h"

namespace {

class s97801_state : public driver_device
{
public:
	s97801_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_term(*this, "term")
	{
	}

	void s97801(machine_config &config) ATTR_COLD;

private:
	required_device<s97801_device> m_term;
};

// SS97 host line = 38400 baud, 7 data bits, odd parity, 1 stop, XON/XOFF software handshake.
static DEVICE_INPUT_DEFAULTS_START(ss97_defaults)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD",   0xff, RS232_BAUD_38400)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD",   0xff, RS232_BAUD_38400)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_7)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY",   0xff, RS232_PARITY_ODD)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
	DEVICE_INPUT_DEFAULTS("FLOW_CONTROL",   0x07, 0x04)
DEVICE_INPUT_DEFAULTS_END

void s97801_state::s97801(machine_config &config)
{
	SIEMENS_97801(config, m_term, 0);

	rs232_port_device &host(RS232_PORT(config, "host_port", default_rs232_devices, nullptr));
	m_term->txd_handler().set(host, FUNC(rs232_port_device::write_txd));
	host.rxd_handler().set(m_term, FUNC(s97801_device::rxd_w));
	host.set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(ss97_defaults));
	host.set_option_device_input_defaults("pty", DEVICE_INPUT_DEFAULTS_NAME(ss97_defaults));
}

ROM_START(s97801)
ROM_END

} // anonymous namespace

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS         INIT        COMPANY    FULLNAME            FLAGS
COMP(1984,  s97801, 0,      0,      s97801,  0,     s97801_state, empty_init, "Siemens", "97801 (terminal)", MACHINE_SUPPORTS_SAVE)
