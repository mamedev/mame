// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    RC2014 Z80 CPU Module

****************************************************************************/

#include "emu.h"
#include "z180cpu.h"

#include "bus/rs232/rs232.h"
#include "cpu/z180/z180.h"
#include "machine/clock.h"


namespace {

//**************************************************************************
//  Z180 CPU base class
//**************************************************************************

class z180cpu_base : public device_t, public device_rc2014_rc80_card_interface
{
protected:
	// construction/destruction
	z180cpu_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void addrmap_mem(address_map &map) { map.unmap_value_high(); }
	void addrmap_io(address_map &map) { map.unmap_value_high(); }

	void clk_w(int state) { m_bus->clk_w(state); }
	void tx_w(int state) { m_bus->tx_w(state); }
	void tx2_w(int state) { m_bus->tx2_w(state); }

	virtual void card_int_w(int state) override { m_maincpu->set_input_line(INPUT_LINE_IRQ0, state); }
	virtual void card_rx_w(int state) override { m_maincpu->rxa0_w(state); }
	virtual void card_rx2_w(int state) override { m_maincpu->rxa1_w(state); }

	// object finders
	required_device<z180_device> m_maincpu;

	static constexpr XTAL MAIN_CLOCK = XTAL(18'432'000);
};

z180cpu_base::z180cpu_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_rc2014_rc80_card_interface(mconfig, *this)
	, m_maincpu(*this, "maincpu")
{
}

void z180cpu_base::device_start()
{
	m_bus->set_bus_clock(MAIN_CLOCK);
	m_maincpu->set_daisy_config(m_bus->get_daisy_chain());
}

void z180cpu_base::device_resolve_objects()
{
	m_bus->assign_installer(AS_PROGRAM, &m_maincpu->space(AS_PROGRAM));
	m_bus->assign_installer(AS_IO, &m_maincpu->space(AS_IO));
}

// This is here only to configure our terminal for interactive use
static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_115200 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_115200 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void z180cpu_base::device_add_mconfig(machine_config &config)
{
	Z8S180(config, m_maincpu, MAIN_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &z180cpu_base::addrmap_mem);
	m_maincpu->set_addrmap(AS_IO, &z180cpu_base::addrmap_io);
	m_maincpu->txa0_wr_callback().set("rs232a", FUNC(rs232_port_device::write_txd));
	m_maincpu->txa0_wr_callback().append(FUNC(z180cpu_base::tx_w));
	m_maincpu->txa1_wr_callback().set("rs232b", FUNC(rs232_port_device::write_txd));
	m_maincpu->txa1_wr_callback().append(FUNC(z180cpu_base::tx2_w));

	clock_device &clock(CLOCK(config, "clock", MAIN_CLOCK));
	clock.signal_handler().append(FUNC(z180cpu_base::clk_w));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, "terminal"));
	rs232a.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));
	rs232a.set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(terminal));
	rs232a.rxd_handler().set(m_maincpu, FUNC(z180_device::rxa0_w));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));
	rs232b.set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(terminal));
	rs232b.rxd_handler().set(m_maincpu, FUNC(z180_device::rxa1_w));
}

//**************************************************************************
//  SC111 Z180 CPU module
//  Module author: Stephen C Cousins
//**************************************************************************

class sc111_device : public z180cpu_base
{
public:
	// construction/destruction
	sc111_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
};

sc111_device::sc111_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: z180cpu_base(mconfig, RC2014_SC111, tag, owner, clock)
{
}

void sc111_device::device_start()
{
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(RC2014_SC111, device_rc2014_rc80_card_interface, sc111_device, "rc2014_sc111", "SC111 Z180 CPU module")
