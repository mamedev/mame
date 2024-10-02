// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    SWTPC MP-S Serial Interface

**********************************************************************/

#include "emu.h"
#include "mps.h"

#include "bus/rs232/rs232.h"
#include "machine/6850acia.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ss50_mps_device

class ss50_mps_device : public device_t, public ss50_card_interface
{
public:
	// construction/destruction
	ss50_mps_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, SS50_MPS, tag, owner, clock)
		, ss50_card_interface(mconfig, *this)
		, m_acia(*this, "acia")
		, m_irq_jumper(*this, "IRQ")
		, m_rate_jumper(*this, "BAUD")
		, m_cts_route(*this, "CTS_ROUTE")
	{
	}

	DECLARE_INPUT_CHANGED_MEMBER(cts_route_change);

protected:
	// device-specific overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// interface-specific overrides
	virtual u8 register_read(offs_t offset) override;
	virtual void register_write(offs_t offset, u8 data) override;
	virtual void f110_w(int state) override;
	virtual void f150_9600_w(int state) override;
	virtual void f300_w(int state) override;
	virtual void f600_4800_w(int state) override;
	virtual void f600_1200_w(int state) override;

private:
	void acia_irq_w(int state);
	void route_cts(int state);

	required_device<acia6850_device> m_acia;
	required_ioport m_irq_jumper;
	required_ioport m_rate_jumper;
	required_ioport m_cts_route;
	int m_cts;
};


static INPUT_PORTS_START( mps )
	PORT_START("IRQ")
	PORT_CONFNAME(1, 0, "IRQ")
	PORT_CONFSETTING(1, DEF_STR(Off))
	PORT_CONFSETTING(0, DEF_STR(On))

	PORT_START("BAUD")
	PORT_CONFNAME(0x1f, 0x1d, "Baud Rate")
	PORT_CONFSETTING(0x1e, "110")
	PORT_CONFSETTING(0x1d, "150 / 9600")
	PORT_CONFSETTING(0x1b, "300")
	PORT_CONFSETTING(0x17, "600")
	PORT_CONFSETTING(0x0f, "1200")

	PORT_START("CTS_ROUTE")
	PORT_CONFNAME(1, 0, "CTS route") PORT_CHANGED_MEMBER(DEVICE_SELF, ss50_mps_device, cts_route_change, 0)
	PORT_CONFSETTING(0, "Wired low (standard)")
	PORT_CONFSETTING(1, "Connected through (modified)")

INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor ss50_mps_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(mps);
}


static DEVICE_INPUT_DEFAULTS_START( terminal )
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

void ss50_mps_device::device_add_mconfig(machine_config &config)
{
	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	//m_acia->rts_handler().set(FUNC(ss50_mps_device::reader_control_w));
	m_acia->irq_handler().set(FUNC(ss50_mps_device::acia_irq_w));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
	rs232.cts_handler().set(FUNC(ss50_mps_device::route_cts));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));
}

void ss50_mps_device::device_start()
{
	save_item(NAME(m_cts));
}

//-------------------------------------------------
//  register_read - read from a port register
//-------------------------------------------------

u8 ss50_mps_device::register_read(offs_t offset)
{
	return m_acia->read(offset & 1);
}

//-------------------------------------------------
//  register_write - write to a port register
//-------------------------------------------------

void ss50_mps_device::register_write(offs_t offset, u8 data)
{
	m_acia->write(offset & 1, data);
}

void ss50_mps_device::f110_w(int state)
{
	if (!BIT(m_rate_jumper->read(), 0))
	{
		m_acia->write_txc(state);
		m_acia->write_rxc(state);
	}
}

void ss50_mps_device::f150_9600_w(int state)
{
	if (!BIT(m_rate_jumper->read(), 1))
	{
		m_acia->write_txc(state);
		m_acia->write_rxc(state);
	}
}

void ss50_mps_device::f300_w(int state)
{
	if (!BIT(m_rate_jumper->read(), 2))
	{
		m_acia->write_txc(state);
		m_acia->write_rxc(state);
	}
}

void ss50_mps_device::f600_4800_w(int state)
{
	if (!BIT(m_rate_jumper->read(), 3))
	{
		m_acia->write_txc(state);
		m_acia->write_rxc(state);
	}
}

void ss50_mps_device::f600_1200_w(int state)
{
	if (!BIT(m_rate_jumper->read(), 4))
	{
		m_acia->write_txc(state);
		m_acia->write_rxc(state);
	}
}

void ss50_mps_device::acia_irq_w(int state)
{
	if (!m_irq_jumper->read())
		write_irq(state);
}

void ss50_mps_device::route_cts(int state)
{
	if (m_cts_route->read())
		m_acia->write_cts(state);

	// Cache the state, in case the ioport setting changes.
	m_cts = state;
}

INPUT_CHANGED_MEMBER(ss50_mps_device::cts_route_change)
{
	if (newval)
		m_acia->write_cts(m_cts);
	else
		m_acia->write_cts(0);
}

// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(SS50_MPS, ss50_card_interface, ss50_mps_device, "ss50_mps", "MP-S Serial Interface")
