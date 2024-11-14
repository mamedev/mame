// license:BSD-3-Clause
// copyright-holders:68bit
/**********************************************************************

    SWTPC MP-S2 Dual Serial Interface
    For 16 byte I/O address block.

**********************************************************************/

#include "emu.h"
#include "mps2.h"

#include "bus/rs232/rs232.h"
#include "machine/6850acia.h"
#include "machine/input_merger.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ss50_mps2_device

class ss50_mps2_device : public device_t, public ss50_card_interface
{
public:
	// construction/destruction
	ss50_mps2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, SS50_MPS2, tag, owner, clock)
		, ss50_card_interface(mconfig, *this)
		, m_acia_upper(*this, "acia_upper")
		, m_tx_rate_upper_jumper(*this, "TX_BAUD_UPPER")
		, m_rx_rate_upper_jumper(*this, "RX_BAUD_UPPER")
		, m_acia_lower(*this, "acia_lower")
		, m_tx_rate_lower_jumper(*this, "TX_BAUD_LOWER")
		, m_rx_rate_lower_jumper(*this, "RX_BAUD_LOWER")
	{
	}

protected:
	// device-specific overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override { }

	// interface-specific overrides
	virtual u8 register_read(offs_t offset) override;
	virtual void register_write(offs_t offset, u8 data) override;
	virtual void f110_w(int state) override;
	virtual void f150_9600_w(int state) override;
	virtual void f300_w(int state) override;
	virtual void f600_4800_w(int state) override;
	virtual void f600_1200_w(int state) override;

private:
	required_device<acia6850_device> m_acia_upper;
	required_ioport m_tx_rate_upper_jumper;
	required_ioport m_rx_rate_upper_jumper;
	required_device<acia6850_device> m_acia_lower;
	required_ioport m_tx_rate_lower_jumper;
	required_ioport m_rx_rate_lower_jumper;
};


static INPUT_PORTS_START( mps2 )
	PORT_START("TX_BAUD_UPPER")
	PORT_DIPNAME(0x1f, 0x1d, "Upper TX Baud Rate")
	PORT_DIPSETTING(0x1e, "110 / 440")
	PORT_DIPSETTING(0x1b, "300 / 1200")
	PORT_DIPSETTING(0x0f, "1200 / 4800")
	PORT_DIPSETTING(0x17, "4800 / 19200")
	PORT_DIPSETTING(0x1d, "9600 / 38400")

	PORT_START("RX_BAUD_UPPER")
	PORT_DIPNAME(0x1f, 0x1d, "Upper RX Baud Rate")
	PORT_DIPSETTING(0x1e, "110 / 440")
	PORT_DIPSETTING(0x1b, "300 / 1200")
	PORT_DIPSETTING(0x0f, "1200 / 4800")
	PORT_DIPSETTING(0x17, "4800 / 19200")
	PORT_DIPSETTING(0x1d, "9600 / 38400")

	PORT_START("TX_BAUD_LOWER")
	PORT_DIPNAME(0x1f, 0x1d, "Lower TX Baud Rate")
	PORT_DIPSETTING(0x1e, "110 / 440")
	PORT_DIPSETTING(0x1b, "300 / 1200")
	PORT_DIPSETTING(0x0f, "1200 / 4800")
	PORT_DIPSETTING(0x17, "4800 / 19200")
	PORT_DIPSETTING(0x1d, "9600 / 38400")

	PORT_START("RX_BAUD_LOWER")
	PORT_DIPNAME(0x1f, 0x1d, "Lower RX Baud Rate")
	PORT_DIPSETTING(0x1e, "110 / 440")
	PORT_DIPSETTING(0x1b, "300 / 1200")
	PORT_DIPSETTING(0x0f, "1200 / 4800")
	PORT_DIPSETTING(0x17, "4800 / 19200")
	PORT_DIPSETTING(0x1d, "9600 / 38400")

INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor ss50_mps2_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(mps2);
}


static DEVICE_INPUT_DEFAULTS_START( terminal_upper )
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY", 0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START( terminal_lower )
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY", 0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
DEVICE_INPUT_DEFAULTS_END

//-------------------------------------------------
//  device_add_mconfig - add device-specific
//  machine configuration
//-------------------------------------------------

