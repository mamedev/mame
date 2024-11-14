// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    RC2014 Serial Module

****************************************************************************/

#include "emu.h"
#include "serial.h"

#include "bus/rs232/rs232.h"
#include "machine/6850acia.h"
#include "machine/z80sio.h"


namespace {

//**************************************************************************
//  RC2014 Serial I/O module
//  Module author: Spencer Owen
//**************************************************************************

class serial_io_device : public device_t, public device_rc2014_card_interface
{
public:
	// construction/destruction
	serial_io_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void irq_w(int state) { m_bus->int_w(state); }
	void tx_w(int state) { m_bus->tx_w(state); }

	virtual void card_clk_w(int state) override  { m_acia->write_txc(state); m_acia->write_rxc(state); }
	virtual void card_rx_w(int state) override   { m_acia->write_rxd(state); }

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
}

void serial_io_device::device_reset()
{
	// A15-A8 and A5-A1 not connected
	m_bus->installer(AS_IO)->install_readwrite_handler(0x80, 0x81, 0, 0xff3e, 0, read8sm_delegate(*m_acia, FUNC(acia6850_device::read)), write8sm_delegate(*m_acia, FUNC(acia6850_device::write)));
}


// JP1 is used to enable power from USB-to-Serial cable

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
	m_acia->txd_handler().append(FUNC(serial_io_device::tx_w));
	m_acia->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_acia->irq_handler().set(FUNC(serial_io_device::irq_w));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));
	rs232.set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(terminal));
}

//**************************************************************************
//  RC2014 Dual Serial module SIO/2
//  Module author: Spencer Owen
//**************************************************************************

//**************************************************************************
//  dual_serial_base
//**************************************************************************

class dual_serial_base : public device_t
{
protected:
	// construction/destruction
	dual_serial_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void irq_w(int state) = 0;
	virtual void tx_w(int state) = 0;
	virtual void tx2_w(int state) = 0;
	void clk1_w(int state) { if (m_clk_portb == 1) { m_sio->txcb_w(state); m_sio->rxcb_w(state); } }
	void clk2_w(int state) { if (m_clk_portb == 0) { m_sio->txcb_w(state); m_sio->rxcb_w(state); } }

	// base-class members
	u8 m_clk_portb;
	required_ioport m_portb;
	required_device<z80sio_device> m_sio;
};

dual_serial_base::dual_serial_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_clk_portb(0)
	, m_portb(*this, "JP1")
	, m_sio(*this, "sio")
{
}

void dual_serial_base::device_start()
{
}

void dual_serial_base::device_reset()
{
	m_clk_portb = m_portb->read();
}

void dual_serial_base::device_add_mconfig(machine_config &config)
{
	Z80SIO(config, m_sio, 0);
	m_sio->out_txda_callback().set("rs232a", FUNC(rs232_port_device::write_txd));
	m_sio->out_txda_callback().append(FUNC(dual_serial_base::tx_w));
	m_sio->out_rtsa_callback().set("rs232a", FUNC(rs232_port_device::write_rts));
	m_sio->out_txdb_callback().set("rs232b", FUNC(rs232_port_device::write_txd));
	m_sio->out_txdb_callback().append(FUNC(dual_serial_base::tx2_w));
	m_sio->out_rtsb_callback().set("rs232b", FUNC(rs232_port_device::write_rts));
	m_sio->out_int_callback().set(FUNC(dual_serial_base::irq_w));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, "terminal"));
	rs232a.rxd_handler().set(m_sio, FUNC(z80sio_device::rxa_w));
	rs232a.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));
	rs232a.set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_sio, FUNC(z80sio_device::rxb_w));
	rs232b.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));
	rs232b.set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(terminal));
}

static INPUT_PORTS_START( dual_serial_jumpers )
	PORT_START("JP1")
	PORT_CONFNAME( 0x1, 0x0, "Port B" )
	PORT_CONFSETTING( 0x0, "CLK2 (Open)" )
	PORT_CONFSETTING( 0x1, "CLK1 (Closed)" )
	// JP2 and JP3 are used to enable power from USB-to-Serial cable
INPUT_PORTS_END

ioport_constructor dual_serial_base::device_input_ports() const
{
	return INPUT_PORTS_NAME( dual_serial_jumpers );
}

//**************************************************************************
//  RC2014 Dual Serial module SIO/2 in extended bus
//**************************************************************************

class dual_serial_device : public dual_serial_base, public device_rc2014_ext_card_interface
{
public:
	// construction/destruction
	dual_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_reset() override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;

	// base-class overrides
	void irq_w(int state) override { m_bus->int_w(state); }
	void tx_w(int state) override { m_bus->tx_w(state); }
	void tx2_w(int state) override { m_bus->tx2_w(state); }

