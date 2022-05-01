// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    RC2014 Serial I/O Module

****************************************************************************/

#include "emu.h"
#include "serial.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "bus/rs232/rs232.h"

class serial_io_device : public device_t, public device_rc2014_card_interface
{
public:
	// construction/destruction
	serial_io_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<acia6850_device> m_acia;
};

serial_io_device::serial_io_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RC2014_SERIAL_IO, tag, owner, clock)
	, device_rc2014_card_interface(mconfig, *this)
	, m_acia(*this, "acia")
{
}

void serial_io_device::device_start()
{
	m_bus->installer(AS_IO)->install_readwrite_handler(0x80, 0x81, read8sm_delegate(*m_acia, FUNC(acia6850_device::read)), write8sm_delegate(*m_acia, FUNC(acia6850_device::write)));
}

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_115200 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_115200 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void serial_io_device::device_add_mconfig(machine_config &config)
{
	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_acia->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	//m_acia->irq_handler().set_inputline("maincpu", INPUT_LINE_IRQ0); // Connect interrupt pin to our Z80 INT line

	clock_device &acia_clock(CLOCK(config, "acia_clock", DERIVED_CLOCK(1,1)));
	acia_clock.signal_handler().set("acia", FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append("acia", FUNC(acia6850_device::write_rxc));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
	rs232.dcd_handler().set(m_acia, FUNC(acia6850_device::write_dcd));
	rs232.cts_handler().set(m_acia, FUNC(acia6850_device::write_cts));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));
}

DEFINE_DEVICE_TYPE_PRIVATE(RC2014_SERIAL_IO, device_rc2014_card_interface, serial_io_device, "rc2014_serial_io", "RC2014 Serial I/O Module")