void ss50_mps2_device::device_add_mconfig(machine_config &config)
{
	ACIA6850(config, m_acia_upper, 0);
	m_acia_upper->txd_handler().set("rs232_upper", FUNC(rs232_port_device::write_txd));
	m_acia_upper->rts_handler().set("rs232_upper", FUNC(rs232_port_device::write_rts));
	m_acia_upper->irq_handler().set("irq", FUNC(input_merger_device::in_w<0>));

	rs232_port_device &rs232_upper(RS232_PORT(config, "rs232_upper", default_rs232_devices, "terminal"));
	rs232_upper.rxd_handler().set(m_acia_upper, FUNC(acia6850_device::write_rxd));
	rs232_upper.cts_handler().set(m_acia_upper, FUNC(acia6850_device::write_cts));
	rs232_upper.dcd_handler().set(m_acia_upper, FUNC(acia6850_device::write_dcd));
	rs232_upper.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal_upper));

	ACIA6850(config, m_acia_lower, 0);
	m_acia_lower->txd_handler().set("rs232_lower", FUNC(rs232_port_device::write_txd));
	m_acia_lower->rts_handler().set("rs232_lower", FUNC(rs232_port_device::write_rts));
	m_acia_lower->irq_handler().set("irq", FUNC(input_merger_device::in_w<1>));

	rs232_port_device &rs232_lower(RS232_PORT(config, "rs232_lower", default_rs232_devices, "terminal"));
	rs232_lower.rxd_handler().set(m_acia_lower, FUNC(acia6850_device::write_rxd));
	rs232_lower.cts_handler().set(m_acia_lower, FUNC(acia6850_device::write_cts));
	rs232_lower.dcd_handler().set(m_acia_lower, FUNC(acia6850_device::write_dcd));
	rs232_lower.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal_lower));

	INPUT_MERGER_ANY_HIGH(config, "irq").output_handler().set(FUNC(ss50_mps2_device::write_irq));
}


//-------------------------------------------------
//  register_read - read from a port register
//-------------------------------------------------

u8 ss50_mps2_device::register_read(offs_t offset)
{
	if (offset < 2)
		return m_acia_upper->read(offset & 1);

	if (offset < 4)
		return 0;

	if (offset < 6)
		return m_acia_lower->read((offset - 4) & 1);

	if (offset < 0xe)
		return 0;

	// TODO there is also a 4 bit control line input port at
	// offset 0x0f, repeated at 0x0e.
	return 0;
}

//-------------------------------------------------
//  register_write - write to a port register
//-------------------------------------------------

void ss50_mps2_device::register_write(offs_t offset, u8 data)
{
	if (offset < 2)
		m_acia_upper->write(offset & 1, data);

	if (offset < 4)
		return;

	if (offset < 6)
		m_acia_lower->write((offset - 4) & 1, data);
}

void ss50_mps2_device::f110_w(int state)
{
	if (!BIT(m_tx_rate_upper_jumper->read(), 0))
		m_acia_upper->write_txc(state);
	if (!BIT(m_rx_rate_upper_jumper->read(), 0))
		m_acia_upper->write_rxc(state);
	if (!BIT(m_tx_rate_lower_jumper->read(), 0))
		m_acia_lower->write_txc(state);
	if (!BIT(m_rx_rate_lower_jumper->read(), 0))
		m_acia_lower->write_rxc(state);
}

void ss50_mps2_device::f150_9600_w(int state)
{
	if (!BIT(m_tx_rate_upper_jumper->read(), 1))
		m_acia_upper->write_txc(state);
	if (!BIT(m_rx_rate_upper_jumper->read(), 1))
		m_acia_upper->write_rxc(state);
	if (!BIT(m_tx_rate_lower_jumper->read(), 1))
		m_acia_lower->write_txc(state);
	if (!BIT(m_rx_rate_lower_jumper->read(), 1))
		m_acia_lower->write_rxc(state);
}

void ss50_mps2_device::f300_w(int state)
{
	if (!BIT(m_tx_rate_upper_jumper->read(), 2))
		m_acia_upper->write_txc(state);
	if (!BIT(m_rx_rate_upper_jumper->read(), 2))
		m_acia_upper->write_rxc(state);
	if (!BIT(m_tx_rate_lower_jumper->read(), 2))
		m_acia_lower->write_txc(state);
	if (!BIT(m_rx_rate_lower_jumper->read(), 2))
		m_acia_lower->write_rxc(state);
}

void ss50_mps2_device::f600_4800_w(int state)
{
	if (!BIT(m_tx_rate_upper_jumper->read(), 3))
		m_acia_upper->write_txc(state);
	if (!BIT(m_rx_rate_upper_jumper->read(), 3))
		m_acia_upper->write_rxc(state);
	if (!BIT(m_tx_rate_lower_jumper->read(), 3))
		m_acia_lower->write_txc(state);
	if (!BIT(m_rx_rate_lower_jumper->read(), 3))
		m_acia_lower->write_rxc(state);
}

void ss50_mps2_device::f600_1200_w(int state)
{
	if (!BIT(m_tx_rate_upper_jumper->read(), 4))
		m_acia_upper->write_txc(state);
	if (!BIT(m_rx_rate_upper_jumper->read(), 4))
		m_acia_upper->write_rxc(state);
	if (!BIT(m_tx_rate_lower_jumper->read(), 4))
		m_acia_lower->write_txc(state);
	if (!BIT(m_rx_rate_lower_jumper->read(), 4))
		m_acia_lower->write_rxc(state);
}


// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(SS50_MPS2, ss50_card_interface, ss50_mps2_device, "ss50_mps2", "MP-S2 Dual Serial Interface")