	virtual void card_clk_w(int state) override  { m_sio->txca_w(state); m_sio->rxca_w(state); clk1_w(state); }
	virtual void card_clk2_w(int state) override { clk2_w(state); }
	virtual void card_rx_w(int state) override   { m_sio->rxa_w(state); }
	virtual void card_rx2_w(int state) override  { m_sio->rxb_w(state); }
};

dual_serial_device::dual_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: dual_serial_base(mconfig, RC2014_DUAL_SERIAL, tag, owner, clock)
	, device_rc2014_ext_card_interface(mconfig, *this)
{
}

void dual_serial_device::device_reset()
{
	dual_serial_base::device_reset();
	// A15-A8 and A2 not connected
	m_bus->installer(AS_IO)->install_readwrite_handler(0x80, 0x80, 0, 0xff04, 0, read8smo_delegate(*m_sio, FUNC(z80sio_device::ca_r)), write8smo_delegate(*m_sio, FUNC(z80sio_device::ca_w)));
	m_bus->installer(AS_IO)->install_readwrite_handler(0x81, 0x81, 0, 0xff04, 0, read8smo_delegate(*m_sio, FUNC(z80sio_device::da_r)), write8smo_delegate(*m_sio, FUNC(z80sio_device::da_w)));
	m_bus->installer(AS_IO)->install_readwrite_handler(0x82, 0x82, 0, 0xff04, 0, read8smo_delegate(*m_sio, FUNC(z80sio_device::cb_r)), write8smo_delegate(*m_sio, FUNC(z80sio_device::cb_w)));
	m_bus->installer(AS_IO)->install_readwrite_handler(0x83, 0x83, 0, 0xff04, 0, read8smo_delegate(*m_sio, FUNC(z80sio_device::db_r)), write8smo_delegate(*m_sio, FUNC(z80sio_device::db_w)));
}

void dual_serial_device::device_resolve_objects()
{
	m_bus->add_to_daisy_chain(m_sio->tag());
}

//**************************************************************************
//  RC2014 Dual Serial module SIO/2 in standard bus
//**************************************************************************

class dual_serial_device_40pin : public dual_serial_base, public device_rc2014_card_interface
{
public:
	// construction/destruction
	dual_serial_device_40pin(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_reset() override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;

	// base-class overrides
	void irq_w(int state) override { m_bus->int_w(state); }
	void tx_w(int state) override { m_bus->tx_w(state); }
	void tx2_w(int state) override { }

	virtual void card_clk_w(int state) override { m_sio->txca_w(state); m_sio->rxca_w(state); clk1_w(state); }
	virtual void card_rx_w(int state) override  { m_sio->rxa_w(state); }
};

dual_serial_device_40pin::dual_serial_device_40pin(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: dual_serial_base(mconfig, RC2014_DUAL_SERIAL_40P, tag, owner, clock)
	, device_rc2014_card_interface(mconfig, *this)
{
}

void dual_serial_device_40pin::device_reset()
{
	dual_serial_base::device_reset();
	// A15-A8 and A2 not connected
	m_bus->installer(AS_IO)->install_readwrite_handler(0x80, 0x80, 0, 0xff04, 0, read8smo_delegate(*m_sio, FUNC(z80sio_device::ca_r)), write8smo_delegate(*m_sio, FUNC(z80sio_device::ca_w)));
	m_bus->installer(AS_IO)->install_readwrite_handler(0x81, 0x81, 0, 0xff04, 0, read8smo_delegate(*m_sio, FUNC(z80sio_device::da_r)), write8smo_delegate(*m_sio, FUNC(z80sio_device::da_w)));
	m_bus->installer(AS_IO)->install_readwrite_handler(0x82, 0x82, 0, 0xff04, 0, read8smo_delegate(*m_sio, FUNC(z80sio_device::cb_r)), write8smo_delegate(*m_sio, FUNC(z80sio_device::cb_w)));
	m_bus->installer(AS_IO)->install_readwrite_handler(0x83, 0x83, 0, 0xff04, 0, read8smo_delegate(*m_sio, FUNC(z80sio_device::db_r)), write8smo_delegate(*m_sio, FUNC(z80sio_device::db_w)));
}

void dual_serial_device_40pin::device_resolve_objects()
{
	m_bus->add_to_daisy_chain(m_sio->tag());
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(RC2014_SERIAL_IO, device_rc2014_card_interface, serial_io_device, "rc2014_serial_io", "RC2014 Serial I/O module")
DEFINE_DEVICE_TYPE_PRIVATE(RC2014_DUAL_SERIAL, device_rc2014_ext_card_interface, dual_serial_device, "rc2014_dual_serial", "RC2014 Dual Serial module SIO/2")
DEFINE_DEVICE_TYPE_PRIVATE(RC2014_DUAL_SERIAL_40P, device_rc2014_card_interface, dual_serial_device_40pin, "rc2014_dual_serial_40p", "RC2014 Dual Serial module SIO/2 (40 pin)")
